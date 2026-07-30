from __future__ import annotations

import json
from argparse import ArgumentParser, Namespace
from pathlib import Path

from motor_debug_core.datalogger import Channel
from motor_debug_core.project import ProjectPaths

from .artifacts import (
    plot_power_limited_acceleration,
    write_csv,
    write_json,
)
from .common import (
    HardwareSession,
    firmware_metadata,
    resolve_output,
    snapshot_dict,
)
from .metrics import analyze_power_limited_acceleration


NAME = "power-limit-500"
HELP = "Validate dynamic active-power limiting during acceleration to 500 rpm."
TARGET_SPEED_RPM = 500.0


def register(parser: ArgumentParser) -> None:
    parser.add_argument("--power-limit-w", type=float, default=2.0)
    parser.add_argument("--controller-gain", type=float, default=0.4)
    parser.add_argument("--timeout", type=float, default=6.0)
    parser.add_argument("--sample-divider", type=int, default=20)
    parser.add_argument("--max-power-overshoot-percent", type=float, default=25.0)
    parser.add_argument("--max-mean-power-error-percent", type=float, default=20.0)
    parser.add_argument("--max-time-to-95-s", type=float, default=0.5)
    parser.add_argument("--min-limited-samples", type=int, default=12)
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("artifacts/e2e/power-limit-500"),
    )


def run(args: Namespace, paths: ProjectPaths) -> None:
    if not 1 <= args.sample_divider <= 255:
        raise ValueError("--sample-divider must be in range 1..255.")
    if args.timeout <= 0 or args.power_limit_w <= 0:
        raise ValueError("Timeout and power limit must be positive.")
    if not 0 <= args.controller_gain <= 1:
        raise ValueError("--controller-gain must be in range 0..1.")
    if min(
        args.max_power_overshoot_percent,
        args.max_mean_power_error_percent,
        args.max_time_to_95_s,
        args.min_limited_samples,
    ) < 0:
        raise ValueError("Acceptance limits must not be negative.")
    output = resolve_output(paths.root, args.output)
    output.mkdir(parents=True, exist_ok=True)

    with HardwareSession(paths) as session:
        initial = session.preflight()
        current_limit = abs(session.read_engineering("refs.Iq_ref", unit="A"))
        if current_limit <= 0:
            raise RuntimeError("Positive refs.Iq_ref current limit is required.")
        current_codec = session.dictionary.codec("refs.Iq_ref", unit="A")
        channels = [
            Channel(
                "speed_rpm",
                "cur_par.speed",
                session.dictionary.codec("cur_par.speed", unit="rpm"),
            ),
            Channel(
                "active_power_w",
                "cur_par.fPower.input",
                session.dictionary.codec("cur_par.power", unit="W"),
            ),
            Channel(
                "iq_limit_a",
                "sm_ctrl.power_iq_limit",
                current_codec,
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
                with session.temporary_engineering_value(
                    "DebugW4", args.power_limit_w
                ) as original_power_limit_raw:
                    with session.temporary_engineering_value(
                        "DebugF1", args.controller_gain
                    ) as original_gain_raw:
                        with session.temporary_engineering_value(
                            "DebugW3", 1
                        ) as original_enable_raw:
                            before = session.verify_mode(5)
                            capture = session.capture_triggered(
                                channels,
                                divider=args.sample_divider,
                                trigger_bit=4,
                                timeout_s=args.timeout,
                                validated_snapshot=before,
                            )
        restored = session.drive.snapshot()
        restored_debug = {
            "enable_raw": session.target.read_u32("DebugW3"),
            "power_limit_raw": session.target.read_u32("DebugW4"),
            "gain_raw": session.target.read_u32("DebugF1"),
        }
        if (
            restored.run_mode != original_run_mode_raw
            or restored.speed_ref_raw != original_speed_ref_raw
            or restored_debug["enable_raw"] != original_enable_raw
            or restored_debug["power_limit_raw"] != original_power_limit_raw
            or restored_debug["gain_raw"] != original_gain_raw
        ):
            raise RuntimeError("Temporary power-limit settings were not restored.")
        hardware = session.hardware()

    metrics = analyze_power_limited_acceleration(
        capture.samples,
        target_speed_rpm=TARGET_SPEED_RPM,
        power_limit_w=args.power_limit_w,
        current_limit_a=current_limit,
        max_power_overshoot_percent=args.max_power_overshoot_percent,
        max_mean_power_error_percent=args.max_mean_power_error_percent,
        max_time_to_95_s=args.max_time_to_95_s,
        min_limited_samples=args.min_limited_samples,
    )
    metrics["criteria"]["runtime_entered_vector_mode"] = (
        capture.runtime.control_state == 5 and capture.runtime.run_mode == 5
    )
    metrics["criteria"]["temporary_settings_restored"] = True
    metrics["passed"] = all(metrics["criteria"].values())

    csv_path = output / "capture.csv"
    png_path = output / "power-transition.png"
    json_path = output / "report.json"
    write_csv(csv_path, capture.samples)
    plot_power_limited_acceleration(
        png_path, capture.samples, power_limit_w=args.power_limit_w
    )
    report = {
        "test": NAME,
        "firmware": firmware_metadata(paths),
        "hardware": hardware,
        "setup": {
            "required_run_mode": 5,
            "target_speed_rpm": TARGET_SPEED_RPM,
            "power_limit_w": args.power_limit_w,
            "controller_gain": args.controller_gain,
            "current_limit_a": current_limit,
            "trigger_bit": 4,
            "trigger_timeout_s": args.timeout,
            "sample_divider": args.sample_divider,
            "original_run_mode_raw": original_run_mode_raw,
            "original_speed_reference_raw": original_speed_ref_raw,
            "original_enable_raw": original_enable_raw,
            "original_power_limit_raw": original_power_limit_raw,
            "original_gain_raw": original_gain_raw,
        },
        "snapshots": {
            "initial": snapshot_dict(initial),
            "before": snapshot_dict(capture.before),
            "runtime": snapshot_dict(capture.runtime),
            "after": snapshot_dict(capture.after),
            "restored": snapshot_dict(restored),
        },
        "restored_debug": restored_debug,
        "metrics": metrics,
        "artifacts": {"csv": csv_path.name, "plot": png_path.name},
    }
    write_json(json_path, report)
    print(json.dumps(metrics, ensure_ascii=False, indent=2))
    print(f"JSON: {json_path}")
    print(f"CSV: {csv_path}")
    print(f"PNG: {png_path}")
    if not metrics["passed"]:
        raise RuntimeError("Power-limited acceleration acceptance criteria failed.")
