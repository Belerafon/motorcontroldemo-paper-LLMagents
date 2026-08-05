from __future__ import annotations

import math
import unittest
from dataclasses import dataclass

from tests.e2e.metrics import (
    analyze_current_mode4,
    analyze_decoupling_correctness,
    analyze_decoupling_stability,
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


class DecouplingMetricsTests(unittest.TestCase):
    def test_correct_decoupling_voltage_passes(self) -> None:
        # we=1.0 pu, Iq=0.2 pu, Id~0, K=0.26 -> dVd=-0.052 pu exactly.
        operating = {
            "we_pu": 1.0, "iq_pu": 0.2, "id_pu": 0.001,
            "dvd_pu": -0.0520, "dvq_pu": 0.00026,
        }
        metrics = analyze_decoupling_correctness(operating, k_pu=0.26)

        self.assertTrue(metrics["passed"])
        self.assertAlmostEqual(metrics["dvd_predicted_pu"], -0.052, places=6)
        self.assertLess(metrics["dvd_abs_error_pu"], 0.001)

    def test_wrong_decoupling_voltage_fails(self) -> None:
        operating = {
            "we_pu": 1.0, "iq_pu": 0.2, "id_pu": 0.0,
            # Wrong sign: should be -0.052.
            "dvd_pu": 0.052, "dvq_pu": 0.0,
        }
        metrics = analyze_decoupling_correctness(operating, k_pu=0.26)

        self.assertFalse(metrics["passed"])
        self.assertFalse(metrics["criteria"]["d_axis_voltage_matches"])

    def test_underexcited_operating_point_fails(self) -> None:
        operating = {
            "we_pu": 0.05, "iq_pu": 0.001, "id_pu": 0.0,
            "dvd_pu": 0.0, "dvq_pu": 0.0,
        }
        metrics = analyze_decoupling_correctness(operating, k_pu=0.26)

        self.assertFalse(metrics["passed"])
        self.assertFalse(metrics["criteria"]["operating_point_excited"])

    @staticmethod
    def _stability_samples(*, id_peak_a: float) -> list[SyntheticSample]:
        result = []
        for number in range(257):
            time_s = number * 0.012
            result.append(
                SyntheticSample(
                    number,
                    time_s,
                    {
                        "speed_rpm": 500.0 * min(1.0, time_s / 0.5),
                        "iq_a": 0.5,
                        "id_a": id_peak_a * (1.0 if time_s > 0.1 else 0.0),
                        "speed_ref_rpm": 500.0,
                    },
                )
            )
        return result

    def test_stable_loops_pass(self) -> None:
        metrics = analyze_decoupling_stability(
            self._stability_samples(id_peak_a=0.05),
            target_speed_rpm=500.0,
        )

        self.assertTrue(metrics["passed"])
        self.assertTrue(metrics["criteria"]["d_axis_loop_intact"])

    def test_large_d_axis_current_fails(self) -> None:
        metrics = analyze_decoupling_stability(
            self._stability_samples(id_peak_a=1.0),
            target_speed_rpm=500.0,
        )

        self.assertFalse(metrics["passed"])
        self.assertFalse(metrics["criteria"]["d_axis_loop_intact"])


if __name__ == "__main__":
    unittest.main()
