from __future__ import annotations

import json
import time
from argparse import ArgumentParser, Namespace, SUPPRESS
from pathlib import Path

from motor_debug_core.datalogger import Channel
from motor_debug_core.project import ProjectPaths
from motor_debug_core.values import FixedPoint

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


NAME = "vector-power-limit"
HELP = "Validate constant-power acceleration in vector mode."
TARGET_SPEED_RPM = 500.0


def register(parser: ArgumentParser) -> None:
    parser.add_argument("--start-drive", action="store_true", help=SUPPRESS)
    parser.add_argument("--power-limit-w", type=int, default=5)
    parser.add_argument("--kp", type=float, default=0.5)
    parser.add_argument("--timeout", type=float, default=8.0)
    parser.add_argument("--sample-divider", type=int, default=20)
    parser.add_argument("--min-plateau-fraction", type=float, default=0.55)
    parser.add_argument("--max-power-overshoot-percent", type=float, default=20.0)
    parser.add_argument("--plateau-low-percent", type=float, default=75.0)
    parser.add_argument("--max-time-to-95-s", type=float, default=4.5)
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("artifacts/e2e/vector-power-limit"),
    )


def run(args: Namespace, paths: ProjectPaths) -> None:
    if not 1 <= args.sample_divider <= 255:
        raise ValueError("--sample-divider must be in range 1..255.")
    if args.power_limit_w <= 0:
        raise ValueError("--power-limit-w must be positive.")
    if not 0 < args.kp <= 4.0:
        raise ValueError("--kp must be in range (0, 4].")
    if args.timeout <= 0:
        raise ValueError("--timeout must be positive.")
    if not 0 <= args.min_plateau_fraction <= 1:
        raise ValueError("--min-plateau-fraction must be in range 0..1.")
    if min(
        args.max_power_overshoot_percent,
        args.plateau_low_percent,
        args.max_time_to_95_s,
    ) < 0:
        raise ValueError("Acceptance limits must not be negative.")

    output = resolve_output(paths.root, args.output)
    output.mkdir(parents=True, exist_ok=True)

    with HardwareSession(paths) as session:
        initial = session.preflight()
        current_limit = abs(session.read_engineering("refs.Iq_ref", unit="A"))
        if current_limit <= 0:
            raise RuntimeError("Positive refs.Iq_ref current limit is required.")
        base_power_w = FixedPoint(q=6, unit="W").decode(
            session.target.read_u32("drv_params.power")
        )
        if base_power_w <= 0:
            raise RuntimeError("Positive drv_params.power is required.")
        if args.power_limit_w > 4.0 * base_power_w:
            raise ValueError(
                "--power-limit-w exceeds the firmware limit of four "
                "nominal powers."
            )

        channels = [
            Channel(
                "speed_rpm",
                "cur_par.speed",
                session.dictionary.codec("cur_par.speed", unit="rpm"),
            ),
            Channel(
                "power_w",
                "power_limiter.power_fdb",
                FixedPoint(q=24, scale=base_power_w, unit="W"),
            ),
            Channel(
                "iq_ref_a",
                "pid_iq.pid_ref_reg3",
                session.dictionary.codec("pid_iq.pid_ref_reg3", unit="A"),
            ),
            Channel(
                "iq_limit_a",
                "pid_spd.pid_out_max",
                session.dictionary.codec("pid_spd.pid_out_max", unit="A"),
            ),
        ]
        kp_raw = round(args.kp * (1 << 24))
        hardware = session.hardware()

        with session.temporary_engineering_value(
            "sm_ctrl.run_mode", 5
        ) as original_run_mode_raw:
            with session.temporary_engineering_value(
                "refs.speed_ref", TARGET_SPEED_RPM, unit="rpm"
            ) as original_speed_ref_raw:
                with session.temporary_engineering_value(
                    "DebugW2", kp_raw
                ) as original_kp_raw:
                    with session.temporary_engineering_value(
                        "DebugW4", args.power_limit_w
                    ) as original_limit_raw:
                        with session.temporary_engineering_value(
                            "DebugW3", 1
                        ) as original_enable_raw:
                            time.sleep(0.05)
                            configured_limit_pu = FixedPoint(q=24).decode(
                                session.target.read_u32(
                                    "power_limiter.power_limit"
                                )
                            )
                            expected_limit_pu = args.power_limit_w / base_power_w
                            if abs(configured_limit_pu - expected_limit_pu) > 1e-4:
                                raise RuntimeError(
                                    "Power-limit conversion was not applied: "
                                    f"{configured_limit_pu:.6f} != "
                                    f"{expected_limit_pu:.6f}."
                                )
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
            "DebugW2": session.target.read_u32("DebugW2"),
            "DebugW3": session.target.read_u32("DebugW3"),
            "DebugW4": session.target.read_u32("DebugW4"),
        }

    metrics = analyze_power_limited_acceleration(
        capture.samples,
        target_speed_rpm=TARGET_SPEED_RPM,
        power_limit_w=float(args.power_limit_w),
        current_limit_a=current_limit,
        min_plateau_fraction=args.min_plateau_fraction,
        max_power_overshoot_percent=args.max_power_overshoot_percent,
        plateau_low_percent=args.plateau_low_percent,
        max_time_to_95_s=args.max_time_to_95_s,
    )
    metrics["criteria"]["runtime_entered_vector_mode"] = (
        capture.runtime.control_state == 5 and capture.runtime.run_mode == 5
    )
    metrics["criteria"]["speed_reference_restored"] = (
        restored.speed_ref_raw == original_speed_ref_raw
    )
    metrics["criteria"]["run_mode_restored"] = (
        restored.run_mode == original_run_mode_raw
    )
    metrics["criteria"]["debug_parameters_restored"] = restored_debug == {
        "DebugW2": original_kp_raw,
        "DebugW3": original_enable_raw,
        "DebugW4": original_limit_raw,
    }
    metrics["passed"] = all(metrics["criteria"].values())

    csv_path = output / "capture.csv"
    png_path = output / "power-limited-acceleration.png"
    json_path = output / "report.json"
    write_csv(csv_path, capture.samples)
    plot_power_limited_acceleration(
        png_path,
        capture.samples,
        float(args.power_limit_w),
    )
    report = {
        "test": NAME,
        "firmware": firmware_metadata(paths),
        "hardware": hardware,
        "setup": {
            "required_run_mode": 5,
            "target_speed_rpm": TARGET_SPEED_RPM,
            "base_power_w": base_power_w,
            "power_limit_w": args.power_limit_w,
            "power_limit_pu": configured_limit_pu,
            "power_controller_kp": args.kp,
            "current_limit_a": current_limit,
            "trigger_bit": 4,
            "trigger_timeout_s": args.timeout,
            "sample_divider": args.sample_divider,
            "original_run_mode_raw": original_run_mode_raw,
            "original_speed_reference_raw": original_speed_ref_raw,
            "original_debug_raw": {
                "DebugW2": original_kp_raw,
                "DebugW3": original_enable_raw,
                "DebugW4": original_limit_raw,
            },
        },
        "snapshots": {
            "initial": snapshot_dict(initial),
            "before": snapshot_dict(capture.before),
            "runtime": snapshot_dict(capture.runtime),
            "after": snapshot_dict(capture.after),
            "restored": snapshot_dict(restored),
        },
        "restored_debug_raw": restored_debug,
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
