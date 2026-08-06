from __future__ import annotations

import math
import unittest
from dataclasses import dataclass

from tests.e2e.metrics import (
    analyze_current_mode4,
    analyze_field_weakening,
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


class FieldWeakeningMetricsTests(unittest.TestCase):
    @staticmethod
    def samples(
        *, final_speed: float, id_ref: float = 0.0, fw_err: float = 0.0
    ) -> list[SyntheticSample]:
        result = []
        for number in range(601):
            time_s = number * 0.01
            speed = final_speed * (1.0 - math.exp(-time_s / 0.30))
            if id_ref < 0:
                id_ref_series = id_ref * (1.0 - math.exp(-time_s / 0.15))
            else:
                id_ref_series = 0.0
            result.append(
                SyntheticSample(
                    number,
                    time_s,
                    {
                        "speed_rpm": speed,
                        "id_ref_a": id_ref_series,
                        "id_a": id_ref_series,
                        "fw_err_pu": fw_err * (1.0 - math.exp(-time_s / 0.15)),
                    },
                )
            )
        return result

    def test_field_weakening_enables_speed_above_limit(self) -> None:
        metrics = analyze_field_weakening(
            self.samples(final_speed=742.0, id_ref=-2.5, fw_err=0.12),
            self.samples(final_speed=680.0),
            target_speed_rpm=740.0,
        )

        self.assertTrue(metrics["passed"])
        self.assertTrue(metrics["criteria"]["weakening_activated"])
        self.assertTrue(metrics["criteria"]["reached_target_with_weakening"])
        self.assertTrue(metrics["criteria"]["blocked_without_weakening"])
        self.assertGreater(metrics["speed_gain_rpm"], 20.0)

    def test_no_weakening_demand_fails_acceptance(self) -> None:
        metrics = analyze_field_weakening(
            self.samples(final_speed=700.0, id_ref=0.0, fw_err=0.0),
            self.samples(final_speed=680.0),
            target_speed_rpm=740.0,
        )

        self.assertFalse(metrics["passed"])
        self.assertFalse(metrics["criteria"]["weakening_activated"])
        self.assertFalse(metrics["criteria"]["voltage_limit_hit"])

    def test_weakening_not_needed_fails_if_speed_gain_is_small(self) -> None:
        metrics = analyze_field_weakening(
            self.samples(final_speed=695.0, id_ref=-0.5, fw_err=0.05),
            self.samples(final_speed=685.0),
            target_speed_rpm=740.0,
        )

        self.assertFalse(metrics["passed"])
        self.assertFalse(metrics["criteria"]["reached_target_with_weakening"])
        self.assertFalse(metrics["criteria"]["meaningful_speed_gain"])


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


if __name__ == "__main__":
    unittest.main()
