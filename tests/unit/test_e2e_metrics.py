from __future__ import annotations

import math
import unittest
from dataclasses import dataclass

from tests.e2e.metrics import (
    analyze_current_mode4,
    analyze_power_limit,
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


class PowerLimitMetricsTests(unittest.TestCase):
    @staticmethod
    def samples(
        *, limit_w: float = 150.0, violation_w: float | None = None
    ) -> list[SyntheticSample]:
        result = []
        for number in range(3001):
            time_s = number * 0.001
            speed = 500.0 * min(time_s / 2.5, 1.0)
            hold = limit_w * min(time_s / 0.6, 1.0)
            power = hold * (1.0 + 0.02 * math.sin(2 * math.pi * 3 * time_s))
            if violation_w is not None:
                if 1.0 <= time_s <= 1.1:
                    power = violation_w
            speed_guard = max(speed, 5.0)
            iq = 3.0 if speed < 5.0 else limit_w * 500.0 / speed_guard
            result.append(
                SyntheticSample(
                    number,
                    time_s,
                    {
                        "speed_rpm": speed,
                        "speed_ref_rpm": 500.0,
                        "iq_ref_a": iq,
                        "power_w": power,
                    },
                )
            )
        return result

    def test_power_limited_acceleration_passes(self) -> None:
        metrics = analyze_power_limit(self.samples(), power_limit_w=150.0)

        self.assertTrue(metrics["passed"])
        self.assertTrue(metrics["criteria"]["power_within_limit"])
        self.assertTrue(metrics["criteria"]["power_limited_engaged"])
        self.assertTrue(metrics["criteria"]["constant_power"])
        self.assertLess(metrics["limited_power_variation_percent"], 30.0)

    def test_power_exceeding_limit_fails(self) -> None:
        metrics = analyze_power_limit(
            self.samples(violation_w=300.0), power_limit_w=150.0
        )

        self.assertFalse(metrics["passed"])
        self.assertFalse(metrics["criteria"]["power_within_limit"])

    def test_limiter_never_engages_fails(self) -> None:
        metrics = analyze_power_limit(self.samples(), power_limit_w=600.0)

        self.assertFalse(metrics["passed"])
        self.assertFalse(metrics["criteria"]["power_limited_engaged"])


if __name__ == "__main__":
    unittest.main()
