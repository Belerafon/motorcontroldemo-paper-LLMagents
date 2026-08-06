from __future__ import annotations

import json
from argparse import ArgumentParser, Namespace, SUPPRESS
from pathlib import Path

from motor_debug_core.datalogger import Channel
from motor_debug_core.project import ProjectPaths

from .artifacts import plot_svpwm_cont, write_csv, write_json
from .common import (
    HardwareSession,
    firmware_metadata,
    resolve_output,
    snapshot_dict,
)
from .metrics import analyze_svpwm_cont


NAME = "svpwm-cont"
HELP = (
    "Validate the continuous (symmetric, min-max) SVPWM type on the motor "
    "model: spin-up to 500 rpm and prove no phase is clamped to a DC bus."
)
TARGET_SPEED_RPM = 500.0
PWM_TYPE_SVPWM_CONT = 6


def register(parser: ArgumentParser) -> None:
    parser.add_argument(
        "--start-drive",
        action="store_true",
        help=SUPPRESS,
    )
    parser.add_argument("--timeout", type=float, default=6.0)
    parser.add_argument("--sample-divider", type=int, default=120)
    parser.add_argument("--max-steady-error-percent", type=float, default=3.0)
    parser.add_argument("--max-speed-ripple-rpm", type=float, default=25.0)
    parser.add_argument("--min-rail-distance-percent", type=float, default=2.0)
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("artifacts/e2e/svpwm-cont"),
    )


def run(args: Namespace, paths: ProjectPaths) -> None:
    if not 1 <= args.sample_divider <= 255:
        raise ValueError("--sample-divider must be in range 1..255.")
    if args.timeout <= 0:
        raise ValueError("--timeout must be positive.")
    if min(
        args.max_steady_error_percent,
        args.max_speed_ripple_rpm,
        args.min_rail_distance_percent,
    ) < 0:
        raise ValueError("Acceptance limits must not be negative.")
    output = resolve_output(paths.root, args.output)
    output.mkdir(parents=True, exist_ok=True)

    with HardwareSession(paths) as session:
        initial = session.preflight()
        hardware = session.hardware()
        # read original PWM type (raw dictionary int) for reporting + restore check
        original_pwm_type = session.target.read_u32("pwm.PWM_type")

        with session.temporary_engineering_value(
            "pwm.PWM_type", PWM_TYPE_SVPWM_CONT
        ) as pwm_type_raw:
            with session.temporary_engineering_value(
                "sm_ctrl.run_mode", 5
            ) as original_run_mode_raw:
                with session.temporary_engineering_value(
                    "refs.speed_ref", TARGET_SPEED_RPM, unit="rpm"
                ) as original_speed_ref_raw:
                    before = session.verify_mode(5)
                    k_pwm = session.target.read_u32("pwm.k_pwm")
                    if k_pwm <= 0:
                        raise RuntimeError(
                            f"Invalid PWM timer period k_pwm={k_pwm}."
                        )
                    channels = [
                        Channel(
                            "speed_rpm",
                            "cur_par.speed",
                            session.dictionary.codec(
                                "cur_par.speed", unit="rpm"
                            ),
                        ),
                        Channel(
                            "gamma_a",
                            "pwm.GammaA",
                            session.dictionary.codec("pwm.GammaA", unit="ticks"),
                        ),
                        Channel(
                            "gamma_b",
                            "pwm.GammaB",
                            session.dictionary.codec("pwm.GammaB", unit="ticks"),
                        ),
                        Channel(
                            "gamma_c",
                            "pwm.GammaC",
                            session.dictionary.codec("pwm.GammaC", unit="ticks"),
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
        restored_pwm_type = session.target.read_u32("pwm.PWM_type")
        if (
            restored.run_mode != original_run_mode_raw
            or restored.speed_ref_raw != original_speed_ref_raw
            or restored_pwm_type != original_pwm_type
        ):
            raise RuntimeError(
                "Temporary SVPWM test settings (PWM type, mode or speed "
                "reference) were not restored."
            )

    metrics = analyze_svpwm_cont(
        capture.samples,
        k_pwm=k_pwm,
        target_speed_rpm=TARGET_SPEED_RPM,
        max_steady_error_percent=args.max_steady_error_percent,
        max_speed_ripple_rpm=args.max_speed_ripple_rpm,
        min_rail_distance_percent=args.min_rail_distance_percent,
    )
    metrics["criteria"]["runtime_entered_vector_mode"] = (
        capture.runtime.control_state == 5 and capture.runtime.run_mode == 5
    )
    metrics["criteria"]["pwm_type_restored"] = restored_pwm_type == original_pwm_type
    metrics["criteria"]["run_mode_restored"] = (
        restored.run_mode == original_run_mode_raw
    )
    metrics["criteria"]["speed_reference_restored"] = (
        restored.speed_ref_raw == original_speed_ref_raw
    )
    metrics["criteria"]["capture_window_covers_limits"] = (
        metrics["window_s"] >= 2.0 + 0.1
    )
    metrics["passed"] = all(metrics["criteria"].values())

    csv_path = output / "capture.csv"
    png_path = output / "svpwm-cont.png"
    json_path = output / "report.json"
    write_csv(csv_path, capture.samples)
    plot_svpwm_cont(png_path, capture.samples, k_pwm)
    report = {
        "test": NAME,
        "firmware": firmware_metadata(paths),
        "hardware": hardware,
        "setup": {
            "required_run_mode": 5,
            "pwm_type": PWM_TYPE_SVPWM_CONT,
            "pwm_type_name": "PWM_TYPE_SVPWM_CONT (continuous symmetric SVPWM)",
            "target_speed_rpm": TARGET_SPEED_RPM,
            "k_pwm_ticks": k_pwm,
            "trigger_bit": 4,
            "trigger_timeout_s": args.timeout,
            "sample_divider": args.sample_divider,
            "original_pwm_type_raw": original_pwm_type,
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
        raise RuntimeError("Continuous SVPWM acceptance criteria failed.")
