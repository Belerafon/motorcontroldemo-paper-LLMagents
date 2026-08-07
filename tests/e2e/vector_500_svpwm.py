from __future__ import annotations

import json
from argparse import ArgumentParser, Namespace, SUPPRESS
from pathlib import Path

from motor_debug_core.datalogger import Channel
from motor_debug_core.project import ProjectPaths

from .artifacts import (
    plot_svpwm_gamma,
    plot_vector_500,
    write_csv,
    write_json,
)
from .common import (
    HardwareSession,
    firmware_metadata,
    resolve_output,
    snapshot_dict,
)
from .metrics import analyze_svpwm_gamma, analyze_vector_500


NAME = "vector-500-svpwm"
HELP = (
    "Validate vector acceleration to 500 rpm using the continuous space-vector "
    "PWM (no bus clamping, full DC-bus utilization)."
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
    parser.add_argument("--pwm-type", type=int, default=PWM_TYPE_SVPWM_CONT)
    parser.add_argument("--u-lim", type=float, default=1.0)
    parser.add_argument("--gamma-duration-s", type=float, default=4.0)
    parser.add_argument("--gamma-divider", type=int, default=120)
    parser.add_argument("--max-time-to-95-s", type=float, default=2.0)
    parser.add_argument("--max-overshoot-percent", type=float, default=10.0)
    parser.add_argument("--max-settling-s", type=float, default=2.5)
    parser.add_argument("--max-steady-error-percent", type=float, default=2.0)
    parser.add_argument("--max-speed-ripple-rpm", type=float, default=25.0)
    parser.add_argument("--current-limit-margin-percent", type=float, default=25.0)
    parser.add_argument("--min-common-mode-fraction", type=float, default=0.02)
    parser.add_argument("--max-speed-deviation-rpm", type=float, default=50.0)
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("artifacts/e2e/vector-500-svpwm"),
    )


def run(args: Namespace, paths: ProjectPaths) -> None:
    if not 1 <= args.sample_divider <= 255:
        raise ValueError("--sample-divider must be in range 1..255.")
    if not 1 <= args.gamma_divider <= 255:
        raise ValueError("--gamma-divider must be in range 1..255.")
    if args.timeout <= 0 or args.gamma_duration_s <= 0:
        raise ValueError("Timeouts and durations must be positive.")
    if args.pwm_type != PWM_TYPE_SVPWM_CONT:
        raise ValueError(
            f"Scenario requires PWM type {PWM_TYPE_SVPWM_CONT} "
            f"(continuous SVPWM), got {args.pwm_type}."
        )
    if not 0.0 < args.u_lim <= 1.0:
        raise ValueError("--u-lim must be in the open interval (0, 1].")
    if min(
        args.max_time_to_95_s,
        args.max_overshoot_percent,
        args.max_settling_s,
        args.max_steady_error_percent,
        args.max_speed_ripple_rpm,
        args.current_limit_margin_percent,
        args.min_common_mode_fraction,
        args.max_speed_deviation_rpm,
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

        vector_channels = [
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
        gamma_channels = [
            Channel(
                "gamma_a",
                "pwm.GammaA",
                session.dictionary.codec("pwm.GammaA"),
            ),
            Channel(
                "gamma_b",
                "pwm.GammaB",
                session.dictionary.codec("pwm.GammaB"),
            ),
            Channel(
                "gamma_c",
                "pwm.GammaC",
                session.dictionary.codec("pwm.GammaC"),
            ),
            Channel(
                "speed_rpm",
                "cur_par.speed",
                session.dictionary.codec("cur_par.speed", unit="rpm"),
            ),
        ]

        with session.temporary_engineering_value(
            "pwm.PWM_type", args.pwm_type
        ) as original_pwm_type_raw:
            with session.temporary_engineering_value(
                "pwm.U_lim", args.u_lim
            ) as original_u_lim_raw:
                with session.temporary_engineering_value(
                    "sm_ctrl.run_mode", 5
                ) as original_run_mode_raw:
                    with session.temporary_engineering_value(
                        "refs.speed_ref", TARGET_SPEED_RPM, unit="rpm"
                    ) as original_speed_ref_raw:
                        before = session.verify_mode(5)
                        capture = session.capture_triggered(
                            vector_channels,
                            divider=args.sample_divider,
                            trigger_bit=4,
                            timeout_s=args.timeout,
                            validated_snapshot=before,
                        )
                        before_gamma = session.verify_mode(5)
                        gamma_capture = session.capture_ring(
                            gamma_channels,
                            divider=args.gamma_divider,
                            duration_s=args.gamma_duration_s,
                            poll_period_s=0.2,
                            validated_snapshot=before_gamma,
                        )
        k_pwm = session.target.read_u32("pwm.k_pwm")
        restored = session.drive.snapshot()
        if (
            restored.run_mode != original_run_mode_raw
            or restored.speed_ref_raw != original_speed_ref_raw
        ):
            raise RuntimeError(
                "Mode or speed reference was not restored after the test."
            )
        pwm_type_restored = (
            session.target.read_u32("pwm.PWM_type") == original_pwm_type_raw
        )
        u_lim_restored = (
            session.target.read_u32("pwm.U_lim") == original_u_lim_raw
        )
        if not (pwm_type_restored and u_lim_restored):
            raise RuntimeError("PWM type or U_lim was not restored after the test.")

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
    metrics["criteria"]["runtime_entered_vector_mode"] = (
        capture.runtime.control_state == 5 and capture.runtime.run_mode == 5
    )
    gamma_metrics = analyze_svpwm_gamma(
        gamma_capture.samples,
        k_pwm=k_pwm,
        target_speed_rpm=TARGET_SPEED_RPM,
        max_speed_deviation_rpm=args.max_speed_deviation_rpm,
        min_common_mode_fraction=args.min_common_mode_fraction,
    )
    for name, passed in gamma_metrics["criteria"].items():
        metrics["criteria"][f"svpwm_{name}"] = passed
    metrics["criteria"]["pwm_type_restored"] = pwm_type_restored
    metrics["criteria"]["u_lim_restored"] = u_lim_restored
    metrics["passed"] = all(metrics["criteria"].values())

    csv_path = output / "capture.csv"
    gamma_csv_path = output / "gamma-capture.csv"
    png_path = output / "speed-transition.png"
    gamma_png_path = output / "svpwm-gamma.png"
    json_path = output / "report.json"
    write_csv(csv_path, capture.samples)
    write_csv(gamma_csv_path, gamma_capture.samples)
    plot_vector_500(png_path, capture.samples)
    plot_svpwm_gamma(gamma_png_path, gamma_capture.samples)
    report = {
        "test": NAME,
        "firmware": firmware_metadata(paths),
        "hardware": hardware,
        "setup": {
            "required_run_mode": 5,
            "target_speed_rpm": TARGET_SPEED_RPM,
            "pwm_type": args.pwm_type,
            "u_lim": args.u_lim,
            "k_pwm": k_pwm,
            "current_limit_a": current_limit,
            "ramp_time_s": ramp_time,
            "trigger_bit": 4,
            "trigger_timeout_s": args.timeout,
            "sample_divider": args.sample_divider,
            "gamma_duration_s": args.gamma_duration_s,
            "gamma_divider": args.gamma_divider,
            "speed_controller": speed_controller,
            "original_pwm_type_raw": original_pwm_type_raw,
            "original_u_lim_raw": original_u_lim_raw,
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
        "gamma_metrics": gamma_metrics,
        "artifacts": {
            "csv": csv_path.name,
            "gamma_csv": gamma_csv_path.name,
            "plot": png_path.name,
            "gamma_plot": gamma_png_path.name,
        },
    }
    write_json(json_path, report)
    print(json.dumps(metrics, ensure_ascii=False, indent=2))
    print(json.dumps(gamma_metrics, ensure_ascii=False, indent=2))
    print(f"JSON: {json_path}")
    print(f"CSV: {csv_path}")
    print(f"Gamma CSV: {gamma_csv_path}")
    print(f"PNG: {png_path}")
    print(f"Gamma PNG: {gamma_png_path}")
    if not metrics["passed"]:
        raise RuntimeError("Vector 500 rpm SVPWM acceptance criteria failed.")
