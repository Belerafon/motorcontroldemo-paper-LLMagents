from __future__ import annotations

import math
import unittest
from dataclasses import dataclass

from tests.e2e.metrics import (
    analyze_current_mode4,
    analyze_svpwm_gamma,
    analyze_vector_500,
)


@dataclass(frozen=True)
class SyntheticSample:
    number: int
    time_s: float
    values: dict[str, float]


class CurrentMode4MetricsTests(unittest.TestCase):
    @staticmethod
    def samples(
        *, iq_bias: float = 0.0, rotor_speed_rpm: float = 0.0
    ) -> list[SyntheticSample]:
        result = []
        for number in range(351):
            time_s = number * 0.001
            result.append(
                SyntheticSample(
                    number,
                    time_s,
                    {
                        "id_ref_a": 1.0,
                        "id_a": 1.0 - math.exp(-time_s / 0.008),
                        "iq_ref_a": 0.0,
                        "iq_a": iq_bias + 0.01 * math.sin(2 * math.pi * 50 * time_s),
                        "rotor_speed_rpm": rotor_speed_rpm,
                    },
                )
            )
        return result

    def test_well_tuned_current_loops_pass(self) -> None:
        metrics = analyze_current_mode4(self.samples())

        self.assertTrue(metrics["passed"])
        self.assertLess(metrics["id_settling_time_s"], 0.05)
        self.assertLess(metrics["iq_steady_rms_a"], 0.10)

    def test_q_axis_bias_fails_acceptance(self) -> None:
        metrics = analyze_current_mode4(self.samples(iq_bias=0.2))

        self.assertFalse(metrics["passed"])
        self.assertFalse(metrics["criteria"]["iq_bias"])
        self.assertFalse(metrics["criteria"]["iq_rms"])

    def test_moving_rotor_invalidates_current_loop_test(self) -> None:
        metrics = analyze_current_mode4(self.samples(rotor_speed_rpm=8.0))

        self.assertFalse(metrics["passed"])
        self.assertFalse(metrics["criteria"]["rotor_stationary"])


class Vector500MetricsTests(unittest.TestCase):
    @staticmethod
    def samples(*, overshoot: bool = False) -> list[SyntheticSample]:
        result = []
        for number in range(3001):
            time_s = number * 0.001
            reference = 500.0 * (1.0 - math.exp(-time_s / 0.10))
            speed = 500.0 * (1.0 - math.exp(-time_s / 0.30))
            if overshoot:
                speed += 140.0 * math.exp(-((time_s - 0.7) / 0.08) ** 2)
            iq = 0.05 + 0.75 * math.exp(-time_s / 0.4)
            result.append(
                SyntheticSample(
                    number,
                    time_s,
                    {
                        "speed_rpm": speed,
                        "speed_ref_rpm": reference,
                        "iq_a": iq,
                        "iq_ref_a": min(0.8, 0.1 + 0.7 * math.exp(-time_s / 0.5)),
                    },
                )
            )
        return result

    def test_nominal_vector_response_passes(self) -> None:
        metrics = analyze_vector_500(self.samples(), current_limit_a=1.0)

        self.assertTrue(metrics["passed"])
        self.assertLess(metrics["time_to_95_percent_s"], 2.0)
        self.assertLess(metrics["steady_speed_error_percent"], 2.0)

    def test_excessive_overshoot_fails_acceptance(self) -> None:
        metrics = analyze_vector_500(
            self.samples(overshoot=True),
            current_limit_a=1.0,
        )

        self.assertFalse(metrics["passed"])
        self.assertFalse(metrics["criteria"]["overshoot"])


class SVPWMAnalyzerTests(unittest.TestCase):
    K_PWM = 1000
    A = 0.25  # phase-voltage amplitude, fraction of half DC-bus

    @staticmethod
    def _phase_duties(theta: float, *, inject: bool) -> tuple[float, float, float]:
        import math

        ua = SVPWMAnalyzerTests.A * math.sin(theta)
        ub = SVPWMAnalyzerTests.A * math.sin(theta - 2 * math.pi / 3)
        uc = SVPWMAnalyzerTests.A * math.sin(theta + 2 * math.pi / 3)
        if inject:
            vmin = min(ua, ub, uc)
            vmax = max(ua, ub, uc)
            offset = -0.5 * (vmin + vmax)
            ua, ub, uc = ua + offset, ub + offset, uc + offset
        return (0.5 - ua, 0.5 - ub, 0.5 - uc)

    @classmethod
    def samples(
        cls,
        *,
        inject: bool,
        clamped_fraction: float = 0.0,
        speed_rpm: float = 500.0,
    ) -> list[SyntheticSample]:
        import math

        result = []
        count = 200
        clamped_count = round(clamped_fraction * count)
        for number in range(count):
            theta = 2 * math.pi * 8.33 * number * 0.004
            da, db, dc = cls._phase_duties(theta, inject=inject)
            ga = round(da * cls.K_PWM)
            gb = round(db * cls.K_PWM)
            gc = round(dc * cls.K_PWM)
            if number < clamped_count:
                gc = 0
            result.append(
                SyntheticSample(
                    number,
                    number * 0.004,
                    {
                        "gamma_a": float(ga),
                        "gamma_b": float(gb),
                        "gamma_c": float(gc),
                        "speed_rpm": speed_rpm,
                    },
                )
            )
        return result

    def test_continuous_svpwm_passes(self) -> None:
        metrics = analyze_svpwm_gamma(
            self.samples(inject=True),
            k_pwm=self.K_PWM,
            target_speed_rpm=500.0,
        )

        self.assertTrue(metrics["passed"])
        self.assertTrue(metrics["criteria"]["no_bus_clamping"])
        self.assertTrue(metrics["criteria"]["zero_sequence_injection_present"])
        self.assertGreater(metrics["peak_abs_common_mode_fraction"], 0.02)
        self.assertEqual(metrics["clamped_samples_fraction"], 0.0)

    def test_momentary_rail_touch_still_passes(self) -> None:
        # A single phase touching the rail for one sample (SVPWM linear-limit
        # touch during a voltage transient) must not count as clamping.
        metrics = analyze_svpwm_gamma(
            self.samples(inject=True, clamped_fraction=1.0 / 200.0),
            k_pwm=self.K_PWM,
            target_speed_rpm=500.0,
        )

        self.assertTrue(metrics["criteria"]["no_bus_clamping"])
        self.assertLess(metrics["clamped_samples_fraction"], 0.10)

    def test_sinusoidal_without_injection_is_detected(self) -> None:
        metrics = analyze_svpwm_gamma(
            self.samples(inject=False),
            k_pwm=self.K_PWM,
            target_speed_rpm=500.0,
        )

        self.assertTrue(metrics["criteria"]["no_bus_clamping"])
        self.assertFalse(metrics["criteria"]["zero_sequence_injection_present"])

    def test_sustained_bus_clamping_is_detected(self) -> None:
        # A discontinuous modulator holds a phase on the rail for a large
        # fraction of the cycle; this must be rejected.
        metrics = analyze_svpwm_gamma(
            self.samples(inject=True, clamped_fraction=0.6),
            k_pwm=self.K_PWM,
            target_speed_rpm=500.0,
        )

        self.assertFalse(metrics["criteria"]["no_bus_clamping"])
        self.assertGreater(metrics["clamped_samples_fraction"], 0.10)

    def test_speed_not_reached_fails(self) -> None:
        metrics = analyze_svpwm_gamma(
            self.samples(inject=True, speed_rpm=200.0),
            k_pwm=self.K_PWM,
            target_speed_rpm=500.0,
            max_speed_deviation_rpm=50.0,
        )

        self.assertFalse(metrics["criteria"]["speed_reached"])


if __name__ == "__main__":
    unittest.main()
