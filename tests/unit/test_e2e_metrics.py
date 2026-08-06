from __future__ import annotations

import math
import unittest
from dataclasses import dataclass

from tests.e2e.metrics import (
    analyze_current_mode4,
    analyze_svpwm_cont,
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


class SvpwmContMetricsTests(unittest.TestCase):
    K_PWM = 10000

    @staticmethod
    def samples(*, clamp_phase_c: bool = False) -> list[SyntheticSample]:
        k_pwm = SvpwmContMetricsTests.K_PWM
        result = []
        # 3 s window at 1 ms -> tail (last 20 %) is steady state at 500 rpm
        for number in range(3001):
            time_s = number * 0.001
            speed = 500.0 * (1.0 - math.exp(-time_s / 0.30))
            # three 120-degree-shifted duty waveforms, continuous SVPWM keeps
            # every compare register strictly inside (0, k_pwm).
            modulation = 0.30 * (1.0 - math.exp(-time_s / 0.30))
            duty_a = 0.5 + modulation * math.sin(2.0 * math.pi * 0.5 * time_s)
            duty_b = 0.5 + modulation * math.sin(
                2.0 * math.pi * 0.5 * time_s - 2.0 * math.pi / 3.0
            )
            duty_c = 0.5 + modulation * math.sin(
                2.0 * math.pi * 0.5 * time_s + 2.0 * math.pi / 3.0
            )
            gamma_a = k_pwm * (1.0 - duty_a)
            gamma_b = k_pwm * (1.0 - duty_b)
            gamma_c = 0.0 if clamp_phase_c else k_pwm * (1.0 - duty_c)
            result.append(
                SyntheticSample(
                    number,
                    time_s,
                    {
                        "speed_rpm": speed,
                        "gamma_a": gamma_a,
                        "gamma_b": gamma_b,
                        "gamma_c": gamma_c,
                    },
                )
            )
        return result

    def test_continuous_modulation_passes(self) -> None:
        metrics = analyze_svpwm_cont(self.samples(), k_pwm=self.K_PWM)

        self.assertTrue(metrics["passed"])
        self.assertFalse(
            any(info["clamped_to_rail"] for info in metrics["phases"].values())
        )
        self.assertGreater(metrics["min_rail_distance_percent"], 2.0)

    def test_clamped_phase_fails_acceptance(self) -> None:
        metrics = analyze_svpwm_cont(
            self.samples(clamp_phase_c=True), k_pwm=self.K_PWM
        )

        self.assertFalse(metrics["passed"])
        self.assertFalse(metrics["criteria"]["no_phase_clamped_to_rail"])
        self.assertFalse(metrics["criteria"]["continuous_modulation_margin"])
        self.assertTrue(metrics["phases"]["gamma_c"]["clamped_to_rail"])


if __name__ == "__main__":
    unittest.main()
