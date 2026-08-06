from __future__ import annotations

import math
import unittest
from dataclasses import dataclass

from tests.e2e.metrics import (
    analyze_current_mode4,
    analyze_decoupling,
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
    def _k(lsq_mh: float = 5.0) -> float:
        return lsq_mh * (500.0 * 2.0 * math.pi / 60.0) * 5.0 / (1000.0 * 311.0)

    @classmethod
    def samples(
        cls, *, on: bool, id_peak: float, comp_scale: float = 1.0
    ) -> list[SyntheticSample]:
        k = cls._k()
        result = []
        for number in range(3001):
            time_s = number * 0.001
            speed = 500.0 * (1.0 - math.exp(-time_s / 0.30))
            iq = 1.0 if time_s < 1.0 else 0.05
            iq_pu = iq / 5.0
            w_pu = speed / 500.0
            ud_comp = -k * w_pu * iq_pu * (comp_scale if on else 0.0)
            id_disturb = id_peak if time_s > 0.5 and time_s < 0.8 else 0.0
            result.append(
                SyntheticSample(
                    number,
                    time_s,
                    {
                        "speed_rpm": speed,
                        "iq_a": iq,
                        "id_a": id_disturb if on else id_disturb,
                        "ud_comp_pu": ud_comp,
                    },
                )
            )
        return result

    def test_nominal_decoupling_passes(self) -> None:
        off = self.samples(on=False, id_peak=0.05)
        on = self.samples(on=True, id_peak=0.03)

        metrics = analyze_decoupling(off, on, lsq_mh=5.0)

        self.assertTrue(metrics["passed"])
        self.assertTrue(metrics["criteria"]["feedforward_matches_theory"])
        self.assertTrue(metrics["criteria"]["compensation_active_when_enabled"])
        self.assertTrue(metrics["criteria"]["compensation_zero_when_disabled"])
        self.assertTrue(metrics["id_disturbance_reduced"])
        self.assertAlmostEqual(
            metrics["feedforward_median_relative_error_percent"], 0.0, places=6
        )

    def test_wrong_feedforward_coefficient_fails(self) -> None:
        off = self.samples(on=False, id_peak=0.05)
        on = self.samples(on=True, id_peak=0.03, comp_scale=2.5)

        metrics = analyze_decoupling(
            off,
            on,
            lsq_mh=5.0,
            feedforward_tolerance_percent=30.0,
        )

        self.assertFalse(metrics["passed"])
        self.assertFalse(metrics["criteria"]["feedforward_matches_theory"])

    def test_no_active_coupling_fails_presence_check(self) -> None:
        off = self.samples(on=False, id_peak=0.05)
        on = self.samples(on=True, id_peak=0.03)
        # Force the active-current sample count below the requirement by
        # raising the active-current threshold above the synthetic Iq.
        metrics = analyze_decoupling(
            off, on, lsq_mh=5.0, current_active_min_a=2.0
        )

        self.assertFalse(metrics["criteria"]["coupling_current_present"])


if __name__ == "__main__":
    unittest.main()
