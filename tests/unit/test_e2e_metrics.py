from __future__ import annotations

import math
import unittest
from dataclasses import dataclass

from tests.e2e.metrics import (
    analyze_constant_power,
    analyze_current_mode4,
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


class ConstantPowerMetricsTests(unittest.TestCase):
    CURRENT_BASE_A = 5.0
    SPEED_BASE_RPM = 500.0
    IQ_MAX_PU = 1.0
    POWER_LIMIT_PU = 0.5

    @staticmethod
    def _trajectory(constant_power: bool) -> list[SyntheticSample]:
        samples = []
        speed_pu = 0.05
        for number in range(301):
            time_s = number * 0.01
            speed_pu = min(1.0, 0.05 + (time_s / 3.0) * 0.95)
            if constant_power:
                iq_pu = min(
                    ConstantPowerMetricsTests.IQ_MAX_PU,
                    ConstantPowerMetricsTests.POWER_LIMIT_PU / speed_pu,
                )
            else:
                iq_pu = ConstantPowerMetricsTests.IQ_MAX_PU
            samples.append(
                SyntheticSample(
                    number,
                    time_s,
                    {
                        "speed_rpm": speed_pu
                        * ConstantPowerMetricsTests.SPEED_BASE_RPM,
                        "speed_ref_rpm": ConstantPowerMetricsTests.SPEED_BASE_RPM,
                        "iq_ref_a": iq_pu
                        * ConstantPowerMetricsTests.CURRENT_BASE_A,
                        "iq_a": iq_pu
                        * ConstantPowerMetricsTests.CURRENT_BASE_A,
                    },
                )
            )
        return samples

    def test_constant_power_trajectory_passes(self) -> None:
        metrics = analyze_constant_power(
            self._trajectory(constant_power=True),
            power_limit_pu=self.POWER_LIMIT_PU,
            current_base_a=self.CURRENT_BASE_A,
            speed_base_rpm=self.SPEED_BASE_RPM,
            iq_max_pu=self.IQ_MAX_PU,
        )

        self.assertTrue(metrics["passed"])
        self.assertLess(metrics["power_ref_spread_percent"], 1.0)
        self.assertLess(metrics["power_ref_error_percent"], 1.0)
        self.assertGreaterEqual(metrics["binding_sample_count"], 20)

    def test_legacy_constant_current_trajectory_fails(self) -> None:
        metrics = analyze_constant_power(
            self._trajectory(constant_power=False),
            power_limit_pu=self.POWER_LIMIT_PU,
            current_base_a=self.CURRENT_BASE_A,
            speed_base_rpm=self.SPEED_BASE_RPM,
            iq_max_pu=self.IQ_MAX_PU,
        )

        self.assertFalse(metrics["passed"])
        self.assertFalse(metrics["criteria"]["power_limit_binds"])
        self.assertFalse(metrics["criteria"]["current_reduced_below_max"])


if __name__ == "__main__":
    unittest.main()
