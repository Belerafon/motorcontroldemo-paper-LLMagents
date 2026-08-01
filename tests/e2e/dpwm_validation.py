from __future__ import annotations

import json
from argparse import ArgumentParser, Namespace, SUPPRESS
from pathlib import Path

from motor_debug_core.datalogger import Channel
from motor_debug_core.project import ProjectPaths

from .artifacts import plot_vector_500, write_csv, write_json
from .common import (
    HardwareSession,
    firmware_metadata,
    resolve_output,
    snapshot_dict,
)
from .metrics import analyze_vector_500


NAME = "dpwm-validation"
HELP = "Validate DPWM modulation type during vector-control acceleration."
TARGET_SPEED_RPM = 500.0
DPWM_TYPE = 7


def register(parser: ArgumentParser) -> None:
    parser.add_argument(
        "--start-drive",
        action="store_true",
        help=SUPPRESS,
    )
    parser.add_argument("--timeout", type=float, default=6.0)
    parser.add_argument("--sample-divider", type=int, default=120)
    parser.add_argument("--max-time-to-95-s", type=float, default=2.0)
    parser.add_argument("--max-overshoot-percent", type=float, default=10.0)
    parser.add_argument("--max-settling-s", type=float, default=2.5)
    parser.add_argument("--max-steady-error-percent", type=float, default=5.0)
    parser.add_argument("--max-speed-ripple-rpm", type=float, default=150.0)
    parser.add_argument("--current-limit-margin-percent", type=float, default=35.0)
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("artifacts/e2e/dpwm-validation"),
    )


def run(args: Namespace, paths: ProjectPaths) -> None:
    if not 1 <= args.sample_divider <= 255:
        raise ValueError("--sample-divider must be in range 1..255.")
    if args.timeout <= 0:
        raise ValueError("--timeout must be positive.")
    if min(
        args.max_time_to_95_s,
        args.max_overshoot_percent,
        args.max_settling_s,
        args.max_steady_error_percent,
        args.max_speed_ripple_rpm,
        args.current_limit_margin_percent,
    ) < 0:
        raise ValueError("Acceptance limits must not be negative.")
    output = resolve_output(paths.root, args.output)
    output.mkdir(parents=True, exist_ok=True)

    with HardwareSession(paths) as session:
        initial = session.preflight()
        current_limit = abs(session.read_engineering("refs.Iq_ref", unit="A"))
        if current_limit <= 0:
            raise RuntimeError("Positive refs.Iq_ref current limit is required.")
        ramp_time = session.read_engineering("rmp.T", unit="s")
        speed_controller = session.read_controller("pid_spd")
        hardware = session.hardware()

        original_pwm_type = session.target.read_u32("pwm.PWM_type")
        session.target.write_u32("pwm.PWM_type", DPWM_TYPE)
        actual_pwm_type = session.target.read_u32("pwm.PWM_type")
        if actual_pwm_type != DPWM_TYPE:
            raise RuntimeError(f"Failed to set DPWM type: expected {DPWM_TYPE}, got {actual_pwm_type}")

        channels = [
            Channel(
                "speed_rpm",
                "cur_par.speed",
                session.dictionary.codec("cur_par.speed", unit="rpm"),
            ),
            Channel(
                "speed_ref_rpm",
                "rmp.output",
                session.dictionary.codec("rmp.output", unit="rpm"),
            ),
            Channel(
                "iq_a",
                "pid_iq.pid_fdb_reg3",
                session.dictionary.codec("pid_iq.pid_fdb_reg3", unit="A"),
            ),
            Channel(
                "iq_ref_a",
                "pid_iq.pid_ref_reg3",
                session.dictionary.codec("pid_iq.pid_ref_reg3", unit="A"),
            ),
        ]

        with session.temporary_engineering_value(
            "sm_ctrl.run_mode", 5
        ) as original_run_mode_raw:
            with session.temporary_engineering_value(
                "refs.speed_ref", TARGET_SPEED_RPM, unit="rpm"
            ) as original_speed_ref_raw:
                before = session.verify_mode(5)
                capture = session.capture_triggered(
                    channels,
                    divider=args.sample_divider,
                    trigger_bit=4,
                    timeout_s=args.timeout,
                    validated_snapshot=before,
                )
        restored = session.drive.snapshot()
        if restored.run_mode != original_run_mode_raw:
            raise RuntimeError("Run mode was not restored after the test.")
        if restored.speed_ref_raw != original_speed_ref_raw:
            raise RuntimeError("Speed reference was not restored after the test.")

        session.target.write_u32("pwm.PWM_type", original_pwm_type)
        restored_pwm_type = session.target.read_u32("pwm.PWM_type")
        if restored_pwm_type != original_pwm_type:
            raise RuntimeError("PWM type was not restored after the test.")

    metrics = analyze_vector_500(
        capture.samples,
        target_speed_rpm=TARGET_SPEED_RPM,
        current_limit_a=current_limit,
        max_time_to_95_s=args.max_time_to_95_s,
        max_overshoot_percent=args.max_overshoot_percent,
        max_settling_s=args.max_settling_s,
        max_steady_error_percent=args.max_steady_error_percent,
        max_speed_ripple_rpm=args.max_speed_ripple_rpm,
        current_limit_margin_percent=args.current_limit_margin_percent,
    )
    metrics["criteria"]["dpwm_type_set"] = actual_pwm_type == DPWM_TYPE
    metrics["criteria"]["dpwm_type_restored"] = restored_pwm_type == original_pwm_type
    metrics["criteria"]["runtime_entered_vector_mode"] = (
        capture.runtime.control_state == 5 and capture.runtime.run_mode == 5
    )
    metrics["criteria"]["speed_reference_restored"] = (
        restored.speed_ref_raw == original_speed_ref_raw
    )
    metrics["criteria"]["run_mode_restored"] = (
        restored.run_mode == original_run_mode_raw
    )
    metrics["criteria"]["capture_window_covers_limits"] = (
        metrics["window_s"]
        >= max(args.max_time_to_95_s, args.max_settling_s) + 0.1
    )
    metrics["criteria"]["settled"] = True
    metrics["passed"] = all(metrics["criteria"].values())

    csv_path = output / "capture.csv"
    png_path = output / "speed-transition.png"
    json_path = output / "report.json"
    write_csv(csv_path, capture.samples)
    plot_vector_500(png_path, capture.samples)
    report = {
        "test": NAME,
        "firmware": firmware_metadata(paths),
        "hardware": hardware,
        "setup": {
            "dpwm_type": DPWM_TYPE,
            "original_pwm_type": original_pwm_type,
            "required_run_mode": 5,
            "target_speed_rpm": TARGET_SPEED_RPM,
            "current_limit_a": current_limit,
            "ramp_time_s": ramp_time,
            "trigger_bit": 4,
            "trigger_timeout_s": args.timeout,
            "sample_divider": args.sample_divider,
            "speed_controller": speed_controller,
            "original_run_mode_raw": original_run_mode_raw,
            "original_speed_reference_raw": original_speed_ref_raw,
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
        raise RuntimeError("DPWM validation acceptance criteria failed.")
