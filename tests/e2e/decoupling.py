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


NAME = "decoupling"
HELP = "Validate current-regulator decoupling improvement at 500 rpm."

TARGET_SPEED_RPM = 500.0


def register(parser: ArgumentParser) -> None:
    parser.add_argument(
        "--start-drive",
        action="store_true",
        help=SUPPRESS,
    )
    parser.add_argument("--timeout", type=float, default=6.0)
    parser.add_argument("--sample-divider", type=int, default=120)
    parser.add_argument(
        "--max-degradation-percent",
        type=float,
        default=5.0,
        help="Max allowed worsening of rise time vs baseline (percent).",
    )
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
    output = resolve_output(paths.root, args.output)
    output.mkdir(parents=True, exist_ok=True)

    baseline_samples = None
    decoupled_samples = None
    baseline_metrics = None
    decoupled_metrics = None

    with HardwareSession(paths) as session:
        initial = session.preflight()
        current_limit = abs(session.read_engineering("refs.Iq_ref", unit="A"))
        if current_limit <= 0:
            raise RuntimeError("Positive refs.Iq_ref current limit is required.")
        ramp_time = session.read_engineering("rmp.T", unit="s")
        speed_controller = session.read_controller("pid_spd")
        hardware = session.hardware()

        # --- Baseline run (decoupling disabled) ---
        with session.temporary_engineering_value(
            "sm_ctrl.run_mode", 5
        ), session.temporary_engineering_value(
            "refs.speed_ref", TARGET_SPEED_RPM, unit="rpm"
        ), session.temporary_engineering_value(
            "DebugW2", 0
        ):
            before = session.verify_mode(5)
            baseline_result = session.capture_triggered(
                [
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
                ],
                divider=args.sample_divider,
                trigger_bit=4,
                timeout_s=args.timeout,
                validated_snapshot=before,
            )
        baseline_samples = baseline_result.samples
        baseline_metrics = analyze_vector_500(
            baseline_samples,
            target_speed_rpm=TARGET_SPEED_RPM,
            current_limit_a=current_limit,
        )

        # --- Decoupled run (decoupling enabled) ---
        with session.temporary_engineering_value(
            "sm_ctrl.run_mode", 5
        ), session.temporary_engineering_value(
            "refs.speed_ref", TARGET_SPEED_RPM, unit="rpm"
        ), session.temporary_engineering_value(
            "DebugW2", 1
        ):
            before = session.verify_mode(5)
            decoupled_result = session.capture_triggered(
                [
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
                ],
                divider=args.sample_divider,
                trigger_bit=4,
                timeout_s=args.timeout,
                validated_snapshot=before,
            )
        decoupled_samples = decoupled_result.samples
        decoupled_metrics = analyze_vector_500(
            decoupled_samples,
            target_speed_rpm=TARGET_SPEED_RPM,
            current_limit_a=current_limit,
        )

    # Compare baseline vs decoupled
    baseline_rise = baseline_metrics.get("rise_time_10_90_s")
    decoupled_rise = decoupled_metrics.get("rise_time_10_90_s")

    rise_degradation_percent = 0.0
    if baseline_rise is not None and decoupled_rise is not None and baseline_rise > 0:
        rise_degradation_percent = (
            (decoupled_rise - baseline_rise) / baseline_rise * 100.0
        )

    criteria = {
        "baseline_passed": baseline_metrics.get("passed", False),
        "decoupled_passed": decoupled_metrics.get("passed", False),
        "rise_time_no_significant_degradation": (
            rise_degradation_percent
            <= args.max_degradation_percent + abs(args.max_degradation_percent * 0.01)
        ),
    }

    all_passed = all(criteria.values())

    report = {
        "test": NAME,
        "firmware": firmware_metadata(paths),
        "hardware": hardware,
        "setup": {
            "target_speed_rpm": TARGET_SPEED_RPM,
            "current_limit_a": current_limit,
            "ramp_time_s": ramp_time,
            "sample_divider": args.sample_divider,
            "speed_controller": speed_controller,
            "max_degradation_percent": args.max_degradation_percent,
        },
        "baseline_metrics": baseline_metrics,
        "decoupled_metrics": decoupled_metrics,
        "comparison": {
            "baseline_rise_time_s": baseline_rise,
            "decoupled_rise_time_s": decoupled_rise,
            "rise_degradation_percent": rise_degradation_percent,
        },
        "criteria": criteria,
        "passed": all_passed,
    }

    baseline_csv = output / "baseline-capture.csv"
    decoupled_csv = output / "decoupled-capture.csv"
    baseline_png = output / "baseline-transition.png"
    decoupled_png = output / "decoupled-transition.png"
    json_path = output / "report.json"

    write_csv(baseline_csv, baseline_samples)
    write_csv(decoupled_csv, decoupled_samples)
    plot_vector_500(baseline_png, baseline_samples)
    plot_vector_500(decoupled_png, decoupled_samples)
    write_json(json_path, report)

    print(json.dumps(baseline_metrics, ensure_ascii=False, indent=2))
    print(json.dumps(decoupled_metrics, ensure_ascii=False, indent=2))
    print(f"Baseline CSV: {baseline_csv}")
    print(f"Decoupled CSV: {decoupled_csv}")
    print(f"Baseline PNG: {baseline_png}")
    print(f"Decoupled PNG: {decoupled_png}")
    print(f"JSON: {json_path}")
    print(f"Rise degradation: {rise_degradation_percent:.2f}%")
    if not all_passed:
        raise RuntimeError("Decoupling acceptance criteria failed.")
