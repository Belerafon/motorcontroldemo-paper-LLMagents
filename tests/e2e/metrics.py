from __future__ import annotations

import math
import statistics
from collections.abc import Sequence
from typing import Any


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


def analyze_constant_power(
    samples: Sequence[Any],
    *,
    target_speed_rpm: float,
    power_limit_pu: float,
    speed_base_rpm: float = 500.0,
    current_base_a: float = 5.0,
    max_time_to_95_s: float = 2.5,
    max_power_exceed_percent: float = 15.0,
    engage_tolerance_percent: float = 25.0,
    max_power_relative_std_percent: float = 15.0,
    max_power_offset_percent: float = 20.0,
    max_current_shape_factor: float = 0.85,
    window_base_speed_frac: float = 0.35,
    window_top_speed_frac: float = 0.9,
) -> dict[str, Any]:
    """Validate constant-power acceleration against a settable power limit.

    The firmware limits the q-current reference to ``power_limit / speed``,
    so during acceleration ``speed_pu * iq_pu`` stays close to the limit.
    Power is computed independently from the logged speed and q-current, and
    the firmware's own power estimate (channel ``power_pu``) is reported too.
    """
    if power_limit_pu <= 0:
        raise ValueError("power_limit_pu must be positive.")
    if not 0.0 < window_base_speed_frac < window_top_speed_frac <= 1.0:
        raise ValueError("Speed window must satisfy base < top and fit in (0, 1].")

    time_s = _times(samples)
    speed_rpm = _series(samples, "speed_rpm")
    speed_ref = _series(samples, "speed_ref_rpm")
    iq_a = _series(samples, "iq_a")
    channel_names = samples[0].values
    power_pu = (
        _series(samples, "power_pu")
        if "power_pu" in channel_names
        else [0.0] * len(samples)
    )
    speed_pu = [value / speed_base_rpm for value in speed_rpm]
    iq_pu = [value / current_base_a for value in iq_a]
    power_actual_pu = [
        sp * iq for sp, iq in zip(speed_pu, iq_pu)
    ]
    period = _sample_period(time_s)
    start = _event_start(speed_ref, target_speed_rpm)

    i95 = _first_crossing(speed_rpm, target_speed_rpm, 0.95, start)
    time_to_95 = None if i95 is None else float(time_s[i95] - time_s[start])

    window = [
        index
        for index, sp in enumerate(speed_pu)
        if window_base_speed_frac <= sp <= window_top_speed_frac
    ]
    if not window:
        raise ValueError(
            "No samples fell inside the constant-power speed window; "
            "the acceleration profile cannot be validated."
        )
    window_power = [power_actual_pu[index] for index in window]
    peak_power = max(window_power)
    mean_power = _mean(window_power)
    std_power = float(statistics.pstdev(window_power))
    relative_std = std_power / mean_power * 100.0 if mean_power else math.inf
    offset_percent = abs(mean_power - power_limit_pu) / power_limit_pu * 100.0
    engaged = peak_power >= power_limit_pu * (1.0 - engage_tolerance_percent / 100.0)
    bounded = peak_power <= power_limit_pu * (1.0 + max_power_exceed_percent / 100.0)

    iq_base = _mean([iq_pu[index] for index in window if speed_pu[index] <=
                     (window_base_speed_frac + window_top_speed_frac) / 2.0])
    iq_top = _mean([iq_pu[index] for index in window if speed_pu[index] >=
                    (window_base_speed_frac + window_top_speed_frac) / 2.0])
    current_shape_factor = (iq_top / iq_base) if iq_base else math.inf

    overall_peak_power = max(power_actual_pu)
    metrics: dict[str, Any] = {
        "samples": len(samples),
        "sample_period_s": period,
        "window_s": float(time_s[-1] - time_s[0]),
        "target_speed_rpm": target_speed_rpm,
        "power_limit_pu": power_limit_pu,
        "speed_base_rpm": speed_base_rpm,
        "current_base_a": current_base_a,
        "time_to_95_percent_s": time_to_95,
        "speed_window": [window_base_speed_frac, window_top_speed_frac],
        "window_sample_count": len(window),
        "power_pu_overall_peak": overall_peak_power,
        "power_pu_window_peak": peak_power,
        "power_pu_window_mean": mean_power,
        "power_pu_window_stddev": std_power,
        "power_pu_window_relative_std_percent": relative_std,
        "power_pu_offset_from_limit_percent": offset_percent,
        "power_limit_engaged": engaged,
        "iq_pu_window_low_speed_mean": iq_base,
        "iq_pu_window_high_speed_mean": iq_top,
        "iq_pu_shape_factor": current_shape_factor,
        "firmware_power_pu_window_mean": _mean(
            [power_pu[index] for index in window]
        ),
    }
    criteria = {
        "enough_samples": len(samples) >= 64,
        "window_covered": len(window) >= 16,
        "reached_95_percent": (
            time_to_95 is not None and time_to_95 <= max_time_to_95_s
        ),
        "power_limit_engaged": engaged,
        "power_bounded_by_limit": bounded,
        "power_constant": (
            relative_std <= max_power_relative_std_percent
            and offset_percent <= max_power_offset_percent
        ),
        "current_drops_with_speed": current_shape_factor <= max_current_shape_factor,
    }
    metrics["limits"] = {
        "power_limit_pu": power_limit_pu,
        "max_time_to_95_s": max_time_to_95_s,
        "max_power_exceed_percent": max_power_exceed_percent,
        "engage_tolerance_percent": engage_tolerance_percent,
        "max_power_relative_std_percent": max_power_relative_std_percent,
        "max_power_offset_percent": max_power_offset_percent,
        "max_current_shape_factor": max_current_shape_factor,
        "allowed_peak_power_pu": power_limit_pu
        * (1.0 + max_power_exceed_percent / 100.0),
    }
    metrics["criteria"] = criteria
    metrics["passed"] = all(criteria.values())
    return metrics
