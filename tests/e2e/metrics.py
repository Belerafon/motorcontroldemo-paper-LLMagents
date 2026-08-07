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


def _constant_power_series(
    samples: Sequence[Any],
    *,
    speed_nom_rpm: float,
    i_nom_a: float,
    speed_lo_rpm: float,
    speed_hi_rpm: float,
    saturation_ratio: float = 0.08,
) -> dict[str, Any]:
    """Collect speed/current samples inside the power-limit speed window."""
    speed = _series(samples, "speed_rpm")
    iq_ref = _series(samples, "iq_ref_a")
    limit = _series(samples, "limit_a")
    keep = []
    for index, value in enumerate(speed):
        if not (speed_lo_rpm <= value <= speed_hi_rpm):
            continue
        if limit[index] <= 0:
            continue
        deviation = abs(iq_ref[index] - limit[index]) / limit[index]
        if deviation > saturation_ratio:
            continue
        keep.append(index)
    if not keep:
        return {"keep": keep, "speed": [], "iq_ref": [], "limit": [], "power_pu": []}
    power = [
        (speed[index] / speed_nom_rpm) * (abs(iq_ref[index]) / i_nom_a)
        for index in keep
    ]
    return {
        "keep": keep,
        "speed": [speed[index] for index in keep],
        "iq_ref": [iq_ref[index] for index in keep],
        "limit": [limit[index] for index in keep],
        "power_pu": power,
    }


def analyze_constant_power(
    samples: Sequence[Any],
    *,
    mode: str,
    speed_nom_rpm: float,
    i_nom_a: float,
    iq_current_limit_a: float,
    power_set_pu: float,
    speed_lo_rpm: float,
    speed_hi_rpm: float,
    max_power_cv_percent: float = 12.0,
    power_match_percent: float = 15.0,
    min_speed_span_rpm: float = 40.0,
    current_drop_ratio: float = 0.90,
    saturation_error_percent: float = 8.0,
    torque_tolerance_percent: float = 10.0,
    min_power_growth_ratio: float = 1.15,
    min_window_samples: int = 10,
) -> dict[str, Any]:
    if mode not in ("power", "torque"):
        raise ValueError("mode must be 'power' or 'torque'.")
    series = _constant_power_series(
        samples,
        speed_nom_rpm=speed_nom_rpm,
        i_nom_a=i_nom_a,
        speed_lo_rpm=speed_lo_rpm,
        speed_hi_rpm=speed_hi_rpm,
    )
    power = series["power_pu"]
    iq_ref = series["iq_ref"]
    limit = series["limit"]
    n_window = len(power)
    if n_window:
        span = max(series["speed"]) - min(series["speed"])
        power_mean = _mean(power)
        power_std = math.sqrt(_mean([(value - power_mean) ** 2 for value in power]))
        power_cv = power_std / power_mean * 100.0 if power_mean else math.inf
        third = max(1, n_window // 4)
        head = _mean(power[:third])
        tail = _mean(power[-third:])
        power_growth_ratio = tail / head if head else math.inf
        iq_start = _mean(iq_ref[:third])
        iq_end = _mean(iq_ref[-third:])
        iq_drop_ratio = iq_end / iq_start if iq_start else math.inf
        median_iq = float(statistics.median(iq_ref))
        saturation_error = _mean(
            [abs(ref - lim) / lim * 100.0 for ref, lim in zip(iq_ref, limit)]
        )
    else:
        span = 0.0
        power_mean = 0.0
        power_std = 0.0
        power_cv = math.inf
        power_growth_ratio = math.inf
        iq_drop_ratio = 0.0
        median_iq = 0.0
        saturation_error = 0.0
    torque_ratio = median_iq / iq_current_limit_a if iq_current_limit_a else 0.0
    power_match = (
        abs(power_mean - power_set_pu) / power_set_pu * 100.0
        if power_set_pu
        else math.inf
    )
    metrics: dict[str, Any] = {
        "mode": mode,
        "window_samples": n_window,
        "window_speed_span_rpm": span,
        "power_mean_pu": power_mean,
        "power_std_pu": power_std,
        "power_cv_percent": power_cv,
        "power_set_pu": power_set_pu,
        "power_match_percent": power_match,
        "power_growth_ratio": power_growth_ratio,
        "iq_reference_median_a": median_iq,
        "iq_current_limit_a": iq_current_limit_a,
        "torque_ratio_to_limit": torque_ratio,
        "iq_drop_ratio_over_window": iq_drop_ratio,
        "saturation_error_percent": saturation_error,
    }
    limits = {
        "max_power_cv_percent": max_power_cv_percent,
        "power_match_percent": power_match_percent,
        "min_speed_span_rpm": min_speed_span_rpm,
        "current_drop_ratio": current_drop_ratio,
        "saturation_error_percent": saturation_error_percent,
        "torque_tolerance_percent": torque_tolerance_percent,
        "min_power_growth_ratio": min_power_growth_ratio,
        "min_window_samples": min_window_samples,
        "speed_lo_rpm": speed_lo_rpm,
        "speed_hi_rpm": speed_hi_rpm,
        "speed_nom_rpm": speed_nom_rpm,
        "i_nom_a": i_nom_a,
    }
    criteria = {
        "enough_window_samples": n_window >= min_window_samples,
        "speed_span_reached": span >= min_speed_span_rpm,
        "power_limited_saturated": saturation_error <= saturation_error_percent,
    }
    if mode == "power":
        criteria.update(
            {
                "power_held_constant": power_cv <= max_power_cv_percent,
                "power_matches_limit": power_match <= power_match_percent,
                "current_ramps_down": iq_drop_ratio <= current_drop_ratio,
            }
        )
    else:
        criteria.update(
            {
                "torque_held_at_limit": abs(torque_ratio - 1.0)
                <= torque_tolerance_percent / 100.0,
                "power_grows_with_speed": power_growth_ratio
                >= min_power_growth_ratio,
            }
        )
    metrics["limits"] = limits
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
