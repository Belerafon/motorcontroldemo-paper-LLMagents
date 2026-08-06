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


class FieldWeakeningMetricsTests(unittest.TestCase):
    @staticmethod
    def samples(
        *,
        fw_active: bool = True,
        settled: bool = True,
        mod_index_peak: float = 1.15,
        steady_mod_index: float = 0.92,
        steady_speed_rpm: float = 750.0,
        speed_ripple_rpm: float = 5.0,
    ) -> list[SyntheticSample]:
        result = []
        total_s = 5.0
        period = 0.025
        n = int(total_s / period)
        for number in range(n + 1):
            time_s = number * period
            ramp_frac = min(1.0, time_s / 1.2)
            if fw_active:
                id_ref = -0.5 * ramp_frac if time_s < 1.0 else -0.5
            else:
                id_ref = 0.0
            mod_index = mod_index_peak - (
                mod_index_peak - steady_mod_index
            ) * ramp_frac
            iq = 0.05 + 0.02 * math.sin(2 * math.pi * 7 * time_s)
            if settled:
                speed = 100.0 + (steady_speed_rpm - 100.0) * ramp_frac
                speed += speed_ripple_rpm * math.sin(2 * math.pi * 13 * time_s)
            else:
                speed = 100.0 + 400.0 * ramp_frac
                speed += 80.0 * math.sin(2 * math.pi * 3 * time_s)
            result.append(
                SyntheticSample(
                    number,
                    time_s,
                    {
                        "speed_rpm": speed,
                        "mod_index_pu": mod_index,
                        "id_ref_a": id_ref,
                        "iq_a": iq,
                    },
                )
            )
        return result

    def test_healthy_fw_activation_passes(self) -> None:
        metrics = analyze_field_weakening(
            self.samples(),
            target_speed_rpm=1500.0,
        )

        self.assertTrue(metrics["passed"])
        self.assertTrue(metrics["criteria"]["field_weakening_activated"])
        self.assertTrue(metrics["criteria"]["speed_above_base_region"])
        self.assertLess(metrics["steady_mod_index_mean_pu"], 1.0)

    def test_inactive_fw_fails_acceptance(self) -> None:
        metrics = analyze_field_weakening(
            self.samples(fw_active=False),
            target_speed_rpm=1500.0,
        )

        self.assertFalse(metrics["passed"])
        self.assertFalse(metrics["criteria"]["field_weakening_activated"])

    def test_unstable_speed_fails_settling(self) -> None:
        metrics = analyze_field_weakening(
            self.samples(settled=False),
            target_speed_rpm=1500.0,
        )

        self.assertFalse(metrics["passed"])
        self.assertFalse(metrics["criteria"]["speed_settled_to_plateau"])
        self.assertFalse(metrics["criteria"]["speed_above_base_region"])

    def test_deeply_saturated_inverter_fails(self) -> None:
        metrics = analyze_field_weakening(
            self.samples(steady_mod_index=1.30, mod_index_peak=1.45),
            target_speed_rpm=1500.0,
        )

        self.assertFalse(metrics["passed"])
        self.assertFalse(
            metrics["criteria"]["modulation_index_regulated_in_steady_state"]
        )


if __name__ == "__main__":
    unittest.main()
