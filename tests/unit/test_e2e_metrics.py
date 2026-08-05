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
    @staticmethod
    def samples(*, power_limited: bool = True) -> list[SyntheticSample]:
        #База: 500 об/мин, 5 А; предел тока 1.0 о.е., предел мощности 0.3 о.е.
        #Угловая скорость = P_limit/Iq_max = 0.3 о.е. = 150 об/мин.
        result: list[SyntheticSample] = []
        base_speed = 500.0
        base_current = 5.0
        iq_limit_pu = 1.0
        power_limit_pu = 0.3
        corner_pu = power_limit_pu / iq_limit_pu
        for number in range(4001):
            time_s = number * 0.001
            speed_pu = min(1.0, 1.0 - math.exp(-time_s / 0.45))
            speed_rpm = speed_pu * base_speed
            if power_limited and speed_pu > corner_pu:
                iq_pu = power_limit_pu / speed_pu
            else:
                iq_pu = iq_limit_pu
            power_pu = iq_pu * speed_pu
            ceiling_pu = min(iq_limit_pu, power_limit_pu / speed_pu) if speed_pu > 0 else iq_limit_pu
            result.append(
                SyntheticSample(
                    number,
                    time_s,
                    {
                        "speed_rpm": speed_rpm,
                        "iq_a": iq_pu * base_current,
                        "power_pu": power_pu,
                        "iq_ceiling_pu": ceiling_pu,
                    },
                )
            )
        return result

    def test_constant_power_acceleration_passes(self) -> None:
        metrics = analyze_constant_power(
            self.samples(),
            target_speed_rpm=500.0,
            power_limit_pu=0.3,
            current_limit_a=5.0,
        )

        self.assertTrue(metrics["passed"])
        self.assertLess(metrics["region_power_cv"], 0.45)
        self.assertTrue(metrics["criteria"]["current_decreases_with_speed"])
        self.assertTrue(metrics["criteria"]["power_near_limit"])

    def test_unlimited_current_fails_flat_power(self) -> None:
        metrics = analyze_constant_power(
            self.samples(power_limited=False),
            target_speed_rpm=500.0,
            power_limit_pu=0.3,
            current_limit_a=5.0,
        )

        self.assertFalse(metrics["passed"])
        self.assertFalse(metrics["criteria"]["power_is_flat"])
        self.assertFalse(metrics["criteria"]["current_decreases_with_speed"])


if __name__ == "__main__":
    unittest.main()
