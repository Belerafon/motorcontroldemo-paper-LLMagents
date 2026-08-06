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
    LIMIT_PU = 0.06
    IQ_LIMIT_PU = 0.2

    @staticmethod
    def samples(*, mode: str = "constant-power") -> list[SyntheticSample]:
        """Synthetic constant-power acceleration.

        speed_pu ramps 0..1 linearly over 2 s; the q-current reference is held
        at iq_limit_pu = min(Iq_ref, power_limit / speed_pu). This keeps
        speed_pu * iq_pu constant at the limit during the constant-power region.
        """
        result = []
        for number in range(501):
            time_s = number * 0.01
            speed_pu = min(1.0, time_s / 2.0)
            if mode == "no-limit":
                iq_pu = ConstantPowerMetricsTests.IQ_LIMIT_PU
            else:
                iq_pu = min(
                    ConstantPowerMetricsTests.IQ_LIMIT_PU,
                    ConstantPowerMetricsTests.LIMIT_PU / speed_pu
                    if speed_pu > 0
                    else ConstantPowerMetricsTests.IQ_LIMIT_PU,
                )
            power_pu = speed_pu * iq_pu
            firmware_power = (
                power_pu if speed_pu >= 0.3 else 0.0
            )
            result.append(
                SyntheticSample(
                    number,
                    time_s,
                    {
                        "speed_rpm": speed_pu * 500.0,
                        "speed_ref_rpm": 500.0,
                        "iq_a": iq_pu * 5.0,
                        "power_pu": firmware_power,
                    },
                )
            )
        return result

    def test_constant_power_acceleration_passes(self) -> None:
        metrics = analyze_constant_power(
            self.samples(),
            target_speed_rpm=500.0,
            power_limit_pu=self.LIMIT_PU,
        )

        self.assertTrue(metrics["passed"])
        self.assertTrue(metrics["criteria"]["power_limit_engaged"])
        self.assertTrue(metrics["criteria"]["power_bounded_by_limit"])
        self.assertTrue(metrics["criteria"]["power_constant"])
        self.assertTrue(metrics["criteria"]["current_drops_with_speed"])
        self.assertAlmostEqual(metrics["power_pu_window_mean"], self.LIMIT_PU, places=3)

    def test_constant_current_fails_constant_power_acceptance(self) -> None:
        metrics = analyze_constant_power(
            self.samples(mode="no-limit"),
            target_speed_rpm=500.0,
            power_limit_pu=self.LIMIT_PU,
        )

        self.assertFalse(metrics["passed"])
        self.assertFalse(metrics["criteria"]["power_bounded_by_limit"])
        self.assertFalse(metrics["criteria"]["power_constant"])
        self.assertFalse(metrics["criteria"]["current_drops_with_speed"])


if __name__ == "__main__":
    unittest.main()
