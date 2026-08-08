from __future__ import annotations

import json
from argparse import ArgumentParser, Namespace, SUPPRESS
from pathlib import Path

from motor_debug_core.datalogger import Channel
from motor_debug_core.project import ProjectPaths
from motor_debug_core.values import FixedPoint

from .artifacts import plot_field_weakening, write_csv, write_json
from .common import (
    HardwareSession,
    firmware_metadata,
    resolve_output,
    snapshot_dict,
)
from .metrics import analyze_field_weakening


NAME = "field-weakening"
HELP = (
    "Validate automatic field weakening: negative Id is injected above base "
    "speed so the motor exceeds the voltage-limited speed."
)
TARGET_SPEED_RPM = 700.0
CURRENT_LIMIT_A = 2.0
FW_VOLTAGE_LIMIT_PU = 0.866

# IQ24 helper for Debug variables that carry no object-dictionary Q-format.
_IQ24 = FixedPoint(q=24, scale=1.0, unit="pu")


def register(parser: ArgumentParser) -> None:
    parser.add_argument(
        "--start-drive",
        action="store_true",
        help=SUPPRESS,
    )
    parser.add_argument("--timeout", type=float, default=9.0)
    parser.add_argument("--sample-divider", type=int, default=200)
    parser.add_argument("--target-speed-rpm", type=float, default=TARGET_SPEED_RPM)
    parser.add_argument("--current-limit-a", type=float, default=CURRENT_LIMIT_A)
    parser.add_argument("--min-negative-id-a", type=float, default=0.02)
    parser.add_argument("--min-voltage-pu", type=float, default=0.80)
    parser.add_argument("--speed-tolerance-percent", type=float, default=5.0)
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("artifacts/e2e/field-weakening"),
    )


def run(args: Namespace, paths: ProjectPaths) -> None:
    if not 1 <= args.sample_divider <= 255:
        raise ValueError("--sample-divider must be in range 1..255.")
    if args.timeout <= 0:
        raise ValueError("--timeout must be positive.")
    if args.target_speed_rpm <= 0:
        raise ValueError("--target-speed-rpm must be positive.")
    if args.current_limit_a <= 0:
        raise ValueError("--current-limit-a must be positive.")
    if args.min_negative_id_a < 0 or args.min_voltage_pu < 0:
        raise ValueError("Acceptance limits must not be negative.")
    output = resolve_output(paths.root, args.output)
    output.mkdir(parents=True, exist_ok=True)

    fw_voltage_limit_raw = _IQ24.encode(FW_VOLTAGE_LIMIT_PU)

    with HardwareSession(paths) as session:
        initial = session.preflight()
        ramp_time = session.read_engineering("rmp.T", unit="s")
        speed_controller = session.read_controller("pid_spd")
        hardware = session.hardware()

        # The base speed is the point where the available voltage saturates.
        # Debug variables are transient field-weakening tuning handles.
        debug_exprs = ["DebugW1", "DebugW2", "DebugW3", "DebugW4"]
        debug_original = {
            expression: session.target.read_u32(expression)
            for expression in debug_exprs
        }
        ulim_original = session.target.read_u32("pwm.U_lim")

        channels = [
            Channel(
                "speed_rpm",
                "cur_par.speed",
                session.dictionary.codec("cur_par.speed", unit="rpm"),
            ),
            Channel(
                "id_a",
                "pid_id.pid_fdb_reg3",
                session.dictionary.codec("pid_id.pid_fdb_reg3", unit="A"),
            ),
            Channel(
                "id_ref_a",
                "pid_id.pid_ref_reg3",
                session.dictionary.codec("pid_id.pid_ref_reg3", unit="A"),
            ),
            Channel(
                "fw_umag_pu",
                "Debug1",
                FixedPoint(q=24, scale=1.0, unit="pu"),
            ),
        ]
        fw_restored: dict[str, int] | None = None
        try:
            # Enable field weakening and pin the voltage limit for a deterministic run.
            session.target.write_u32("DebugW1", 1)
            session.target.write_u32("DebugW2", fw_voltage_limit_raw)
            session.target.write_u32("DebugW3", 0)
            session.target.write_u32("DebugW4", 0)
            with session.temporary_engineering_value(
                "pwm.U_lim", FW_VOLTAGE_LIMIT_PU
            ) as original_ulim_raw:
                with session.temporary_engineering_value(
                    "sm_ctrl.run_mode", 5
                ) as original_run_mode_raw:
                    with session.temporary_engineering_value(
                        "refs.speed_ref", args.target_speed_rpm, unit="rpm"
                    ) as original_speed_ref_raw:
                        with session.temporary_engineering_value(
                            "refs.Iq_ref", args.current_limit_a, unit="A"
                        ) as original_iq_ref_raw:
                            before = session.verify_mode(5)
                            capture = session.capture_triggered(
                                channels,
                                divider=args.sample_divider,
                                trigger_bit=4,
                                timeout_s=args.timeout,
                                validated_snapshot=before,
                            )
            restored = session.drive.snapshot()
        finally:
            for expression in debug_exprs:
                session.target.write_u32(expression, debug_original[expression])
            session.target.write_u32("pwm.U_lim", ulim_original)
            session.drive.stop()
            fw_restored = {
                expression: session.target.read_u32(expression)
                for expression in debug_exprs
            }

        if restored.run_mode != original_run_mode_raw:
            raise RuntimeError("Run mode was not restored after the test.")
        if restored.speed_ref_raw != original_speed_ref_raw:
            raise RuntimeError("Speed reference was not restored after the test.")

    metrics = analyze_field_weakening(
        capture.samples,
        target_speed_rpm=args.target_speed_rpm,
        min_negative_id_a=args.min_negative_id_a,
        min_voltage_pu=args.min_voltage_pu,
        speed_tolerance_percent=args.speed_tolerance_percent,
    )
    metrics["criteria"]["runtime_entered_vector_mode"] = (
        capture.runtime.control_state == 5 and capture.runtime.run_mode == 5
    )
    metrics["criteria"]["speed_reference_restored"] = (
        restored.speed_ref_raw == original_speed_ref_raw
    )
    metrics["criteria"]["debug_handles_restored"] = fw_restored == debug_original
    metrics["passed"] = all(metrics["criteria"].values())

    csv_path = output / "capture.csv"
    png_path = output / "field-weakening.png"
    json_path = output / "report.json"
    write_csv(csv_path, capture.samples)
    plot_field_weakening(png_path, capture.samples)
    report = {
        "test": NAME,
        "firmware": firmware_metadata(paths),
        "hardware": hardware,
        "setup": {
            "required_run_mode": 5,
            "target_speed_rpm": args.target_speed_rpm,
            "current_limit_a": args.current_limit_a,
            "fw_voltage_limit_pu": FW_VOLTAGE_LIMIT_PU,
            "ramp_time_s": ramp_time,
            "trigger_bit": 4,
            "trigger_timeout_s": args.timeout,
            "sample_divider": args.sample_divider,
            "speed_controller": speed_controller,
            "original_run_mode_raw": original_run_mode_raw,
            "original_speed_reference_raw": original_speed_ref_raw,
            "original_iq_reference_raw": original_iq_ref_raw,
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
    }
    write_json(json_path, report)
    print(json.dumps(metrics, ensure_ascii=False, indent=2))
    print(f"JSON: {json_path}")
    print(f"CSV: {csv_path}")
    print(f"PNG: {png_path}")
    if not metrics["passed"]:
        raise RuntimeError("Field weakening acceptance criteria failed.")
