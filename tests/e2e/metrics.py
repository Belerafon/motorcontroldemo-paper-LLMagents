from __future__ import annotations

import math
import statistics
from collections.abc import Sequence
from typing import Any


def expected_voltage_limit(
    udc_meas: float, u_lim: float, floor: float = 0.05
) -> float:
    """Dynamic current-controller output limit in per-unit.

    Mirrors the firmware: the current regulators may command no more phase
    voltage than the inverter can physically deliver at the measured DC-link
    voltage (adc.Udc_meas, in per-unit of Udc_nom), and never more than the
    PWM modulation can realize (pwm.U_lim). Hence the limit is the smaller of
    the two. A small floor guards against overflow inside the regulators.
    """
    return max(floor, min(udc_meas, u_lim))


def analyze_dynamic_limit(
    points: dict[str, dict[str, float]],
    *,
    floor: float = 0.05,
    tolerance: float = 1e-3,
) -> dict[str, Any]:
    """Validate that pid_out_max/min track udc_meas * u_lim at every point.

    `points` maps a label to `{"udc_meas": float, "u_lim": float,
    "out_max": float, "out_min": float}` in per-unit. Both the positive and
    negative limits must match the expected clamped value.
    """
    criteria: dict[str, bool] = {}
    errors: dict[str, float] = {}
    for name, values in points.items():
        expected = expected_voltage_limit(
            values["udc_meas"], values["u_lim"], floor=floor
        )
        error = max(
            abs(values["out_max"] - expected),
            abs(values["out_min"] - (-expected)),
        )
        errors[name] = error
        criteria[name] = error <= tolerance
    return {
        "floor": floor,
        "tolerance": tolerance,
        "errors": errors,
        "criteria": criteria,
        "passed": all(criteria.values()),
    }


def _series(samples: Sequence[Any], name: str) -> list[float]:
    if not samples:
        raise ValueError("At least one sample is required.")
    return [float(sample.values[name]) for sample in samples]


def _times(samples: Sequence[Any]) -> list[float]:
    return [float(sample.time_s) for sample in samples]


def _mean(values: Sequence[float]) -> float:
    return float(statistics.fmean(values))


def _rms(values: Sequence[float]) -> float:
    return math.sqrt(_mean([value * value for value in values]))


def _rmse(actual: Sequence[float], expected: Sequence[float]) -> float:
    return _rms([left - right for left, right in zip(actual, expected)])


def _tail_start(length: int, fraction: float = 0.2) -> int:
    return max(0, length - max(10, round(length * fraction)))


def _sample_period(time_s: Sequence[float]) -> float:
    if len(time_s) < 2:
        raise ValueError("At least two timestamps are required.")
    return float(statistics.median(b - a for a, b in zip(time_s, time_s[1:])))


def _first_crossing(
    values: Sequence[float], target: float, fraction: float, start: int
) -> int | None:
    direction = 1.0 if target >= 0 else -1.0
    threshold = abs(target) * fraction
    for index in range(start, len(values)):
        if direction * values[index] >= threshold:
            return index
    return None


def _event_start(reference: Sequence[float], target: float) -> int:
    threshold = max(abs(target) * 0.01, 0.01)
    for index, value in enumerate(reference):
        if abs(value) >= threshold:
            return index
    return 0


def _settling_time(
    time_s: Sequence[float],
    values: Sequence[float],
    target: float,
    tolerance: float,
    dwell_s: float,
    start: int,
) -> float | None:
    period = _sample_period(time_s)
    dwell_points = max(1, math.ceil(dwell_s / period))
    stop = len(values) - dwell_points + 1
    for index in range(start, max(start, stop)):
        if all(abs(value - target) <= tolerance for value in values[index:]):
            return float(time_s[index] - time_s[start])
    return None


def _trapz_abs_error(
    time_s: Sequence[float],
    actual: Sequence[float],
    expected: Sequence[float],
    start: int,
) -> float:
    total = 0.0
    for index in range(start + 1, len(time_s)):
        left = abs(actual[index - 1] - expected[index - 1])
        right = abs(actual[index] - expected[index])
        total += 0.5 * (left + right) * (time_s[index] - time_s[index - 1])
    return float(total)


def analyze_current_mode4(
    samples: Sequence[Any],
    *,
    max_id_error_percent: float = 5.0,
    max_id_error_a: float = 0.05,
    max_iq_rms_a: float = 0.10,
    max_iq_bias_a: float = 0.05,
    max_overshoot_percent: float = 20.0,
    max_settling_s: float = 0.05,
    max_rotor_speed_rpm: float = 5.0,
    min_test_current_a: float = 0.10,
    settling_dwell_s: float = 0.01,
) -> dict[str, Any]:
    time_s = _times(samples)
    id_ref = _series(samples, "id_ref_a")
    id_actual = _series(samples, "id_a")
    channel_names = samples[0].values
    iq_ref = (
        _series(samples, "iq_ref_a")
        if "iq_ref_a" in channel_names
        else [0.0] * len(samples)
    )
    iq_actual = _series(samples, "iq_a")
    rotor_speed = (
        _series(samples, "rotor_speed_rpm")
        if "rotor_speed_rpm" in channel_names
        else None
    )
    period = _sample_period(time_s)
    tail_start = _tail_start(len(samples))
    target = float(statistics.median(id_ref[tail_start:]))
    start = _event_start(id_ref, target)
    direction = 1.0 if target >= 0 else -1.0
    steady_id = id_actual[tail_start:]
    steady_id_ref = id_ref[tail_start:]
    steady_iq = iq_actual[tail_start:]
    i10 = _first_crossing(id_actual, target, 0.10, start)
    i90 = _first_crossing(id_actual, target, 0.90, start)
    peak_id = max(direction * value for value in id_actual[start:])
    overshoot_percent = (
        max(0.0, peak_id - abs(target)) / abs(target) * 100.0
        if target
        else math.inf
    )
    tolerance = max(max_id_error_a, abs(target) * max_id_error_percent / 100.0)
    settling = _settling_time(
        time_s,
        id_actual,
        target,
        tolerance,
        settling_dwell_s,
        start,
    )
    steady_error = _mean(
        [actual - expected for actual, expected in zip(steady_id, steady_id_ref)]
    )
    steady_abs_error = abs(steady_error)
    steady_error_percent = (
        steady_abs_error / abs(target) * 100.0 if target else math.inf
    )
    iq_bias = _mean(steady_iq)
    iq_rms = _rms(steady_iq)
    peak_rotor_speed = (
        max(abs(value) for value in rotor_speed)
        if rotor_speed is not None
        else None
    )
    metrics: dict[str, Any] = {
        "samples": len(samples),
        "sample_period_s": period,
        "window_s": float(time_s[-1] - time_s[0]),
        "id_target_a": target,
        "id_steady_a": _mean(steady_id),
        "id_steady_error_a": steady_abs_error,
        "id_steady_error_percent": steady_error_percent,
        "id_steady_rmse_a": _rmse(steady_id, steady_id_ref),
        "id_rise_time_10_90_s": (
            None
            if i10 is None or i90 is None
            else float(time_s[i90] - time_s[i10])
        ),
        "id_overshoot_percent": overshoot_percent,
        "id_settling_time_s": settling,
        "iq_reference_peak_abs_a": max(abs(value) for value in iq_ref),
        "iq_steady_bias_a": iq_bias,
        "iq_steady_rms_a": iq_rms,
        "iq_steady_peak_abs_a": max(abs(value) for value in steady_iq),
        "dq_cross_axis_rms_percent": (
            iq_rms / abs(target) * 100.0 if target else math.inf
        ),
        "peak_abs_rotor_speed_rpm": peak_rotor_speed,
        "steady_rotor_speed_rms_rpm": (
            _rms(rotor_speed[tail_start:])
            if rotor_speed is not None
            else None
        ),
    }
    criteria = {
        "enough_samples": len(samples) >= 64,
        "id_excitation_present": abs(target) >= min_test_current_a,
        "iq_reference_is_zero": metrics["iq_reference_peak_abs_a"] <= 0.01,
        "id_steady_error": steady_abs_error <= tolerance,
        "id_overshoot": overshoot_percent <= max_overshoot_percent,
        "id_settled": settling is not None and settling <= max_settling_s,
        "iq_bias": abs(iq_bias) <= max_iq_bias_a,
        "iq_rms": iq_rms <= max_iq_rms_a,
        "rotor_stationary": (
            peak_rotor_speed is not None
            and peak_rotor_speed <= max_rotor_speed_rpm
        ),
    }
    metrics["limits"] = {
        "id_error_tolerance_a": tolerance,
        "max_overshoot_percent": max_overshoot_percent,
        "max_settling_s": max_settling_s,
        "max_iq_bias_a": max_iq_bias_a,
        "max_iq_rms_a": max_iq_rms_a,
        "max_rotor_speed_rpm": max_rotor_speed_rpm,
        "min_test_current_a": min_test_current_a,
        "settling_dwell_s": settling_dwell_s,
    }
    metrics["criteria"] = criteria
    metrics["passed"] = all(criteria.values())
    return metrics


def analyze_vector_500(
    samples: Sequence[Any],
    *,
    target_speed_rpm: float = 500.0,
    current_limit_a: float,
    max_time_to_95_s: float = 2.0,
    max_overshoot_percent: float = 10.0,
    max_settling_s: float = 2.5,
    max_steady_error_percent: float = 2.0,
    max_speed_ripple_rpm: float = 25.0,
    current_limit_margin_percent: float = 25.0,
    settling_dwell_s: float = 0.10,
) -> dict[str, Any]:
    time_s = _times(samples)
    speed = _series(samples, "speed_rpm")
    speed_ref = _series(samples, "speed_ref_rpm")
    iq = _series(samples, "iq_a")
    iq_ref = _series(samples, "iq_ref_a")
    period = _sample_period(time_s)
    tail_start = _tail_start(len(samples))
    start = _event_start(speed_ref, target_speed_rpm)
    direction = 1.0 if target_speed_rpm >= 0 else -1.0
    i10 = _first_crossing(speed, target_speed_rpm, 0.10, start)
    i90 = _first_crossing(speed, target_speed_rpm, 0.90, start)
    i95 = _first_crossing(speed, target_speed_rpm, 0.95, start)
    peak_speed = max(direction * value for value in speed[start:])
    overshoot_percent = (
        max(0.0, peak_speed - abs(target_speed_rpm))
        / abs(target_speed_rpm)
        * 100.0
    )
    speed_tolerance = max(1.0, abs(target_speed_rpm) * 0.02)
    settling = _settling_time(
        time_s,
        speed,
        target_speed_rpm,
        speed_tolerance,
        settling_dwell_s,
        start,
    )
    steady_speed = speed[tail_start:]
    steady_ref = speed_ref[tail_start:]
    steady_iq = iq[tail_start:]
    acceleration_stop = i95 if i95 is not None else len(samples) - 1
    acceleration_iq = iq[start : acceleration_stop + 1]
    derivative_window = max(1, round(0.005 / period))
    acceleration_values = [
        direction
        * (speed[index + derivative_window] - speed[index])
        / (time_s[index + derivative_window] - time_s[index])
        for index in range(start, len(speed) - derivative_window)
    ]
    steady_error = abs(_mean(steady_speed) - target_speed_rpm)
    steady_error_percent = steady_error / abs(target_speed_rpm) * 100.0
    peak_current = max(abs(value) for value in acceleration_iq)
    allowed_current = abs(current_limit_a) * (
        1.0 + current_limit_margin_percent / 100.0
    )
    final_reference = _mean(steady_ref)
    metrics: dict[str, Any] = {
        "samples": len(samples),
        "sample_period_s": period,
        "window_s": float(time_s[-1] - time_s[0]),
        "target_speed_rpm": target_speed_rpm,
        "final_speed_reference_rpm": final_reference,
        "steady_speed_rpm": _mean(steady_speed),
        "steady_speed_error_rpm": steady_error,
        "steady_speed_error_percent": steady_error_percent,
        "steady_speed_stddev_rpm": float(statistics.pstdev(steady_speed)),
        "steady_speed_ripple_peak_to_peak_rpm": max(steady_speed)
        - min(steady_speed),
        "rise_time_10_90_s": (
            None
            if i10 is None or i90 is None
            else float(time_s[i90] - time_s[i10])
        ),
        "time_to_95_percent_s": (
            None if i95 is None else float(time_s[i95] - time_s[start])
        ),
        "overshoot_percent": overshoot_percent,
        "settling_time_2_percent_s": settling,
        "integrated_absolute_speed_error_rpm_s": _trapz_abs_error(
            time_s, speed, speed_ref, start
        ),
        "steady_tracking_rmse_rpm": _rmse(steady_speed, steady_ref),
        "max_acceleration_rpm_per_s": (
            max(acceleration_values) if acceleration_values else None
        ),
        "peak_abs_iq_a_during_acceleration": peak_current,
        "rms_iq_a_during_acceleration": _rms(acceleration_iq),
        "peak_abs_iq_reference_a": max(abs(value) for value in iq_ref),
        "steady_iq_mean_a": _mean(steady_iq),
        "steady_iq_rms_a": _rms(steady_iq),
    }
    command_tolerance = max(5.0, abs(target_speed_rpm) * 0.01)
    criteria = {
        "enough_samples": len(samples) >= 64,
        "speed_reference_reached_command": (
            abs(final_reference - target_speed_rpm) <= command_tolerance
        ),
        "reached_95_percent": (
            metrics["time_to_95_percent_s"] is not None
            and metrics["time_to_95_percent_s"] <= max_time_to_95_s
        ),
        "overshoot": overshoot_percent <= max_overshoot_percent,
        "settled": settling is not None and settling <= max_settling_s,
        "steady_speed_error": steady_error_percent <= max_steady_error_percent,
        "steady_speed_ripple": (
            metrics["steady_speed_ripple_peak_to_peak_rpm"]
            <= max_speed_ripple_rpm
        ),
        "iq_within_limit_margin": (
            current_limit_a > 0 and peak_current <= allowed_current
        ),
    }
    metrics["limits"] = {
        "max_time_to_95_s": max_time_to_95_s,
        "max_overshoot_percent": max_overshoot_percent,
        "max_settling_s": max_settling_s,
        "max_steady_error_percent": max_steady_error_percent,
        "max_speed_ripple_rpm": max_speed_ripple_rpm,
        "current_limit_a": current_limit_a,
        "current_limit_margin_percent": current_limit_margin_percent,
        "allowed_peak_current_a": allowed_current,
        "settling_dwell_s": settling_dwell_s,
    }
    metrics["criteria"] = criteria
    metrics["passed"] = all(criteria.values())
    return metrics
