from __future__ import annotations

import json
import math
import statistics
from argparse import ArgumentParser, Namespace, SUPPRESS
from pathlib import Path

from motor_debug_core.datalogger import Channel
from motor_debug_core.project import ProjectPaths
from motor_debug_core.values import SignedInt32

from .artifacts import plot_pwm_svpwm, write_csv, write_json
from .common import (
    HardwareSession,
    firmware_metadata,
    resolve_output,
    snapshot_dict,
)


NAME = "pwm-svpwm"
HELP = (
    "Engage the new centered-SVPWM type (PWM_TYPE_SVPWM_CENTERED) and verify "
    "the model still reaches 500 rpm with no phase clamped to a DC bus."
)

TARGET_SPEED_RPM = 500.0
PWM_TYPE_SVPWM_CENTERED = 6
DEFAULT_K_INJ_IQ24 = 1 << 24  # _IQ(1.0): full min/max zero-sequence injection


def register(parser: ArgumentParser) -> None:
    parser.add_argument(
        "--start-drive",
        action="store_true",
        help=SUPPRESS,
    )
    parser.add_argument("--timeout", type=float, default=6.0)
    parser.add_argument("--sample-divider", type=int, default=120)
    parser.add_argument("--k-inj", type=float, default=1.0)
    parser.add_argument("--max-time-to-95-s", type=float, default=2.0)
    parser.add_argument("--max-settling-s", type=float, default=2.5)
    parser.add_argument("--max-steady-error-percent", type=float, default=2.0)
    parser.add_argument(
        "--max-steady-clamped-fraction",
        type=float,
        default=0.10,
        help=(
            "Maximum fraction of steady-state samples for which any of "
            "GammaA/GammaB/GammaC may sit on either rail (0 or k_pwm+1)."
        ),
    ),
    parser.add_argument(
        "--rail-margin-lsb",
        type=int,
        default=3,
        help=(
            "Distance in PWM LSB from {0, k_pwm+1} below which a Gamma "
            "sample is considered 'clamped to a rail'."
        ),
    ),
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("artifacts/e2e/pwm-svpwm"),
    )


def _tail_start(length: int, fraction: float = 0.2) -> int:
    return max(0, length - max(10, round(length * fraction)))


def _settling_time_s(
    time_s, values, target, tolerance, dwell_s, start
) -> float | None:
    """Mirrors metrics._settling_time: first index from `start` such that every
    subsequent sample stays within +/-tolerance of `target` for the remainder of
    the capture, with at least `dwell_s` of dwell at the end."""
    if len(time_s) < 2:
        return None
    period = (time_s[-1] - time_s[0]) / (len(time_s) - 1)
    dwell_points = max(1, math.ceil(dwell_s / period))
    stop = len(values) - dwell_points + 1
    for index in range(start, max(start, stop)):
        if all(abs(v - target) <= tolerance for v in values[index:]):
            return float(time_s[index] - time_s[start])
    return None


def _analyze(
    samples,
    *,
    target_speed_rpm: float,
    k_pwm: int,
    max_time_to_95_s: float,
    max_settling_s: float,
    max_steady_error_percent: float,
    max_steady_clamped_fraction: float,
    rail_margin_lsb: int,
    settling_dwell_s: float = 0.1,
) -> dict:
    speed = [s.values["speed_rpm"] for s in samples]
    gamma_a = [s.values["gamma_a"] for s in samples]
    gamma_b = [s.values["gamma_b"] for s in samples]
    gamma_c = [s.values["gamma_c"] for s in samples]
    time_s = [s.time_s for s in samples]

    if len(speed) < 2:
        raise RuntimeError("Capture is too short for analysis.")

    target_95 = 0.95 * target_speed_rpm
    time_to_95 = None
    for t, v in zip(time_s, speed):
        if v >= target_95:
            time_to_95 = t
            break

    tolerance = max(target_speed_rpm * max_steady_error_percent / 100.0, 1.0)
    tail_start = _tail_start(len(samples))
    steady_speed = speed[tail_start:]
    steady_mean = statistics.fmean(steady_speed)
    steady_error_percent = (
        abs(steady_mean - target_speed_rpm) / target_speed_rpm * 100.0
    )

    settling_time = _settling_time_s(
        time_s, speed, target_speed_rpm, tolerance, settling_dwell_s, start=0
    )

    high_rail = k_pwm + 1
    low_rail = 0
    steady_gammas = (
        gamma_a[tail_start:] + gamma_b[tail_start:] + gamma_c[tail_start:]
    )
    if not steady_gammas:
        raise RuntimeError("Capture is too short for steady-state Gamma analysis.")
    rail_clamped = [
        (g <= low_rail + rail_margin_lsb) or (g >= high_rail - rail_margin_lsb)
        for g in steady_gammas
    ]
    clamped_fraction = sum(1 for x in rail_clamped if x) / len(rail_clamped)
    gamma_min = min(steady_gammas)
    gamma_max = max(steady_gammas)

    criteria = {
        "reached_95_percent": (
            time_to_95 is not None and time_to_95 <= max_time_to_95_s
        ),
        "settled_in_time": (
            settling_time is not None and settling_time <= max_settling_s
        ),
        "steady_error_within_budget": steady_error_percent
        <= max_steady_error_percent,
        "no_persistent_rail_clamp": clamped_fraction
        <= max_steady_clamped_fraction,
        "gammas_leave_both_rails": (
            gamma_min > low_rail + rail_margin_lsb
            and gamma_max < high_rail - rail_margin_lsb
        ),
    }
    return {
        "target_speed_rpm": target_speed_rpm,
        "time_to_95_s": time_to_95,
        "settling_time_s": settling_time,
        "settling_dwell_s": settling_dwell_s,
        "steady_mean_speed_rpm": steady_mean,
        "steady_error_percent": steady_error_percent,
        "k_pwm": k_pwm,
        "gamma_min_steady": gamma_min,
        "gamma_max_steady": gamma_max,
        "rail_clamped_fraction_steady": clamped_fraction,
        "max_steady_clamped_fraction": max_steady_clamped_fraction,
        "rail_margin_lsb": rail_margin_lsb,
        "criteria": criteria,
        "passed": all(criteria.values()),
    }


def run(args: Namespace, paths: ProjectPaths) -> None:
    if not 1 <= args.sample_divider <= 255:
        raise ValueError("--sample-divider must be in range 1..255.")
    if args.timeout <= 0:
        raise ValueError("--timeout must be positive.")
    if not 0.0 <= args.k_inj <= 1.0:
        raise ValueError("--k-inj must be in range 0..1.")
    if min(
        args.max_time_to_95_s,
        args.max_settling_s,
        args.max_steady_error_percent,
        args.max_steady_clamped_fraction,
    ) < 0:
        raise ValueError("Acceptance limits must not be negative.")
    if args.rail_margin_lsb < 0:
        raise ValueError("--rail-margin-lsb must not be negative.")
    output = resolve_output(paths.root, args.output)
    output.mkdir(parents=True, exist_ok=True)

    k_inj_raw = round(args.k_inj * (1 << 24)) & 0xFFFF_FFFF

    with HardwareSession(paths) as session:
        initial = session.preflight()
        original_pwm_type = session.target.read_u16("pwm.PWM_type")
        original_debug1 = session.target.read_u32("Debug1")
        original_debug2 = session.target.read_u32("Debug2")
        current_limit = abs(session.read_engineering("refs.Iq_ref", unit="A"))
        if current_limit <= 0:
            raise RuntimeError("Positive refs.Iq_ref current limit is required.")
        ramp_time = session.read_engineering("rmp.T", unit="s")
        hardware = session.hardware()
        k_pwm = session.target.read_u16("pwm.k_pwm")
        if not 100 <= k_pwm <= 0xFFFF:
            raise RuntimeError(f"Unexpected pwm.k_pwm value: {k_pwm}.")

        speed_codec = session.dictionary.codec("cur_par.speed", unit="rpm")
        gamma_codec = SignedInt32(scale=1.0)
        channels = [
            Channel("speed_rpm", "cur_par.speed", speed_codec),
            Channel("gamma_a", "pwm.GammaA", gamma_codec),
            Channel("gamma_b", "pwm.GammaB", gamma_codec),
            Channel("gamma_c", "pwm.GammaC", gamma_codec),
        ]

        try:
            session.target.write_u16("pwm.PWM_type", PWM_TYPE_SVPWM_CENTERED)
            session.target.write_u32("Debug1", k_inj_raw)
            applied_pwm_type = session.target.read_u16("pwm.PWM_type")
            applied_debug1 = session.target.read_u32("Debug1")
            if applied_pwm_type != PWM_TYPE_SVPWM_CENTERED:
                raise RuntimeError(
                    "Could not select PWM_TYPE_SVPWM_CENTERED: "
                    f"read back 0x{applied_pwm_type:04X}."
                )
            if applied_debug1 != k_inj_raw:
                raise RuntimeError(
                    "Could not write Debug1 injection coefficient: "
                    f"read back 0x{applied_debug1:08X}."
                )

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
        finally:
            session.drive.stop()
            session.target.write_u32("Debug2", original_debug2)
            session.target.write_u32("Debug1", original_debug1)
            session.target.write_u16("pwm.PWM_type", original_pwm_type)
            restored_pwm_type = session.target.read_u16("pwm.PWM_type")
            restored_debug1 = session.target.read_u32("Debug1")
            restored_debug2 = session.target.read_u32("Debug2")
            if restored_pwm_type != original_pwm_type:
                raise RuntimeError(
                    "PWM_type was not restored after the test: "
                    f"0x{restored_pwm_type:04X} != 0x{original_pwm_type:04X}."
                )
            if restored_debug1 != original_debug1 or restored_debug2 != original_debug2:
                raise RuntimeError("Debug variables were not restored after the test.")

        restored = session.drive.snapshot()
        if (
            restored.run_mode != original_run_mode_raw
            or restored.speed_ref_raw != original_speed_ref_raw
        ):
            raise RuntimeError("Mode or speed reference was not restored after the test.")

    metrics = _analyze(
        capture.samples,
        target_speed_rpm=TARGET_SPEED_RPM,
        k_pwm=k_pwm,
        max_time_to_95_s=args.max_time_to_95_s,
        max_settling_s=args.max_settling_s,
        max_steady_error_percent=args.max_steady_error_percent,
        max_steady_clamped_fraction=args.max_steady_clamped_fraction,
        rail_margin_lsb=args.rail_margin_lsb,
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
    metrics["criteria"]["pwm_type_restored"] = restored_pwm_type == original_pwm_type
    metrics["criteria"]["debug_state_restored"] = (
        restored_debug1 == original_debug1 and restored_debug2 == original_debug2
    )
    window_s = (
        capture.samples[-1].time_s - capture.samples[0].time_s
        if capture.samples
        else 0.0
    )
    metrics["criteria"]["capture_window_covers_limits"] = (
        window_s >= max(args.max_time_to_95_s, args.max_settling_s) + 0.1
    )
    metrics["passed"] = all(metrics["criteria"].values())

    csv_path = output / "capture.csv"
    png_path = output / "pwm-svpwm.png"
    json_path = output / "report.json"
    write_csv(csv_path, capture.samples)
    plot_pwm_svpwm(png_path, capture.samples, k_pwm=k_pwm)
    report = {
        "test": NAME,
        "firmware": firmware_metadata(paths),
        "hardware": hardware,
        "setup": {
            "required_run_mode": 5,
            "target_speed_rpm": TARGET_SPEED_RPM,
            "current_limit_a": current_limit,
            "ramp_time_s": ramp_time,
            "selected_pwm_type": PWM_TYPE_SVPWM_CENTERED,
            "selected_pwm_type_name": "PWM_TYPE_SVPWM_CENTERED",
            "zero_sequence_injection_k_inj": args.k_inj,
            "zero_sequence_injection_k_inj_raw": k_inj_raw,
            "trigger_bit": 4,
            "trigger_timeout_s": args.timeout,
            "sample_divider": args.sample_divider,
            "original_run_mode_raw": original_run_mode_raw,
            "original_speed_reference_raw": original_speed_ref_raw,
            "original_pwm_type_raw": original_pwm_type,
            "original_debug1_raw": original_debug1,
            "original_debug2_raw": original_debug2,
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
        raise RuntimeError("PWM SVPWM acceptance criteria failed.")
