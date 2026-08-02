from __future__ import annotations

import math
import unittest
from dataclasses import dataclass

from tests.e2e.metrics import (
    analyze_current_mode4,
    analyze_dynamic_limits,
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


class DynamicLimitsMetricsTests(unittest.TestCase):
    @staticmethod
    def baseline(*, udc: float = 0.66, pwm_ulim: float = 0.866, iq_ref: float = 1.0):
        scale_v = 20.0
        scale_a = 5.0
        pid_iq = {
            "output_max_v": udc * pwm_ulim * scale_v,
            "output_min_v": -udc * pwm_ulim * scale_v,
        }
        pid_id = {
            "output_max_v": udc * pwm_ulim * scale_v,
            "output_min_v": -udc * pwm_ulim * scale_v,
        }
        pid_spd = {
            "output_max_v": iq_ref * udc * scale_a,
            "output_min_v": 0.0,
        }
        return {
            "udc_before": udc,
            "udc_after": udc,
            "pwm_ulim_before": pwm_ulim,
            "pwm_ulim_after": pwm_ulim * 0.5,
            "iq_ref_before": iq_ref,
            "iq_ref_after": iq_ref * 0.5,
            "pid_iq_baseline": pid_iq,
            "pid_iq_pwm_scaled": {
                "output_max_v": pid_iq["output_max_v"] * 0.5,
                "output_min_v": pid_iq["output_min_v"] * 0.5,
            },
            "pid_iq_udc_scaled": pid_iq,
            "pid_id_baseline": pid_id,
            "pid_id_pwm_scaled": {
                "output_max_v": pid_id["output_max_v"] * 0.5,
                "output_min_v": pid_id["output_min_v"] * 0.5,
            },
            "pid_id_udc_scaled": pid_id,
            "pid_spd_baseline": pid_spd,
            "pid_spd_iq_scaled": {
                "output_max_v": pid_spd["output_max_v"] * 0.5,
                "output_min_v": pid_spd["output_min_v"] * 0.5,
            },
            "pid_spd_udc_scaled": pid_spd,
            "voltage_scale": 0.5,
            "limit_tolerance": 0.08,
        }

    def test_scaled_limits_pass(self) -> None:
        metrics = analyze_dynamic_limits(**self.baseline())
        self.assertTrue(metrics["passed"])
        self.assertTrue(metrics["criteria"]["iq_pwm_limit_scaled"])
        self.assertTrue(metrics["criteria"]["id_pwm_limit_scaled"])
        self.assertTrue(metrics["criteria"]["spd_iq_limit_scaled"])

    def test_clamped_pwm_ulim_scales_correctly(self) -> None:
        # When pwm.U_lim is clamped by pwm_max_voltage, scaling should still work
        udc = 0.66
        pwm_ulim = 1.0  # clamped to ~0.866
        iq_ref = 1.0
        scale_v = 20.0
        scale_a = 5.0
        estimated_effective_ulim = udc * 0.866 * scale_v
        pid_iq = {
            "output_max_v": estimated_effective_ulim,
            "output_min_v": -estimated_effective_ulim,
        }
        pid_id = {
            "output_max_v": estimated_effective_ulim,
            "output_min_v": -estimated_effective_ulim,
        }
        pid_spd = {
            "output_max_v": iq_ref * udc * scale_a,
            "output_min_v": 0.0,
        }
        scaled_pwm = pwm_ulim * 0.5
        metrics = analyze_dynamic_limits(
            udc_before=udc,
            udc_after=udc,
            pwm_ulim_before=pwm_ulim,
            pwm_ulim_after=scaled_pwm,
            iq_ref_before=iq_ref,
            iq_ref_after=iq_ref,
            pid_iq_baseline=pid_iq,
            pid_iq_pwm_scaled={
                "output_max_v": estimated_effective_ulim * 0.5,
                "output_min_v": -estimated_effective_ulim * 0.5,
            },
            pid_iq_udc_scaled=pid_iq,
            pid_id_baseline=pid_id,
            pid_id_pwm_scaled={
                "output_max_v": estimated_effective_ulim * 0.5,
                "output_min_v": -estimated_effective_ulim * 0.5,
            },
            pid_id_udc_scaled=pid_id,
            pid_spd_baseline=pid_spd,
            pid_spd_iq_scaled=pid_spd,
            pid_spd_udc_scaled=pid_spd,
            voltage_scale=0.5,
            limit_tolerance=0.08,
        )
        self.assertTrue(metrics["passed"])
        self.assertTrue(metrics["criteria"]["iq_pwm_limit_scaled"])


if __name__ == "__main__":
    unittest.main()
