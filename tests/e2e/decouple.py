from __future__ import annotations

import json
import time
from argparse import ArgumentParser, Namespace, SUPPRESS
from pathlib import Path

from motor_debug_core.datalogger import Channel
from motor_debug_core.project import ProjectPaths
from motor_debug_core.values import FixedPoint

from .artifacts import plot_decouple, write_csv, write_json
from .common import (
    HardwareSession,
    firmware_metadata,
    resolve_output,
    snapshot_dict,
)
from .metrics import analyze_decouple


NAME = "decouple"
HELP = (
    "Validate d-q cross-coupling decoupling of the current regulators "
    "during a vector-control acceleration."
)
TARGET_SPEED_RPM = 500.0
TRIGGER_BIT = 4
IQ24 = 24


def register(parser: ArgumentParser) -> None:
    parser.add_argument(
        "--start-drive",
        action="store_true",
        help=SUPPRESS,
    )
    parser.add_argument("--timeout", type=float, default=6.0)
    parser.add_argument("--sample-divider", type=int, default=120)
    parser.add_argument(
        "--k-dec-pu",
        type=float,
        default=0.1,
        help="Cross-coupling gain in per-unit (written to DebugW2 as IQ24).",
    )
    parser.add_argument("--max-time-to-95-s", type=float, default=2.0)
    parser.add_argument("--max-overshoot-percent", type=float, default=12.0)
    parser.add_argument("--max-settling-s", type=float, default=2.5)
    parser.add_argument("--max-steady-error-percent", type=float, default=2.5)
    parser.add_argument("--min-decouple-peak-pu", type=float, default=0.002)
    parser.add_argument("--max-decouple-rmse-pu", type=float, default=0.01)
    parser.add_argument("--max-steady-uq-dec-abs-pu", type=float, default=0.05)
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("artifacts/e2e/decouple"),
    )


def _verify_disabled_outputs_zero(session: HardwareSession) -> None:
    raw_ud = session.target.read_u32("Debug1")
    raw_uq = session.target.read_u32("Debug2")
    if raw_ud != 0 or raw_uq != 0:
        raise RuntimeError(
            "Decoupling outputs are not identically zero while disabled: "
            f"Debug1=0x{raw_ud:08X}, Debug2=0x{raw_uq:08X}."
        )


def run(args: Namespace, paths: ProjectPaths) -> None:
    if not 1 <= args.sample_divider <= 255:
        raise ValueError("--sample-divider must be in range 1..255.")
    if args.timeout <= 0:
        raise ValueError("--timeout must be positive.")
    if args.k_dec_pu < 0:
        raise ValueError("--k-dec-pu must not be negative.")
    if min(
        args.max_time_to_95_s,
        args.max_overshoot_percent,
        args.max_settling_s,
        args.max_steady_error_percent,
        args.min_decouple_peak_pu,
        args.max_decouple_rmse_pu,
        args.max_steady_uq_dec_abs_pu,
    ) < 0:
        raise ValueError("Acceptance limits must not be negative.")
    output = resolve_output(paths.root, args.output)
    output.mkdir(parents=True, exist_ok=True)

    k_dec_raw = int(round(args.k_dec_pu * (1 << IQ24))) & 0xFFFFFFFF
    k_dec_pu = k_dec_raw / float(1 << IQ24)

    with HardwareSession(paths) as session:
        initial = session.preflight()
        current_limit = abs(session.read_engineering("refs.Iq_ref", unit="A"))
        if current_limit <= 0:
            raise RuntimeError("Positive refs.Iq_ref current limit is required.")
        speed_nom = session.target.read_u32("drv_params.speed_nom")
        if speed_nom <= 0:
            raise RuntimeError("Positive drv_params.speed_nom is required.")
        current_base = session.target.read_u32("drv_params.I_nom")
        if current_base <= 0:
            raise RuntimeError("Positive drv_params.I_nom is required.")
        ramp_time = session.read_engineering("rmp.T", unit="s")
        hardware = session.hardware()

        original_debug_w1 = session.target.read_u32("DebugW1")
        original_debug_w2 = session.target.read_u32("DebugW2")

        disabled_ud_raw: int | None = None
        disabled_uq_raw: int | None = None
        capture: object = None
        original_run_mode_raw = 0
        original_speed_ref_raw = 0
        try:
            session.target.write_u32("DebugW2", k_dec_raw)
            session.target.write_u32("DebugW1", 0)

            with session.temporary_engineering_value(
                "sm_ctrl.run_mode", 5
            ) as original_run_mode_raw:
                with session.temporary_engineering_value(
                    "refs.speed_ref", TARGET_SPEED_RPM, unit="rpm"
                ) as original_speed_ref_raw:
                    disabled_before = session.verify_mode(5)
                    session.drive.start(validated_snapshot=disabled_before)
                    deadline = time.monotonic() + min(args.timeout, 3.0)
                    while time.monotonic() < deadline:
                        runtime = session.drive.snapshot()
                        if (
                            runtime.protection_state != 1
                            or runtime.fault1
                            or runtime.fault2
                        ):
                            raise RuntimeError(
                                "Protection changed while disabled-decouple "
                                f"spin-up: {runtime.describe()}"
                            )
                        time.sleep(0.1)
                    _verify_disabled_outputs_zero(session)
                    disabled_ud_raw = session.target.read_u32("Debug1")
                    disabled_uq_raw = session.target.read_u32("Debug2")
            session.stop_and_wait()

            with session.temporary_engineering_value(
                "sm_ctrl.run_mode", 5
            ) as second_run_mode_raw:
                if second_run_mode_raw != original_run_mode_raw:
                    raise RuntimeError(
                        "Run mode changed unexpectedly between phases."
                    )
                with session.temporary_engineering_value(
                    "refs.speed_ref", TARGET_SPEED_RPM, unit="rpm"
                ) as second_speed_ref_raw:
                    if second_speed_ref_raw != original_speed_ref_raw:
                        raise RuntimeError(
                            "Speed reference changed unexpectedly between phases."
                        )
                    session.target.write_u32("DebugW1", 1)
                    before = session.verify_mode(5)
                    voltage_codec = FixedPoint(q=IQ24, scale=1.0, unit="pu")
                    channels = [
                        Channel(
                            "speed_rpm",
                            "cur_par.speed",
                            session.dictionary.codec(
                                "cur_par.speed", unit="rpm"
                            ),
                        ),
                        Channel(
                            "iq_a",
                            "pid_iq.pid_fdb_reg3",
                            session.dictionary.codec(
                                "pid_iq.pid_fdb_reg3", unit="A"
                            ),
                        ),
                        Channel("ud_dec_pu", "Debug1", voltage_codec),
                        Channel("uq_dec_pu", "Debug2", voltage_codec),
                    ]
                    capture = session.capture_triggered(
                        channels,
                        divider=args.sample_divider,
                        trigger_bit=TRIGGER_BIT,
                        timeout_s=args.timeout,
                        validated_snapshot=before,
                    )
        finally:
            session.target.write_u32("DebugW1", original_debug_w1)
            session.target.write_u32("DebugW2", original_debug_w2)

        restored = session.drive.snapshot()
        restored_debug_w1 = session.target.read_u32("DebugW1")
        restored_debug_w2 = session.target.read_u32("DebugW2")
        if (
            restored.run_mode != original_run_mode_raw
            or restored.speed_ref_raw != original_speed_ref_raw
            or restored_debug_w1 != original_debug_w1
            or restored_debug_w2 != original_debug_w2
        ):
            raise RuntimeError(
                "Temporary decouple test settings were not restored."
            )

    metrics = analyze_decouple(
        capture.samples,
        target_speed_rpm=TARGET_SPEED_RPM,
        speed_nom_rpm=speed_nom,
        current_base_a=current_base,
        k_dec_pu=k_dec_pu,
        max_time_to_95_s=args.max_time_to_95_s,
        max_overshoot_percent=args.max_overshoot_percent,
        max_settling_s=args.max_settling_s,
        max_steady_error_percent=args.max_steady_error_percent,
        min_decouple_peak_pu=args.min_decouple_peak_pu,
        max_decouple_rmse_pu=args.max_decouple_rmse_pu,
        max_steady_uq_dec_abs_pu=args.max_steady_uq_dec_abs_pu,
    )
    metrics["criteria"]["disabled_outputs_zero"] = (
        disabled_ud_raw == 0 and disabled_uq_raw == 0
    )
    metrics["criteria"]["runtime_entered_vector_mode"] = (
        capture.runtime.control_state == 5 and capture.runtime.run_mode == 5
    )
    metrics["criteria"]["temporary_settings_restored"] = (
        restored.run_mode == original_run_mode_raw
        and restored.speed_ref_raw == original_speed_ref_raw
        and restored_debug_w1 == original_debug_w1
        and restored_debug_w2 == original_debug_w2
    )
    metrics["criteria"]["capture_window_covers_limits"] = (
        metrics["window_s"] >= max(args.max_time_to_95_s, args.max_settling_s) + 0.1
    )
    metrics["passed"] = all(metrics["criteria"].values())

    csv_path = output / "capture.csv"
    png_path = output / "decouple.png"
    json_path = output / "report.json"
    write_csv(csv_path, capture.samples)
    plot_decouple(png_path, capture.samples)
    report = {
        "test": NAME,
        "firmware": firmware_metadata(paths),
        "hardware": hardware,
        "setup": {
            "required_run_mode": 5,
            "target_speed_rpm": TARGET_SPEED_RPM,
            "current_limit_a": current_limit,
            "speed_nom_rpm": speed_nom,
            "current_base_a": current_base,
            "ramp_time_s": ramp_time,
            "k_dec_pu_requested": args.k_dec_pu,
            "k_dec_pu_effective": k_dec_pu,
            "k_dec_debugw2_raw": k_dec_raw,
            "trigger_bit": TRIGGER_BIT,
            "trigger_timeout_s": args.timeout,
            "sample_divider": args.sample_divider,
            "enable_debugw1_raw_on": 1,
            "original_debug_w1_raw": original_debug_w1,
            "original_debug_w2_raw": original_debug_w2,
            "original_run_mode_raw": original_run_mode_raw,
            "original_speed_reference_raw": original_speed_ref_raw,
            "disabled_ud_dec_raw": disabled_ud_raw,
            "disabled_uq_dec_raw": disabled_uq_raw,
        },
        "snapshots": {
            "initial": snapshot_dict(initial),
            "disabled_before": snapshot_dict(disabled_before),
            "before": snapshot_dict(capture.before),
            "runtime": snapshot_dict(capture.runtime),
            "after": snapshot_dict(capture.after),
            "restored": snapshot_dict(restored),
        },
        "metrics": metrics,
        "artifacts": {"csv": csv_path.name, "plot": png_path.name},
    }
    write_json(json_path, report)
    print(json.dumps(metrics, ensure_ascii=False, indent=2))
    print(f"JSON: {json_path}")
    print(f"CSV: {csv_path}")
    print(f"PNG: {png_path}")
    if not metrics["passed"]:
        raise RuntimeError("Decoupling acceptance criteria failed.")
