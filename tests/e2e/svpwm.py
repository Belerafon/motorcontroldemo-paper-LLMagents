"""Аналитика симметричной (непрерывной) векторной ШИМ SVPWM.

Модуль — точный Python-зеркальный аналог расчёта ``PWM_Module_SV_Update``
из ``Vsrc/V_PWM_Module.c``. Все вычисления идут в целочисленном формате
IQ24 с теми же константами и тем же порядком операций, что и в прошивке,
поэтому ожидаемые значения Gamma совпадают с прошивкой побитово (с точностью
до усечения `_IQ24mpy`).

Соглашения прошивки:
- ``UalphaNorm``/``UbetaNorm`` - IQ24, масштаб "длина базового вектора"
  (после ``PWM_Module_NormInput``: ограничение по U_lim, компенсация Ud и
  умножение на 0.866).
- ``GammaA``/``GammaB``/``GammaC`` - целые отсчёты таймера (IQ0), диапазон
  ``0..k_pwm+1``. По действиям AQCTL: ``duty = 1 - CMPA/k_pwm``.
- ``k_pwm`` - период таймера в отсчётах (TBPRD).

Свойство SVPWM: в линейной области (``|UalphaNorm| <= 0.866``,
т.е. ``|UalphaRef| <= 1.0``) ни одна фаза не привязана к шине. Линейная
область по ``|UalphaRef|`` простирается до 1.0 - это на 1/0.866 ~ 1.155
раза (на 15.5 %) больше, чем у синусоидальной ШИМ, для которой линейный
предел - 0.866.
"""

from __future__ import annotations

import math
from collections.abc import Sequence
from typing import Any


# --- Константы IQ24, идентичные прошивке ------------------------------

IQ24 = 24
ONE_IQ24 = 1 << IQ24                       # _IQ(1.0)
HALF_IQ24 = 1 << (IQ24 - 1)                # _IQ(0.5)
NEG_HALF_IQ24 = -(1 << (IQ24 - 1))         # _IQ(-0.5)
# _IQ(2.0/3.0) — (long)(2.0/3.0 * 2^24), усечение, как в C
TWO_THIRDS_IQ24 = int((2.0 / 3.0) * float(ONE_IQ24))
# _IQ(0.8660254) — та же константа, что в прошивке
SQRT3_HALF_IQ24 = int(0.8660254 * float(ONE_IQ24))


def iq24mpy(a: int, b: int) -> int:
    """Целочисленное умножение IQ24 с тем же усечением, что в прошивке.

    ``(a * b) >> 24`` для знаковых чисел соответствует арифметическому
    сдвигу C (ARM GCC). Python ``>>`` для отрицательных чисел даёт то же
    усечение вниз, поэтому результат совпадает с `(int32)(((int64)a*b)>>24)`
    для значений, укладывающихся в int32.
    """
    return (a * b) >> IQ24


def sign32(raw: int) -> int:
    """Знаковое расширение беззнакового 32-битного слова из транспорту.

    Транспорт OpenOCD возвращает слова как unsigned hex. Прошивка хранит
    IQ24-значения как ``int32``, поэтому отрицательные числа приходят
    как ``0xFFxxxxxx`` и их нужно привести к знаковому виду.
    """
    return raw - 0x1_0000_0000 if raw >= 0x8000_0000 else raw


def to_iq24(value: float) -> int:
    """`_IQ(value)` как `(long)(value * 2^24)` (с усечением)."""
    return int(value * float(ONE_IQ24))


def from_iq24(raw: int) -> float:
    """Декодирование IQ24 в float (с знаковым расширением сырого слова)."""
    return sign32(raw) / float(ONE_IQ24)


def compute_sv_gamma(
    ualpha_norm_iq24: int, ubeta_norm_iq24: int, k_pwm: int
) -> tuple[int, int, int]:
    """Точный аналог ``PWM_Module_SV_Update`` для GammaA, GammaB, GammaC.

    На входе - ``UalphaNorm``/``UbetaNorm`` (сырые слова из памяти; могут
    быть беззнаковыми hex от транспорта - знаковое расширение выполняется
    внутри), IQ24, масштаб базового вектора, как оставляет их
    ``PWM_Module_NormInput``; и ``k_pwm``. На выходе - кортеж целых Gamma
    (отсчёты таймера) с теми же насыщениями ``0..k_pwm+1``, что в прошивке.
    """
    ua = sign32(ualpha_norm_iq24)
    ubeta = sign32(ubeta_norm_iq24)
    ub = iq24mpy(NEG_HALF_IQ24, ua) + iq24mpy(SQRT3_HALF_IQ24, ubeta)
    uc = iq24mpy(NEG_HALF_IQ24, ua) - iq24mpy(SQRT3_HALF_IQ24, ubeta)

    u_max = max(ua, ub, uc)
    u_min = min(ua, ub, uc)
    uz = iq24mpy(NEG_HALF_IQ24, u_max + u_min)

    da = HALF_IQ24 + iq24mpy(TWO_THIRDS_IQ24, ua + uz)
    db = HALF_IQ24 + iq24mpy(TWO_THIRDS_IQ24, ub + uz)
    dc = HALF_IQ24 + iq24mpy(TWO_THIRDS_IQ24, uc + uz)

    gamma_a = iq24mpy(k_pwm, ONE_IQ24 - da)
    gamma_b = iq24mpy(k_pwm, ONE_IQ24 - db)
    gamma_c = iq24mpy(k_pwm, ONE_IQ24 - dc)

    def saturate(gamma: int) -> int:
        if gamma < 0:
            return 0
        if gamma > k_pwm:
            return k_pwm + 1
        return gamma

    return saturate(gamma_a), saturate(gamma_b), saturate(gamma_c)


def compute_sin_pwm_gamma(
    ualpha_ref_iq24: int, ubeta_ref_iq24: int, u_lim_iq24: int, k_pwm: int
) -> tuple[int, int, int]:
    """Зеркальный аналог ``PWM_Module_Sin_Update`` (для A/B-сравнения).

    На входе - ``UalphaRef``/``UbetaRef`` (IQ24) и ``U_lim``. Возвращает
    GammaA/GammaB/GammaC (отсчёты таймера). Реализует то же ограничение по
    U_lim и то же масштабирование ``1/(0.866*2)``, что и прошивка.
    """
    alpha = ualpha_ref_iq24
    beta = ubeta_ref_iq24

    # _IQmag(alpha, beta) - IQ24: sqrt((alpha/2^24)^2 + (beta/2^24)^2) * 2^24
    a_f = from_iq24(alpha)
    b_f = from_iq24(beta)
    mag = to_iq24(math.hypot(a_f, b_f))

    u_lim = u_lim_iq24
    if mag >= u_lim:
        if mag == 0:
            alpha = 0
            beta = 0
        else:
            knorm = to_iq24(from_iq24(u_lim) / from_iq24(mag))
            alpha = iq24mpy(knorm, alpha)
            beta = iq24mpy(knorm, beta)

    # p->UalphaNorm = UalphaNorm * _IQ(1/0.866/2)
    scale = to_iq24(1.0 / 0.866 / 2.0)
    alpha_n = iq24mpy(alpha, scale)
    beta_n = iq24mpy(beta, scale)

    half = HALF_IQ24
    sqrt3_half = SQRT3_HALF_IQ24

    phase_a = half - alpha_n
    phase_b = half - (iq24mpy(beta_n, sqrt3_half) - (alpha_n >> 1))
    phase_c = half - (-(iq24mpy(beta_n, sqrt3_half)) - (alpha_n >> 1))

    gamma_a = iq24mpy(k_pwm, phase_a)
    gamma_b = iq24mpy(k_pwm, phase_b)
    gamma_c = iq24mpy(k_pwm, phase_c)

    def saturate(gamma: int) -> int:
        if gamma < 0:
            return 0
        if gamma > k_pwm:
            return k_pwm + 1
        return gamma

    return saturate(gamma_a), saturate(gamma_b), saturate(gamma_c)


def analyze_pwm_svpwm(
    samples: Sequence[Any],
    *,
    k_pwm: int,
    math_tolerance_ticks: int = 2,
    min_required_amplitude_fraction: float = 0.05,
    rail_margin_fraction: float = 0.02,
) -> dict[str, Any]:
    """Сверка захваченных Gamma с SVPWM-математикой и проверка непрерывности.

    ``samples`` - записи регистратора; в каждой должны быть сырые (int)
    значения ``ualpha_norm``, ``ubeta_norm``, ``gamma_a``, ``gamma_b``.
    Возвращает словарь с метриками и критериями приёмки:
    - соответствие математики SVPWM (для ``GammaA``/``GammaB``);
    - непрерывность: ни одна фаза не "залипла" на шине (нет сэмплов, где
      ``Gamma`` попадает в узкую ``rail_margin_fraction`` полосу у 0 или у
      ``k_pwm+1`` при ненулевом векторе);
    - достаточный размах задания (чтобы тест вообще exerciser SVPWM).
    """
    if k_pwm <= 0:
        raise ValueError("k_pwm must be positive.")
    if not samples:
        raise ValueError("At least one sample is required.")

    times = [float(sample.time_s) for sample in samples]
    window_s = times[-1] - times[0] if len(times) >= 2 else 0.0

    expected_a: list[int] = []
    expected_b: list[int] = []
    expected_c: list[int] = []
    actual_a: list[int] = []
    actual_b: list[int] = []
    actual_c: list[int] = []
    magnitudes_bv: list[float] = []

    for sample in samples:
        values = sample.values
        ua = int(sample.raw["ualpha_norm"])
        ub = int(sample.raw["ubeta_norm"])
        ga_exp, gb_exp, gc_exp = compute_sv_gamma(ua, ub, k_pwm)
        expected_a.append(ga_exp)
        expected_b.append(gb_exp)
        expected_c.append(gc_exp)
        actual_a.append(int(sample.raw["gamma_a"]))
        actual_b.append(int(sample.raw["gamma_b"]))
        if "gamma_c" in sample.raw:
            actual_c.append(int(sample.raw["gamma_c"]))
        magnitudes_bv.append(math.hypot(from_iq24(ua), from_iq24(ub)))

    errors_a = [abs(a - e) for a, e in zip(actual_a, expected_a)]
    errors_b = [abs(a - e) for a, e in zip(actual_b, expected_b)]
    max_error = max(errors_a) if errors_a else 0
    if errors_b:
        max_error = max(max_error, max(errors_b))
    mean_error_a = sum(errors_a) / len(errors_a) if errors_a else 0.0
    mean_error_b = sum(errors_b) / len(errors_b) if errors_b else 0.0
    mean_error = 0.5 * (mean_error_a + mean_error_b)
    within_tol = sum(
        1
        for pair in zip(errors_a + errors_b, [math_tolerance_ticks] * (len(errors_a) + len(errors_b)))
        if pair[0] <= pair[1]
    )
    within_tol_fraction = within_tol / (len(errors_a) + len(errors_b))

    # Размах задания по alpha/beta (в долях базового вектора). На выводе
    # NormInput max |U| в этом масштабе - 0.866 (вписанная окружность).
    peak_magnitude = max(magnitudes_bv) if magnitudes_bv else 0.0

    # Проверка непрерывности: ни одна фаза не "залипает" на шине.
    rail_low = rail_margin_fraction * k_pwm
    rail_high = k_pwm + 1 - rail_margin_fraction * k_pwm
    clamped_samples = 0
    nonzero_samples = 0
    for index in range(len(actual_a)):
        magnitude = magnitudes_bv[index]
        if magnitude < min_required_amplitude_fraction * 0.866:
            continue
        nonzero_samples += 1
        clamped = False
        for gamma in (actual_a[index], actual_b[index]):
            if gamma <= rail_low or gamma >= rail_high:
                clamped = True
                break
        if "gamma_c" in samples[0].raw and index < len(actual_c):
            gc = actual_c[index]
            if gc <= rail_low or gc >= rail_high:
                clamped = True
        if clamped:
            clamped_samples += 1
    clamped_fraction = (
        clamped_samples / nonzero_samples if nonzero_samples else 0.0
    )

    metrics: dict[str, Any] = {
        "samples": len(samples),
        "window_s": window_s,
        "k_pwm": k_pwm,
        "math_tolerance_ticks": math_tolerance_ticks,
        "svpwm_math_max_error_ticks": max_error,
        "svpwm_math_mean_error_ticks": mean_error,
        "svpwm_math_within_tolerance_fraction": within_tol_fraction,
        "peak_umagnitude_base_vector": peak_magnitude,
        "peak_umagnitude_inscribed_circle_fraction": (
            peak_magnitude / 0.866 if 0.866 else 0.0
        ),
        "rail_margin_fraction": rail_margin_fraction,
        "nonzero_samples": nonzero_samples,
        "clamped_samples": clamped_samples,
        "clamped_fraction": clamped_fraction,
        "limits": {
            "math_tolerance_ticks": math_tolerance_ticks,
            "min_required_amplitude_fraction": min_required_amplitude_fraction,
            "rail_margin_fraction": rail_margin_fraction,
        },
    }
    criteria = {
        "enough_samples": len(samples) >= 64,
        "svpwm_math_matches": (
            within_tol_fraction >= 0.99 and max_error <= math_tolerance_ticks * 4
        ),
        "exercise_above_noise": (
            peak_magnitude >= min_required_amplitude_fraction * 0.866
        ),
        "no_bus_clamping": clamped_fraction <= 0.02,
    }
    metrics["criteria"] = criteria
    metrics["passed"] = all(criteria.values())
    return metrics
