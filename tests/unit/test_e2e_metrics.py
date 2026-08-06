from __future__ import annotations

import math
import unittest
from dataclasses import dataclass

from tests.e2e.metrics import (
    analyze_current_mode4,
    analyze_dynamic_limit,
    analyze_vector_500,
    expected_voltage_limit,
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


class DynamicLimitMetricsTests(unittest.TestCase):
    def test_expected_limit_is_min_of_udc_and_ulim(self) -> None:
        # DC-link voltage is below the modulation capability: limit = Udc.
        self.assertAlmostEqual(
            expected_voltage_limit(udc_meas=0.6, u_lim=0.866), 0.6, places=4
        )

    def test_expected_limit_capped_by_pwm_ulim(self) -> None:
        # DC-link voltage exceeds the PWM capability: limit = U_lim.
        self.assertAlmostEqual(
            expected_voltage_limit(udc_meas=1.2, u_lim=0.866), 0.866, places=4
        )

    def test_expected_limit_never_below_floor(self) -> None:
        self.assertEqual(expected_voltage_limit(udc_meas=0.01, u_lim=0.866), 0.05)

    def test_tracking_pass_and_fail(self) -> None:
        good = {
            "low": {
                "udc_meas": 0.6,
                "u_lim": 0.866,
                "out_max": 0.6,
                "out_min": -0.6,
            },
            "high": {
                "udc_meas": 0.95,
                "u_lim": 1.2,
                "out_max": 0.95,
                "out_min": -0.95,
            },
            "pwm-limited": {
                "udc_meas": 0.95,
                "u_lim": 0.5,
                "out_max": 0.5,
                "out_min": -0.5,
            },
        }
        metrics = analyze_dynamic_limit(good)
        self.assertTrue(metrics["passed"])

        bad = dict(good)
        bad["high"] = {
            "udc_meas": 0.95,
            "u_lim": 1.2,
            "out_max": 0.9999,
            "out_min": -0.9999,
        }
        failed = analyze_dynamic_limit(bad)
        self.assertFalse(failed["passed"])
        self.assertFalse(failed["criteria"]["high"])


if __name__ == "__main__":
    unittest.main()
