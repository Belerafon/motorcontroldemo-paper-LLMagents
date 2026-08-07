from __future__ import annotations

import math
import statistics
from collections.abc import Sequence
from typing import Any


def _series(samples: Sequence[Any], name: str) -> list[float]:
    if not samples:
        raise ValueError("At least one sample is required.")
    return [float(sample.values[name]) for sample in samples]


def _raw_series(samples: Sequence[Any], name: str) -> list[int]:
    if not samples:
        raise ValueError("At least one sample is required.")
    return [int(sample.raw[name]) for sample in samples]


def _signed(raw: int, width: int = 32) -> int:
    if raw & (1 << (width - 1)):
        return raw - (1 << width)
    return raw


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


_IQ24 = 1 << 24


def _high_speed_window(
    speed: Sequence[float], target_speed_rpm: float, fraction: float
) -> list[bool]:
    threshold = abs(target_speed_rpm) * fraction
    return [abs(value) >= threshold for value in speed]


def decouple_metric(
    samples: Sequence[Any],
    *,
    target_speed_rpm: float,
    high_speed_fraction: float = 0.5,
    speed_name: str = "speed_rpm",
    ud_name: str = "ud_v",
    iq_name: str = "iq_a",
    id_name: str = "id_a",
    dec_name: str = "dec_v",
    min_iq_pu: float = 0.05,
) -> dict[str, Any]:
    """Coupling-region metrics, including the coupling-to-output slope.

    The d-axis regulator output is ud = -K * w * Iq plus a ripple floor that
    does not correlate with the coupling w*Iq.  Regressing ud against the
    coupling isolates the cross-coupling component of ud and is robust to that
    floor.  The returned coupling_coeff is the per-unit slope A of
    ud_pu ~ A * (w_pu * Iq_pu).
    """
    speed = _series(samples, speed_name)
    ud = _series(samples, ud_name)
    speed_raw = _raw_series(samples, speed_name)
    iq_raw = _raw_series(samples, iq_name)
    ud_raw = _raw_series(samples, ud_name)
    window = _high_speed_window(speed, target_speed_rpm, high_speed_fraction)
    peak_speed = max(abs(value) for value in speed)
    mean_abs_ud = _mean([abs(value) for value, keep in zip(ud, window) if keep])
    peak_abs_ud = max(abs(value) for value in ud)
    channel_names = samples[0].values
    numerator = 0.0
    denominator = 0.0
    coupling_points = 0
    for keep, ws, wu, wi in zip(window, speed_raw, ud_raw, iq_raw):
        if not keep:
            continue
        ws_s = _signed(ws)
        wi_s = _signed(wi)
        wu_s = _signed(wu)
        w_pu = ws_s / _IQ24
        iq_pu = wi_s / _IQ24
        if abs(iq_pu) < min_iq_pu:
            continue
        ud_pu = wu_s / _IQ24
        coupling_pu = w_pu * iq_pu
        numerator += ud_pu * coupling_pu
        denominator += coupling_pu * coupling_pu
        coupling_points += 1
    coupling_coeff = (
        numerator / denominator if denominator > 0 and coupling_points else None
    )
    id_abs = (
        [abs(value) for value in _series(samples, id_name)]
        if id_name in channel_names
        else []
    )
    dec_abs = (
        [abs(value) for value in _series(samples, dec_name)]
        if dec_name in channel_names
        else []
    )
    return {
        "samples": len(samples),
        "high_speed_points": sum(window),
        "coupling_points": coupling_points,
        "coupling_coeff": coupling_coeff,
        "peak_abs_speed_rpm": peak_speed,
        "mean_abs_ud_v": mean_abs_ud,
        "peak_abs_ud_v": peak_abs_ud,
        "peak_abs_id_a": max(id_abs) if id_abs else None,
        "mean_abs_id_a": _mean(id_abs) if id_abs else None,
        "peak_abs_dec_v": max(dec_abs) if dec_abs else None,
        "mean_abs_dec_v": _mean(dec_abs) if dec_abs else None,
    }


def calibrate_decoupling_gain(
    samples: Sequence[Any],
    *,
    speed_name: str = "speed_rpm",
    ud_name: str = "ud_v",
    iq_name: str = "iq_a",
    high_speed_fraction: float = 0.5,
    max_gain: float = 2.0,
) -> float:
    """Estimate the per-unit d-axis coupling coefficient K from a capture.

    In the synchronous frame with Id ~ 0 the d-axis regulator must output
    ud = -K * w * Iq to cancel the cross-coupling -w*Lq*Iq.  The firmware
    applies vd_dec = -DebugW2 * w * Iq, so from the raw IQ24 words the
    per-unit gain is DebugW2 = -ud_raw * 2^24 / (w_raw * Iq_raw).  The
    returned value is the per-unit gain to write into DebugW2.
    """
    speed_raw = _raw_series(samples, speed_name)
    ud_raw = _raw_series(samples, ud_name)
    iq_raw = _raw_series(samples, iq_name)
    peak_speed = max(abs(_signed(raw)) for raw in speed_raw)
    speed_threshold = peak_speed * high_speed_fraction
    current_threshold = _IQ24 // 10
    numerator = 0.0
    denominator = 0.0
    count = 0
    for ws, wu, wi in zip(speed_raw, ud_raw, iq_raw):
        ws_s = _signed(ws)
        wu_s = _signed(wu)
        wi_s = _signed(wi)
        if abs(ws_s) < speed_threshold or abs(wi_s) < current_threshold:
            continue
        w_pu = ws_s / _IQ24
        iq_pu = wi_s / _IQ24
        ud_pu = wu_s / _IQ24
        coupling_pu = w_pu * iq_pu
        numerator += ud_pu * coupling_pu
        denominator += coupling_pu * coupling_pu
        count += 1
    if count == 0 or denominator <= 0:
        raise ValueError("No valid high-speed coupling points for calibration.")
    # ud_pu = -K * w_pu * Iq_pu, so K = -sum(ud*coupling)/sum(coupling^2).
    gain = -numerator / denominator
    if not math.isfinite(gain) or not 0.0 < gain <= max_gain:
        raise ValueError(
            f"Calibrated decoupling gain out of range: {gain:g}."
        )
    return float(gain)


def analyze_decoupling(
    off: dict[str, Any],
    on: dict[str, Any],
    *,
    target_speed_rpm: float,
    max_ud_ratio: float = 0.6,
    max_id_a: float = 1.0,
    min_coupling_v: float = 0.5,
) -> dict[str, Any]:
    reached = 0.8 * abs(target_speed_rpm)
    mean_abs_ud_ratio = (
        None
        if off["mean_abs_ud_v"] <= 0
        else on["mean_abs_ud_v"] / off["mean_abs_ud_v"]
    )
    coupling_ratio = None
    if (
        off["coupling_coeff"] is not None
        and on["coupling_coeff"] is not None
        and abs(off["coupling_coeff"]) > 1e-6
    ):
        coupling_ratio = abs(on["coupling_coeff"]) / abs(off["coupling_coeff"])
    metrics: dict[str, Any] = {
        "off": off,
        "on": on,
        "mean_abs_ud_ratio": mean_abs_ud_ratio,
        "coupling_coeff_ratio": coupling_ratio,
    }
    criteria = {
        "off_reached_high_speed": off["peak_abs_speed_rpm"] >= reached,
        "on_reached_high_speed": on["peak_abs_speed_rpm"] >= reached,
        "coupling_present": off["peak_abs_ud_v"] >= min_coupling_v,
        "decoupling_active": (
            on["peak_abs_dec_v"] is not None and on["peak_abs_dec_v"] > 1e-4
        ),
        "id_stays_zero": (
            off["peak_abs_id_a"] is not None
            and off["peak_abs_id_a"] <= max_id_a
        ),
        "d_regulator_relieved": (
            coupling_ratio is not None and coupling_ratio <= max_ud_ratio
        ),
    }
    metrics["limits"] = {
        "target_speed_rpm": target_speed_rpm,
        "max_ud_ratio": max_ud_ratio,
        "max_id_a": max_id_a,
        "min_coupling_v": min_coupling_v,
    }
    metrics["criteria"] = criteria
    metrics["passed"] = all(criteria.values())
    return metrics
