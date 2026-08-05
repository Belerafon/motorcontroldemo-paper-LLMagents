from __future__ import annotations

import math
import unittest
from dataclasses import dataclass

from tests.e2e.metrics import (
    analyze_current_mode4,
    analyze_vector_500,
    expected_current_reg_limit,
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


class ExpectedCurrentRegLimitTests(unittest.TestCase):
    def test_limit_equals_u_lim_independent_of_udc_and_udcomp(self) -> None:
        # The DC-bus dependence is handled by the PWM Ud-compensation, so the
        # regulator command limit must be U_lim regardless of Udc/UdComp.
        for udc in (0.3, 0.5, 0.667, 1.0, 1.2):
            for udcomp_ena in (0, 1):
                for udcomp_k in (0.0, 0.5, 1.0, 2.0):
                    with self.subTest(udc=udc, ena=udcomp_ena, k=udcomp_k):
                        self.assertAlmostEqual(
                            expected_current_reg_limit(
                                u_lim_pu=0.866,
                                udc_meas_pu=udc,
                                udcomp_enable=udcomp_ena,
                                udcomp_k=udcomp_k,
                            ),
                            0.866,
                            places=9,
                        )

    def test_negative_u_lim_clamped_to_zero(self) -> None:
        self.assertEqual(
            expected_current_reg_limit(
                u_lim_pu=-0.5,
                udc_meas_pu=1.0,
                udcomp_enable=1,
                udcomp_k=1.0,
            ),
            0.0,
        )

    def test_delivered_voltage_ceiling_is_u_lim_times_udc(self) -> None:
        from tests.e2e.metrics import delivered_voltage_ceiling

        self.assertAlmostEqual(
            delivered_voltage_ceiling(0.866, 0.667), 0.577622, places=6
        )
        self.assertEqual(delivered_voltage_ceiling(-1.0, 0.5), 0.0)


if __name__ == "__main__":
    unittest.main()
