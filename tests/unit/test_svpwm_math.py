from __future__ import annotations

import math
import unittest

from tests.e2e.svpwm import (
    ONE_IQ24,
    analyze_pwm_svpwm,
    compute_sin_pwm_gamma,
    compute_sv_gamma,
    from_iq24,
    iq24mpy,
    to_iq24,
)


class _SyntheticSample:
    """Лёгкая имитация ``motor_debug_core.datalogger.Sample`` для unit-тестов."""

    def __init__(self, number: int, time_s: float, raw: dict[str, int]):
        self.number = number
        self.time_s = time_s
        self.raw = dict(raw)
        self.values = {key: float(value) for key, value in raw.items()}



class SvpwmMathMirrorTests(unittest.TestCase):
    """Побитовые свойства зеркального расчёта SVPWM (без контроллера)."""

    def test_bit_exact_match_reference_values(self) -> None:
        # ua=0.5, ub=uc=-0.25 (в масштабе базового вектора), k_pwm=10000.
        ua = to_iq24(0.5)
        ub = to_iq24(0.0)
        ga, gb, gc = compute_sv_gamma(ua, ub, 10000)
        # ua=0.5; ub=uc=-0.25; uz=-0.125; da=db-shifted, ожидаем:
        # da = 0.5 + (2/3)*(0.5-0.125) = 0.75 -> gamma_a = 10000*0.25 = 2500
        # db = 0.5 + (2/3)*(-0.25-0.125) = 0.25 -> gamma_b = 10000*0.75 = 7500
        self.assertEqual(ga, 2500)
        self.assertEqual(gb, 7500)
        self.assertEqual(gc, 7500)

    def test_zero_input_is_neutral(self) -> None:
        ga, gb, gc = compute_sv_gamma(0, 0, 10000)
        # Нулевой вектор -> все три фазы 50 % скважности -> Gamma = k_pwm/2.
        # В IQ24: da=db=dc=0.5 -> 1-0.5=0.5 -> gamma=k_pwm*0.5=5000.
        self.assertEqual(ga, 5000)
        self.assertEqual(gb, 5000)
        self.assertEqual(gc, 5000)

    def test_no_rail_touch_below_inscribed_circle(self) -> None:
        # Внутри вписанной окружности (|UalphaNorm| < 0.866) SVPWM не должна
        # касаться шин: все Gamma строго в (margin, k_pwm-margin) при разумном
        # margin. На самой вписанной окружности касание возможно (это граница
        # линейной области), поэтому берём радиус 0.5 - с запасом внутри.
        radius = 0.5
        worst_low = 10_000_000
        worst_high = 0
        for step in range(0, 360):
            angle = math.radians(step)
            ua = to_iq24(radius * math.cos(angle))
            ub = to_iq24(radius * math.sin(angle))
            for gamma in compute_sv_gamma(ua, ub, 10000):
                worst_low = min(worst_low, gamma)
                worst_high = max(worst_high, gamma)
        # При r=0.5 extreme Gamma ~(1-0.789)*10000=2111 и 0.789*10000=7889.
        self.assertGreater(worst_low, 1500)
        self.assertLess(worst_high, 8500)

    def test_inscribed_circle_stays_in_range(self) -> None:
        # На вписанной окружности SVPWM может касаться шин (граница линейной
        # области), но никогда не выходит за пределы [0, k_pwm+1].
        radius = 0.866
        for step in range(0, 360):
            angle = math.radians(step)
            ua = to_iq24(radius * math.cos(angle))
            ub = to_iq24(radius * math.sin(angle))
            for gamma in compute_sv_gamma(ua, ub, 10000):
                self.assertGreaterEqual(gamma, 0)
                self.assertLessEqual(gamma, 10000 + 1)

    def test_svpwm_vs_sin_pwm_voltage_utilization(self) -> None:
        # Главное потребительское свойство: при |UalphaRef| > 0.866 синусоидальная
        # ШИМ насыщается (одна из Gamma уходит в 0 или k_pwm+1), а SVPWM в
        # линейной области работает без насыщения вплоть до |UalphaRef| = 1.0.
        u_lim = to_iq24(0.95)
        k_pwm = 10000
        for step in range(0, 360):
            angle = math.radians(step)
            ua_ref = to_iq24(0.95 * math.cos(angle))
            ub_ref = to_iq24(0.95 * math.sin(angle))
            sa, sb, sc = compute_sin_pwm_gamma(ua_ref, ub_ref, u_lim, k_pwm)
            sin_clamped = any(g == 0 or g == k_pwm + 1 for g in (sa, sb, sc))

            ua_norm = to_iq24(0.95 * 0.866 * math.cos(angle))
            ub_norm = to_iq24(0.95 * 0.866 * math.sin(angle))
            va, vb, vc = compute_sv_gamma(ua_norm, ub_norm, k_pwm)
            sv_clamped = any(g == 0 or g == k_pwm + 1 for g in (va, vb, vc))

            # Хотя бы на части orientations синус должен попасть в насыщение,
            # а SVPWM в линейной области (0.95 < 1.0) насыщаться не должна.
            if abs(math.cos(angle)) > 0.5 or abs(math.sin(angle)) > 0.5:
                self.assertFalse(
                    sv_clamped,
                    f"SVPWM не должен насыщаться при |U|<1.0 (angle={step})",
                )
            # Синусоидальная на каком-то сэмпле точно входит в насыщение.
            if step == 0:
                self.assertTrue(
                    sin_clamped,
                    "Синусоидальная ШИМ должна насыщаться при |UalphaRef|>0.866",
                )

    def test_svpwm_linear_limit_is_one(self) -> None:
        # Линейный предел SVPWM по |UalphaRef| = 1.0. На самóм пределе
        # (|UalphaNorm| = 0.866 на альфе) одна Gamma доходит точно до 0 или
        # k_pwm+1, но не вылетает за пределы.
        ua = to_iq24(0.866)
        ub = to_iq24(0.0)
        ga, gb, gc = compute_sv_gamma(ua, ub, 10000)
        for gamma in (ga, gb, gc):
            self.assertGreaterEqual(gamma, 0)
            self.assertLessEqual(gamma, 10000 + 1)

    def test_svpwm_min_max_injection_centers_duties(self) -> None:
        # min-max впрыск Uz = -(max+min)/2 ставит активный вектор в центр
        # периода: в скважностях это свойство max(D) + min(D) = 1.
        ua = to_iq24(0.4)
        ub = to_iq24(0.3)
        ga, gb, gc = compute_sv_gamma(ua, ub, 10000)
        duties = [(10000 - g) / 10000.0 for g in (ga, gb, gc)]
        self.assertAlmostEqual(max(duties) + min(duties), 1.0, delta=2.0 / 10000)


class AnalyzePwmSvpwmTests(unittest.TestCase):
    def _samples(self) -> list[_SyntheticSample]:
        samples: list[_SyntheticSample] = []
        k_pwm = 10000
        for step in range(256):
            time_s = step * 0.0004
            angle = step * 0.05
            ua_norm = to_iq24(0.6 * math.cos(angle))
            ub_norm = to_iq24(0.6 * math.sin(angle))
            ga, gb, _ = compute_sv_gamma(ua_norm, ub_norm, k_pwm)
            samples.append(
                _SyntheticSample(
                    step,
                    time_s,
                    {
                        "ualpha_norm": ua_norm,
                        "ubeta_norm": ub_norm,
                        "gamma_a": ga,
                        "gamma_b": gb,
                    },
                )
            )
        return samples

    def test_clean_capture_passes(self) -> None:
        metrics = analyze_pwm_svpwm(self._samples(), k_pwm=10000)
        self.assertTrue(metrics["criteria"]["svpwm_math_matches"])
        self.assertTrue(metrics["criteria"]["no_bus_clamping"])
        self.assertTrue(metrics["criteria"]["exercise_above_noise"])
        self.assertEqual(metrics["svpwm_math_max_error_ticks"], 0)
        self.assertTrue(metrics["passed"])

    def test_broken_math_fails(self) -> None:
        samples = self._samples()
        # Псевдо-сдвигаем все gamma_a на +20 тиков - имитация ошибки прошивки.
        for sample in samples:
            sample.raw["gamma_a"] = int(sample.raw["gamma_a"]) + 20
            sample.values["gamma_a"] = float(sample.raw["gamma_a"])
        metrics = analyze_pwm_svpwm(samples, k_pwm=10000)
        self.assertFalse(metrics["passed"])
        self.assertFalse(metrics["criteria"]["svpwm_math_matches"])

    def test_clamped_samples_fail_no_bus_clamping(self) -> None:
        samples = self._samples()
        # Загоним gamma_b в "привязку к шине" на половине сэмплов.
        for index, sample in enumerate(samples):
            if index % 2 == 0:
                sample.raw["gamma_b"] = 0
                sample.values["gamma_b"] = 0.0
        metrics = analyze_pwm_svpwm(samples, k_pwm=10000)
        self.assertFalse(metrics["criteria"]["no_bus_clamping"])


if __name__ == "__main__":
    unittest.main()
