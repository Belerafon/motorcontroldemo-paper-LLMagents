from __future__ import annotations

import math
import unittest
from dataclasses import dataclass

from tests.e2e.metrics import (
    analyze_current_mode4,
    analyze_pwm_zero_seq,
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


class PwmZeroSeqMetricsTests(unittest.TestCase):
    KPWM = 2500.0

    @staticmethod
    def centered_samples(
        *,
        max_speed_rpm: float = 500.0,
        max_envelope_ticks: float = 0.8 * 2500.0,
    ) -> list[SyntheticSample]:
        k = 2500.0
        result = []
        for number in range(300):
            time_s = number * 0.01
            speed = max_speed_rpm * (1.0 - math.exp(-time_s / 0.30))
            half = max_envelope_ticks / 2.0 * (1.0 - math.exp(-time_s / 0.5))
            gamma_a = k / 2.0 + half
            gamma_b = k / 2.0 - half
            gamma_c = k / 2.0 - half / 3.0
            result.append(
                SyntheticSample(
                    number,
                    time_s,
                    {
                        "gamma_a": gamma_a,
                        "gamma_b": gamma_b,
                        "gamma_c": gamma_c,
                        "speed_rpm": speed,
                    },
                )
            )
        return result

    @staticmethod
    def constant_samples(gamma: tuple[float, float, float]) -> list[SyntheticSample]:
        return [
            SyntheticSample(
                number,
                number * 0.01,
                {
                    "gamma_a": gamma[0],
                    "gamma_b": gamma[1],
                    "gamma_c": gamma[2],
                    "speed_rpm": 500.0,
                },
            )
            for number in range(200)
        ]

    def test_centered_modulation_passes(self) -> None:
        metrics = analyze_pwm_zero_seq(
            self.centered_samples(), k_pwm=self.KPWM
        )

        self.assertTrue(metrics["passed"])
        self.assertLess(metrics["centering_error_max_percent"], 1e-6)
        self.assertEqual(metrics["bus_tie_violations"], 0)
        self.assertTrue(metrics["criteria"]["speed_reached_target"])
        self.assertTrue(metrics["criteria"]["envelope_modulated"])

    def test_uncentered_signature_fails_centering(self) -> None:
        # Sinusoidal PWM keeps min + max < k_pwm while modulating (e.g. 0.9 k).
        metrics = analyze_pwm_zero_seq(
            self.constant_samples((0.3 * self.KPWM, 0.3 * self.KPWM,
                                   0.6 * self.KPWM)),
            k_pwm=self.KPWM,
        )

        self.assertFalse(metrics["passed"])
        self.assertFalse(metrics["criteria"]["centering_ok"])
        self.assertLess(metrics["centering_error_max_percent"] - 10.0, 1e-6)

    def test_bus_tie_fails_no_bus_tie(self) -> None:
        # Overmodulated (phase pinned to a rail) still centers, but violates
        # the "no bus tie" requirement.
        metrics = analyze_pwm_zero_seq(
            self.constant_samples((0.0, self.KPWM, 0.0)),
            k_pwm=self.KPWM,
        )

        self.assertFalse(metrics["passed"])
        self.assertTrue(metrics["criteria"]["centering_ok"])
        self.assertFalse(metrics["criteria"]["no_bus_tie"])
        self.assertGreater(metrics["bus_tie_violations"], 0)

    def test_slow_drive_fails_speed_criterion(self) -> None:
        metrics = analyze_pwm_zero_seq(
            self.centered_samples(max_speed_rpm=300.0), k_pwm=self.KPWM
        )

        self.assertFalse(metrics["passed"])
        self.assertFalse(metrics["criteria"]["speed_reached_target"])


if __name__ == "__main__":
    unittest.main()
