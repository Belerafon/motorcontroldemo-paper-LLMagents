from __future__ import annotations

import json
from argparse import ArgumentParser, Namespace
from contextlib import ExitStack
from pathlib import Path

from motor_debug_core.datalogger import Channel
from motor_debug_core.project import ProjectPaths
from motor_debug_core.values import FixedPoint

from .artifacts import plot_constant_power, write_csv, write_json
from .common import HardwareSession, firmware_metadata, resolve_output, snapshot_dict
from .metrics import analyze_constant_power


NAME = "constant-power"
HELP = "Validate dynamic active-power limiting during vector acceleration."
TARGET_SPEED_RPM = 500.0


def register(parser: ArgumentParser) -> None:
    parser.add_argument("--power-limit-w", type=float, default=0.5)
    parser.add_argument("--gain", type=float, default=0.01)
    parser.add_argument("--ramp-time-s", type=float, default=0.5)
    parser.add_argument("--timeout", type=float, default=6.0)
    parser.add_argument("--sample-divider", type=int, default=100)
    parser.add_argument("--engagement-settling-s", type=float, default=0.30)
    parser.add_argument("--min-controlled-duration-s", type=float, default=0.20)
    parser.add_argument("--max-mean-error-percent", type=float, default=10.0)
    parser.add_argument("--max-overshoot-percent", type=float, default=30.0)
    parser.add_argument("--max-ripple-percent", type=float, default=30.0)
    parser.add_argument(
        "--output", type=Path, default=Path("artifacts/e2e/constant-power")
    )


def run(args: Namespace, paths: ProjectPaths) -> None:
    if not 1 <= args.sample_divider <= 255:
        raise ValueError("--sample-divider must be in range 1..255.")
    if (
        args.power_limit_w <= 0
        or args.gain < 0
        or args.ramp_time_s <= 0
        or args.timeout <= 0
    ):
        raise ValueError("Power limit and timeout must be positive; gain cannot be negative.")
    if min(
        args.min_controlled_duration_s,
        args.engagement_settling_s,
        args.max_mean_error_percent,
        args.max_overshoot_percent,
        args.max_ripple_percent,
    ) < 0:
        raise ValueError("Acceptance limits must not be negative.")

    output = resolve_output(paths.root, args.output)
    output.mkdir(parents=True, exist_ok=True)

    with HardwareSession(paths) as session:
        initial = session.preflight()
        current_limit_a = abs(session.read_engineering("refs.Iq_ref", unit="A"))
        if current_limit_a <= 0:
            raise RuntimeError("Positive refs.Iq_ref current limit is required.")
        power_base_w = (session.target.read_u32("drv_params.power") & 0xFFFF) / 64.0
        if not 0 < args.power_limit_w <= power_base_w:
            raise ValueError(
                f"--power-limit-w must be in range (0, {power_base_w:g}]."
            )
        current_base_a = session.target.read_u32("drv_params.I_nom") & 0xFFFF
        hardware = session.hardware()
        channels = [
            Channel(
                "speed_rpm",
                "cur_par.speed",
                session.dictionary.codec("cur_par.speed", unit="rpm"),
            ),
            Channel(
                "power_w",
                "cur_par.power",
                FixedPoint(24, power_base_w, "W"),
            ),
            Channel(
                "current_limit_a",
                "power_limiter.current_limit",
                FixedPoint(24, current_base_a, "A"),
            ),
            Channel(
                "iq_a",
                "pid_iq.pid_fdb_reg3",
                session.dictionary.codec("pid_iq.pid_fdb_reg3", unit="A"),
            ),
        ]

        with ExitStack() as stack:
            original_run_mode_raw = stack.enter_context(
                session.temporary_engineering_value("sm_ctrl.run_mode", 5)
            )
            original_speed_ref_raw = stack.enter_context(
                session.temporary_engineering_value(
                    "refs.speed_ref", TARGET_SPEED_RPM, unit="rpm"
                )
            )
            original_ramp_time_raw = stack.enter_context(
                session.temporary_engineering_value("rmp.T", args.ramp_time_s, unit="s")
            )
            original_power_limit_raw = stack.enter_context(
                session.temporary_engineering_value("DebugF1", args.power_limit_w)
            )
            original_gain_raw = stack.enter_context(
                session.temporary_engineering_value("DebugF2", args.gain)
            )
            original_enable_raw = stack.enter_context(
                session.temporary_engineering_value("DebugW3", 1)
            )
            before = session.verify_mode(5)
            print(
                f"Power limiter: enable=1, limit={args.power_limit_w:g} W, "
                f"gain={args.gain:g}, ramp={args.ramp_time_s:g} s, "
                f"Iq ceiling={current_limit_a:g} A"
            )
            capture = session.capture_triggered(
                channels,
                divider=args.sample_divider,
                trigger_bit=4,
                timeout_s=args.timeout,
                validated_snapshot=before,
            )

        restored = session.drive.snapshot()
        restored_values = {
            "DebugW3": session.target.read_u32("DebugW3"),
            "DebugF1": session.target.read_u32("DebugF1"),
            "DebugF2": session.target.read_u32("DebugF2"),
            "rmp.T": session.target.read_u32("rmp.T"),
        }
        expected_values = {
            "DebugW3": original_enable_raw,
            "DebugF1": original_power_limit_raw,
            "DebugF2": original_gain_raw,
            "rmp.T": original_ramp_time_raw,
        }
        if restored_values != expected_values:
            raise RuntimeError("Power-limiter settings were not restored after the test.")

    metrics = analyze_constant_power(
        capture.samples,
        power_limit_w=args.power_limit_w,
        current_limit_a=current_limit_a,
        target_speed_rpm=TARGET_SPEED_RPM,
        engagement_settling_s=args.engagement_settling_s,
        min_controlled_duration_s=args.min_controlled_duration_s,
        max_mean_error_percent=args.max_mean_error_percent,
        max_overshoot_percent=args.max_overshoot_percent,
        max_ripple_percent=args.max_ripple_percent,
    )
    metrics["criteria"]["runtime_entered_vector_mode"] = (
        capture.runtime.control_state == 5 and capture.runtime.run_mode == 5
    )
    metrics["criteria"]["temporary_settings_restored"] = (
        restored_values == expected_values
        and restored.run_mode == original_run_mode_raw
        and restored.speed_ref_raw == original_speed_ref_raw
    )
    metrics["passed"] = all(metrics["criteria"].values())

    csv_path = output / "capture.csv"
    png_path = output / "constant-power.png"
    json_path = output / "report.json"
    write_csv(csv_path, capture.samples)
    plot_constant_power(png_path, capture.samples, power_limit_w=args.power_limit_w)
    report = {
        "test": NAME,
        "firmware": firmware_metadata(paths),
        "hardware": hardware,
        "setup": {
            "required_run_mode": 5,
            "target_speed_rpm": TARGET_SPEED_RPM,
            "power_limit_w": args.power_limit_w,
            "power_base_w": power_base_w,
            "gain": args.gain,
            "ramp_time_s": args.ramp_time_s,
            "current_limit_a": current_limit_a,
            "sample_divider": args.sample_divider,
            "trigger_bit": 4,
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
        raise RuntimeError("Constant-power acceptance criteria failed.")
