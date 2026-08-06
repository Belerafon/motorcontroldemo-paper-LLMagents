from __future__ import annotations

import json
from argparse import ArgumentParser, Namespace, SUPPRESS
from pathlib import Path

from motor_debug_core.datalogger import Channel
from motor_debug_core.project import ProjectPaths
from motor_debug_core.values import FixedPoint, Float32

from .artifacts import plot_decoupling, write_csv, write_json
from .common import (
    HardwareSession,
    firmware_metadata,
    resolve_output,
    snapshot_dict,
)
from .metrics import analyze_decoupling


NAME = "decoupling"
HELP = "Validate dq current-regulator cross-coupling compensation (decoupling)."

TARGET_SPEED_RPM = 500.0
# Firmware per-unit bases, kept in sync with SM_Ctrl.c DECOUP_* defines.
SPEED_BASE_RPM = 500.0
CURRENT_BASE_A = 5.0
VOLTAGE_BASE_V = 311.0


def _capture_run(
    session: HardwareSession,
    *,
    decoupling_on: bool,
    speed_rpm: float,
    args: Namespace,
) -> tuple[list, int, int]:
    """One acceleration capture with decoupling enabled or disabled."""
    codec_ud = FixedPoint(24, 1.0, "pu")
    channels = [
        Channel(
            "speed_rpm",
            "cur_par.speed",
            session.dictionary.codec("cur_par.speed", unit="rpm"),
        ),
        Channel(
            "iq_a",
            "pid_iq.pid_fdb_reg3",
            session.dictionary.codec("pid_iq.pid_fdb_reg3", unit="A"),
        ),
        Channel(
            "id_a",
            "pid_id.pid_fdb_reg3",
            session.dictionary.codec("pid_id.pid_fdb_reg3", unit="A"),
        ),
        Channel("ud_comp_pu", "DebugW2", codec_ud),
    ]
    orig_w1 = session.target.read_u32("DebugW1")
    orig_f1 = session.target.read_u32("DebugF1")
    try:
        session.target.write_u32("DebugW1", 1 if decoupling_on else 0)
        session.target.write_u32("DebugF1", 0)  # auto coefficient from motor params
        with session.temporary_engineering_value("sm_ctrl.run_mode", 5):
            with session.temporary_engineering_value(
                "refs.speed_ref", speed_rpm, unit="rpm"
            ):
                # Clean d=0 field-oriented control so the d-axis current is a
                # pure disturbance measure rather than a commanded excitation.
                with session.temporary_engineering_value(
                    "refs.i_flux_ref", 0.0, unit="A"
                ):
                    before = session.verify_mode(5)
                    capture = session.capture_triggered(
                        channels,
                        divider=args.sample_divider,
                        trigger_bit=4,
                        timeout_s=args.timeout,
                        validated_snapshot=before,
                    )
    finally:
        session.target.write_u32("DebugW1", orig_w1)
        session.target.write_u32("DebugF1", orig_f1)
    return capture.samples, orig_w1, orig_f1


def register(parser: ArgumentParser) -> None:
    parser.add_argument("--timeout", type=float, default=6.0)
    parser.add_argument("--sample-divider", type=int, default=120)
    parser.add_argument("--high-speed-min-rpm", type=float, default=400.0)
    parser.add_argument("--current-active-min-a", type=float, default=0.10)
    parser.add_argument(
        "--feedforward-tolerance-percent", type=float, default=30.0
    )
    parser.add_argument("--min-compensation-pu", type=float, default=1.0e-5)
    parser.add_argument(
        "--max-disabled-compensation-pu", type=float, default=1.0e-6
    )
    parser.add_argument("--min-id-reduction-percent", type=float, default=0.0)
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("artifacts/e2e/decoupling"),
    )


def run(args: Namespace, paths: ProjectPaths) -> None:
    if not 1 <= args.sample_divider <= 255:
        raise ValueError("--sample-divider must be in range 1..255.")
    if args.timeout <= 0:
        raise ValueError("--timeout must be positive.")
    if min(
        args.high_speed_min_rpm,
        args.current_active_min_a,
        args.feedforward_tolerance_percent,
        args.min_compensation_pu,
        args.max_disabled_compensation_pu,
        args.min_id_reduction_percent,
    ) < 0:
        raise ValueError("Acceptance limits must not be negative.")
    output = resolve_output(paths.root, args.output)
    output.mkdir(parents=True, exist_ok=True)

    with HardwareSession(paths) as session:
        initial = session.preflight()
        hardware = session.hardware()
        lsq_mh = Float32().decode(
            session.target.read_u32("model.motorInternals.lsq")
        )
        decoup_default_raw = session.target.read_u32("DebugW1")
        decoup_enabled_by_default = bool(decoup_default_raw & 1)

        off_samples, _, _ = _capture_run(
            session, decoupling_on=False, speed_rpm=TARGET_SPEED_RPM, args=args
        )
        on_samples, _, _ = _capture_run(
            session, decoupling_on=True, speed_rpm=TARGET_SPEED_RPM, args=args
        )
        restored = session.drive.snapshot()

    metrics = analyze_decoupling(
        off_samples,
        on_samples,
        lsq_mh=lsq_mh,
        speed_base_rpm=SPEED_BASE_RPM,
        current_base_a=CURRENT_BASE_A,
        voltage_base_v=VOLTAGE_BASE_V,
        high_speed_min_rpm=args.high_speed_min_rpm,
        current_active_min_a=args.current_active_min_a,
        feedforward_tolerance_percent=args.feedforward_tolerance_percent,
        min_compensation_pu=args.min_compensation_pu,
        max_disabled_compensation_pu=args.max_disabled_compensation_pu,
        min_id_reduction_percent=args.min_id_reduction_percent,
    )
    metrics["criteria"]["decoupling_enabled_by_default"] = (
        decoup_enabled_by_default
    )
    metrics["passed"] = all(metrics["criteria"].values())

    csv_off = output / "capture-off.csv"
    csv_on = output / "capture-on.csv"
    png_path = output / "decoupling.png"
    json_path = output / "report.json"
    write_csv(csv_off, off_samples)
    write_csv(csv_on, on_samples)
    plot_decoupling(png_path, off_samples, on_samples)
    report = {
        "test": NAME,
        "firmware": firmware_metadata(paths),
        "hardware": hardware,
        "setup": {
            "required_run_mode": 5,
            "target_speed_rpm": TARGET_SPEED_RPM,
            "motor_lsq_mh": lsq_mh,
            "speed_base_rpm": SPEED_BASE_RPM,
            "current_base_a": CURRENT_BASE_A,
            "voltage_base_v": VOLTAGE_BASE_V,
            "decoupling_default_raw": decoup_default_raw,
            "trigger_bit": 4,
            "trigger_timeout_s": args.timeout,
            "sample_divider": args.sample_divider,
            "limits": metrics["limits"],
        },
        "snapshots": {
            "initial": snapshot_dict(initial),
            "restored": snapshot_dict(restored),
        },
        "metrics": metrics,
        "artifacts": {
            "csv_off": csv_off.name,
            "csv_on": csv_on.name,
            "plot": png_path.name,
        },
    }
    write_json(json_path, report)
    print(json.dumps(metrics, ensure_ascii=False, indent=2))
    print(f"JSON: {json_path}")
    print(f"CSV OFF: {csv_off}")
    print(f"CSV ON: {csv_on}")
    print(f"PNG: {png_path}")
    if not metrics["passed"]:
        raise RuntimeError("Decoupling acceptance criteria failed.")
