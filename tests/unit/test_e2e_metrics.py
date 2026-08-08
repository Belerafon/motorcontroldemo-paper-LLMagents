from __future__ import annotations

import math
import unittest
from dataclasses import dataclass

from tests.e2e.metrics import (
    analyze_current_mode4,
    analyze_decoupling,
    analyze_decoupling_feedforward,
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
    @staticmethod
    def run_samples(*, decoupled: bool) -> list[SyntheticSample]:
        result = []
        for number in range(401):
            time_s = number * 0.005
            speed = 200.0 * (1.0 - math.exp(-time_s / 0.30))
            iq_ref = 0.8 if time_s < 1.0 else 0.2
            tau = 0.01 if decoupled else 0.05
            iq = 0.2 + (0.8 - 0.2) * math.exp(-(time_s - 1.0) / tau) if time_s >= 1.0 else 0.8
            peak_id = 0.02 if decoupled else 0.25
            id_disturbance = peak_id * math.exp(
                -((time_s - 1.0) / 0.05) ** 2
            )
            result.append(
                SyntheticSample(
                    number,
                    time_s,
                    {
                        "speed_rpm": speed,
                        "iq_ref_a": iq_ref,
                        "iq_a": iq,
                        "id_a": id_disturbance,
                    },
                )
            )
        return result

    def test_decoupling_reduces_dq_errors(self) -> None:
        metrics = analyze_decoupling(
            self.run_samples(decoupled=False),
            self.run_samples(decoupled=True),
            target_speed_rpm=200.0,
            current_limit_a=1.0,
        )

        self.assertTrue(metrics["passed"])
        self.assertGreater(metrics["id_disturbance_reduction_percent"], 10.0)
        self.assertGreater(metrics["iq_tracking_improvement_percent"], 10.0)
        self.assertTrue(metrics["observations"]["d_axis_disturbance_reduced"])
        self.assertTrue(
            metrics["criteria"]["decoupling_active_improves_q_axis_tracking"]
        )

    def test_decoupling_without_benefit_fails(self) -> None:
        metrics = analyze_decoupling(
            self.run_samples(decoupled=False),
            self.run_samples(decoupled=False),
            target_speed_rpm=200.0,
            current_limit_a=1.0,
        )

        self.assertFalse(metrics["passed"])
        self.assertFalse(
            metrics["criteria"]["decoupling_active_improves_q_axis_tracking"]
        )


class DecouplingFeedforwardMetricsTests(unittest.TestCase):
    @staticmethod
    def samples(*, correct: bool = True) -> list[SyntheticSample]:
        k_dq, k_flx = 0.26, 1.70
        speed, iq = 200.0, 3.0
        we_pu, iq_pu = 200.0 / 500.0, 3.0 / 5.0
        expected_udff = k_dq * iq_pu * we_pu
        expected_uqff = k_flx * we_pu
        if not correct:
            expected_udff *= 0.4
            expected_uqff *= 0.4
        result = []
        for number in range(201):
            time_s = number * 0.01
            result.append(
                SyntheticSample(
                    number,
                    time_s,
                    {
                        "ud_ff": expected_udff,
                        "uq_ff": expected_uqff,
                        "speed_rpm": speed,
                        "iq_a": iq,
                    },
                )
            )
        return result

    def test_correct_feedforward_passes(self) -> None:
        metrics = analyze_decoupling_feedforward(
            self.samples(correct=True),
            i_nom_a=5.0,
            speed_nom_rpm=500.0,
            k_dq=0.26,
            k_flx=1.70,
        )

        self.assertTrue(metrics["passed"])
        self.assertTrue(metrics["criteria"]["feedforward_active"])
        self.assertTrue(metrics["d_axis_feedforward_matches"])
        self.assertLess(metrics["d_axis_feedforward_max_error_percent"], 1.0)
        self.assertLess(metrics["q_axis_feedforward_max_error_percent"], 1.0)

    def test_wrong_feedforward_fails(self) -> None:
        metrics = analyze_decoupling_feedforward(
            self.samples(correct=False),
            i_nom_a=5.0,
            speed_nom_rpm=500.0,
            k_dq=0.26,
            k_flx=1.70,
        )

        self.assertFalse(metrics["passed"])
        self.assertFalse(metrics["d_axis_feedforward_matches"])
        self.assertFalse(metrics["criteria"]["q_axis_feedforward_matches"])


if __name__ == "__main__":
    unittest.main()
