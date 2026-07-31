from __future__ import annotations

import json
import math
from argparse import ArgumentParser, Namespace, SUPPRESS
from pathlib import Path

from motor_debug_core.datalogger import Channel
from motor_debug_core.project import ProjectPaths

from .artifacts import plot_decoupling_high_speed, write_csv, write_json
from .common import (
    HardwareSession,
    firmware_metadata,
    resolve_output,
    snapshot_dict,
)
from .metrics import analyze_decoupling_high_speed

NAME = "decoupling-high-speed"
HELP = "Validate current-loop decoupling at high motor speed."
TARGET_SPEED_RPM = 500.0


def register(parser: ArgumentParser) -> None:
    parser.add_argument(
        "--start-drive",
        action="store_true",
        help=SUPPRESS,
    )
    parser.add_argument("--timeout", type=float, default=8.0)
    parser.add_argument("--sample-divider", type=int, default=120)
    parser.add_argument("--settling-s", type=float, default=1.0)
    parser.add_argument("--dwell-s", type=float, default=0.5)
    parser.add_argument("--max-steady-iq-error-percent", type=float, default=15.0)
    parser.add_argument("--max-steady-iq-error-a", type=float, default=0.1)
    parser.add_argument("--max-steady-id-a", type=float, default=0.2)
    parser.add_argument("--max-steady-iq-ripple-a", type=float, default=0.15)
    parser.add_argument("--max-peak-iq-a", type=float, default=1.5)
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("artifacts/e2e/decoupling-high-speed"),
    )


def run(args: Namespace, paths: ProjectPaths) -> None:
    if not 1 <= args.sample_divider <= 255:
        raise ValueError("--sample-divider must be in range 1..255.")
    if args.timeout <= 0:
        raise ValueError("--timeout must be positive.")
    if args.settling_s < 0:
        raise ValueError("--settling-s must not be negative.")
    if args.dwell_s < 0:
        raise ValueError("--dwell-s must not be negative.")
    if min(
        args.max_steady_iq_error_percent,
        args.max_steady_iq_error_a,
        args.max_steady_id_a,
        args.max_steady_iq_ripple_a,
        args.max_peak_iq_a,
    ) < 0:
        raise ValueError("Acceptance limits must not be negative.")
    output = resolve_output(paths.root, args.output)
    output.mkdir(parents=True, exist_ok=True)

    with HardwareSession(paths) as session:
        initial = session.preflight()
        current_limit = abs(session.read_engineering("refs.Iq_ref", unit="A"))
        if current_limit <= 0:
            raise RuntimeError("Positive refs.Iq_ref current limit is required.")
        hardware = session.hardware()
        with session.temporary_engineering_value(
            "sm_ctrl.run_mode", 5
        ) as original_run_mode_raw:
            with session.temporary_engineering_value(
                "refs.speed_ref", TARGET_SPEED_RPM, unit="rpm"
            ) as original_speed_ref_raw:
                before = session.verify_mode(5)
                channels = [
                    Channel(
                        "iq_ref_a",
                        "pid_iq.pid_ref_reg3",
                        session.dictionary.codec(
                            "pid_iq.pid_ref_reg3", unit="A"
                        ),
                    ),
                    Channel(
                        "iq_a",
                        "pid_iq.pid_fdb_reg3",
                        session.dictionary.codec(
                            "pid_iq.pid_fdb_reg3", unit="A"
                        ),
                    ),
                    Channel(
                        "id_a",
                        "pid_id.pid_fdb_reg3",
                        session.dictionary.codec(
                            "pid_id.pid_fdb_reg3", unit="A"
                        ),
                    ),
                    Channel(
                        "rotor_speed_rpm",
                        "cur_par.speed",
                        session.dictionary.codec(
                            "cur_par.speed", unit="rpm"
                        ),
                    ),
                ]
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

    metrics = analyze_decoupling_high_speed(
        capture.samples,
        target_speed_rpm=TARGET_SPEED_RPM,
        settling_s=args.settling_s,
        dwell_s=args.dwell_s,
        max_steady_iq_error_percent=args.max_steady_iq_error_percent,
        max_steady_iq_error_a=args.max_steady_iq_error_a,
        max_steady_id_a=args.max_steady_id_a,
        max_steady_iq_ripple_a=args.max_steady_iq_ripple_a,
        max_peak_iq_a=args.max_peak_iq_a,
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
    metrics["criteria"]["capture_window_covers_settling"] = (
        metrics["window_s"]
        >= args.settling_s + args.dwell_s + 0.5
    )
    metrics["passed"] = all(metrics["criteria"].values())

    csv_path = output / "capture.csv"
    png_path = output / "high-speed-decoupling.png"
    json_path = output / "report.json"
    write_csv(csv_path, capture.samples)
    plot_decoupling_high_speed(png_path, capture.samples, target_speed_rpm=TARGET_SPEED_RPM)
    report = {
        "test": NAME,
        "firmware": firmware_metadata(paths),
        "hardware": hardware,
        "setup": {
            "required_run_mode": 5,
            "target_speed_rpm": TARGET_SPEED_RPM,
            "current_limit_a": current_limit,
            "settling_s": args.settling_s,
            "dwell_s": args.dwell_s,
            "trigger_bit": 4,
            "trigger_timeout_s": args.timeout,
            "sample_divider": args.sample_divider,
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
        raise RuntimeError(
            "High-speed decoupling acceptance criteria failed."
        )
