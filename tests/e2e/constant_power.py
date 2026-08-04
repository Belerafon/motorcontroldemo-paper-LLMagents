from __future__ import annotations

import json
from argparse import ArgumentParser, Namespace, SUPPRESS
from pathlib import Path

from motor_debug_core.datalogger import Channel
from motor_debug_core.project import ProjectPaths

from .artifacts import plot_constant_power, write_csv, write_json
from .common import (
    HardwareSession,
    firmware_metadata,
    resolve_output,
    snapshot_dict,
)
from .metrics import analyze_constant_power


NAME = "constant-power"
HELP = "Validate constant-power acceleration with dynamic power limiting."
TARGET_SPEED_RPM = 500.0


def register(parser: ArgumentParser) -> None:
    parser.add_argument(
        "--start-drive",
        action="store_true",
        help=SUPPRESS,
    )
    parser.add_argument("--timeout", type=float, default=6.0)
    parser.add_argument("--sample-divider", type=int, default=120)
    parser.add_argument("--power-limit-w", type=float, default=150.0)
    parser.add_argument("--max-power-w", type=float, default=300.0)
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("artifacts/e2e/constant-power"),
    )


def run(args: Namespace, paths: ProjectPaths) -> None:
    if not 1 <= args.sample_divider <= 255:
        raise ValueError("--sample-divider must be in range 1..255.")
    if args.timeout <= 0:
        raise ValueError("--timeout must be positive.")
    if args.power_limit_w <= 0:
        raise ValueError("--power-limit-w must be positive.")
    if args.max_power_w < args.power_limit_w:
        raise ValueError("--max-power-w must not be less than --power-limit-w.")
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
                "power_w",
                "cur_par.power",
                session.dictionary.codec("cur_par.power", unit="W"),
            ),
        ]

        with session.temporary_engineering_value(
            "sm_ctrl.run_mode", 5
        ) as original_run_mode_raw:
            with session.temporary_engineering_value(
                "refs.speed_ref", TARGET_SPEED_RPM, unit="rpm"
            ) as original_speed_ref_raw:
                with session.temporary_engineering_value(
                    "DebugW1", 1, unit=""
                ) as original_debug_w1:
                    with session.temporary_engineering_value(
                        "DebugW2", args.power_limit_w, unit=""
                    ) as original_debug_w2:
                        before = session.verify_mode(5)
                        capture = session.capture_triggered(
                            channels,
                            divider=args.sample_divider,
                            trigger_bit=4,
                            timeout_s=args.timeout,
                            validated_snapshot=before,
                        )
        restored = session.drive.snapshot()
        if (
            restored.run_mode != original_run_mode_raw
            or restored.speed_ref_raw != original_speed_ref_raw
        ):
            raise RuntimeError(
                "Mode or speed reference was not restored after the test."
            )

    metrics = analyze_constant_power(
        capture.samples,
        target_speed_rpm=TARGET_SPEED_RPM,
        power_limit_w=args.power_limit_w,
        max_power_w=args.max_power_w,
        max_time_to_95_s=args.timeout * 0.8,
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
    metrics["criteria"]["capture_window_covers_limits"] = (
        metrics["window_s"] >= args.timeout * 0.5
    )
    metrics["passed"] = all(metrics["criteria"].values())

    csv_path = output / "capture.csv"
    png_path = output / "constant-power.png"
    json_path = output / "report.json"
    write_csv(csv_path, capture.samples)
    plot_constant_power(png_path, capture.samples)
    report = {
        "test": NAME,
        "firmware": firmware_metadata(paths),
        "hardware": hardware,
        "setup": {
            "required_run_mode": 5,
            "target_speed_rpm": TARGET_SPEED_RPM,
            "power_limit_w": args.power_limit_w,
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
        raise RuntimeError("Constant power acceptance criteria failed.")
