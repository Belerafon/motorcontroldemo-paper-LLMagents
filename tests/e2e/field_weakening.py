from __future__ import annotations

import json
from argparse import ArgumentParser, Namespace, SUPPRESS
from pathlib import Path

from motor_debug_core.datalogger import Channel
from motor_debug_core.project import ProjectPaths
from motor_debug_core.values import FixedPoint

from .artifacts import plot_field_weakening, write_csv, write_json
from .common import HardwareSession, firmware_metadata, resolve_output, snapshot_dict
from .metrics import analyze_field_weakening


NAME = "field-weakening"
HELP = "Validate automatic negative-Id field weakening at 700 rpm."
TARGET_SPEED_RPM = 700.0


def register(parser: ArgumentParser) -> None:
    parser.add_argument("--start-drive", action="store_true", help=SUPPRESS)
    parser.add_argument("--target-speed-rpm", type=float, default=TARGET_SPEED_RPM)
    parser.add_argument("--timeout", type=float, default=7.0)
    parser.add_argument("--sample-divider", type=int, default=160)
    parser.add_argument("--max-speed-error-percent", type=float, default=2.0)
    parser.add_argument("--min-weakening-current-a", type=float, default=0.05)
    parser.add_argument("--max-voltage-error-pu", type=float, default=0.03)
    parser.add_argument("--current-limit-margin-percent", type=float, default=3.0)
    parser.add_argument(
        "--output", type=Path, default=Path("artifacts/e2e/field-weakening")
    )


def run(args: Namespace, paths: ProjectPaths) -> None:
    if not 1 <= args.sample_divider <= 255:
        raise ValueError("--sample-divider must be in range 1..255.")
    if args.target_speed_rpm <= 500.0:
        raise ValueError("Field-weakening target must be above 500 rpm.")
    if min(
        args.timeout,
        args.max_speed_error_percent,
        args.min_weakening_current_a,
        args.max_voltage_error_pu,
        args.current_limit_margin_percent,
    ) <= 0:
        raise ValueError("Timeout and acceptance limits must be positive.")
    output = resolve_output(paths.root, args.output)
    output.mkdir(parents=True, exist_ok=True)

    with HardwareSession(paths) as session:
        initial = session.preflight()
        hardware = session.hardware()
        current_limit = abs(session.read_engineering("refs.Iq_ref", unit="A"))
        if current_limit <= 0:
            raise RuntimeError("Positive refs.Iq_ref current limit is required.")
        enabled = session.target.read_u16("field_weakening.enabled")
        debug_mode_raw = session.target.read_u32("DebugW3")
        if debug_mode_raw >= 0x8000_0000:
            raise RuntimeError("DebugW3 disables field weakening; set it to 0 or 1.")
        if not enabled and debug_mode_raw == 0:
            raise RuntimeError("Compiled field-weakening default is disabled.")
        q24 = FixedPoint(24)
        voltage_limit = q24.decode(session.target.read_u32("pwm.U_lim"))
        target_ratio = q24.decode(
            session.target.read_u32("field_weakening.voltage_target_ratio")
        )
        voltage_target = voltage_limit * target_ratio
        current_codec = session.dictionary.codec("pid_id.pid_ref_reg3", unit="A")
        channels = [
            Channel(
                "speed_rpm",
                "cur_par.speed",
                session.dictionary.codec("cur_par.speed", unit="rpm"),
            ),
            Channel("id_ref_a", "field_weakening.id_ref", current_codec),
            Channel(
                "iq_ref_a",
                "pid_iq.pid_ref_reg3",
                session.dictionary.codec("pid_iq.pid_ref_reg3", unit="A"),
            ),
            Channel(
                "voltage_mag_pu",
                "field_weakening.voltage_mag",
                FixedPoint(24, unit="pu"),
            ),
        ]

        with session.temporary_engineering_value("sm_ctrl.run_mode", 5) as old_mode:
            with session.temporary_engineering_value(
                "refs.speed_ref", args.target_speed_rpm, unit="rpm"
            ) as old_speed:
                before = session.verify_mode(5)
                capture = session.capture_triggered(
                    channels,
                    divider=args.sample_divider,
                    trigger_bit=4,
                    timeout_s=args.timeout,
                    validated_snapshot=before,
                )
        restored = session.drive.snapshot()
        if restored.run_mode != old_mode or restored.speed_ref_raw != old_speed:
            raise RuntimeError("Mode or speed reference was not restored.")

    metrics = analyze_field_weakening(
        capture.samples,
        target_speed_rpm=args.target_speed_rpm,
        current_limit_a=current_limit,
        voltage_target_pu=voltage_target,
        max_speed_error_percent=args.max_speed_error_percent,
        min_weakening_current_a=args.min_weakening_current_a,
        max_voltage_error_pu=args.max_voltage_error_pu,
        current_limit_margin_percent=args.current_limit_margin_percent,
    )
    metrics["criteria"]["runtime_entered_vector_mode"] = (
        capture.runtime.control_state == 5 and capture.runtime.run_mode == 5
    )
    metrics["criteria"]["speed_reference_restored"] = restored.speed_ref_raw == old_speed
    metrics["criteria"]["run_mode_restored"] = restored.run_mode == old_mode
    metrics["passed"] = all(metrics["criteria"].values())

    csv_path = output / "capture.csv"
    png_path = output / "field-weakening.png"
    json_path = output / "report.json"
    write_csv(csv_path, capture.samples)
    plot_field_weakening(
        png_path,
        capture.samples,
        target_speed_rpm=args.target_speed_rpm,
        voltage_target_pu=voltage_target,
    )
    write_json(
        json_path,
        {
            "test": NAME,
            "firmware": firmware_metadata(paths),
            "hardware": {
                **hardware,
                "field_weakening_enabled": enabled,
                "debug_mode_raw": debug_mode_raw,
            },
            "setup": {
                "required_run_mode": 5,
                "target_speed_rpm": args.target_speed_rpm,
                "current_limit_a": current_limit,
                "voltage_limit_pu": voltage_limit,
                "voltage_target_ratio": target_ratio,
                "voltage_target_pu": voltage_target,
                "sample_divider": args.sample_divider,
            },
            "snapshots": {
                "initial": snapshot_dict(initial),
                "before": snapshot_dict(capture.before),
                "runtime": snapshot_dict(capture.runtime),
                "after": snapshot_dict(capture.after),
                "restored": snapshot_dict(restored),
            },
            "metrics": metrics,
            "artifacts": {"csv": csv_path.name, "plot": png_path.name},
        },
    )
    print(json.dumps(metrics, ensure_ascii=False, indent=2))
    print(f"JSON: {json_path}")
    print(f"CSV: {csv_path}")
    print(f"PNG: {png_path}")
    if not metrics["passed"]:
        raise RuntimeError("Field-weakening acceptance criteria failed.")
