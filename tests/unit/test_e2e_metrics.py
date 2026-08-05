from __future__ import annotations

import math
import unittest
from dataclasses import dataclass

from tests.e2e.metrics import (
    analyze_current_mode4,
    analyze_field_weakening,
    analyze_vector_500,
    compare_field_weakening_speed,
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
    ID_SCALE_A = 5.0
    U_REF_PU = 0.8
    U_LIM_PU = 0.866
    ID_MIN_PU = -0.8

    @staticmethod
    def samples(*, engage: bool = True, strengthen: bool = False) -> list[SyntheticSample]:
        result = []
        for number in range(501):
            time_s = number * 0.01
            #Speed ramps up and settles near the target.
            speed = 1500.0 * (1.0 - math.exp(-time_s / 0.5))
            #Voltage magnitude climbs and saturates above U_ref.
            u_mag = min(0.86, 0.2 + 0.8 * (1.0 - math.exp(-time_s / 0.4)))
            #Id reference stays at 0 while voltage is below U_ref, then weakens.
            id_ref_pu = 0.0
            if engage and u_mag >= 0.75:
                id_ref_pu = -0.25 * min(1.0, (time_s - 0.4) / 0.6)
            if strengthen:
                id_ref_pu = max(id_ref_pu, 0.08)
            result.append(
                SyntheticSample(
                    number,
                    time_s,
                    {
                        "speed_rpm": speed,
                        "id_ref_a": id_ref_pu * FieldWeakeningMetricsTests.ID_SCALE_A,
                        "id_a": id_ref_pu * FieldWeakeningMetricsTests.ID_SCALE_A,
                        "u_mag_pu": u_mag,
                    },
                )
            )
        return result

    def test_engaging_run_passes(self) -> None:
        metrics = analyze_field_weakening(
            self.samples(),
            target_speed_rpm=1500.0,
            u_ref_pu=self.U_REF_PU,
            u_lim_pu=self.U_LIM_PU,
            id_min_pu=self.ID_MIN_PU,
            id_scale_a=self.ID_SCALE_A,
        )

        self.assertTrue(metrics["passed"])
        self.assertTrue(metrics["saturation_occurred"])
        self.assertTrue(metrics["field_weakening_engaged"])
        self.assertTrue(metrics["criteria"]["engaged_when_saturated"])
        self.assertTrue(metrics["criteria"]["no_field_strengthening"])

    def test_saturation_without_engagement_fails(self) -> None:
        metrics = analyze_field_weakening(
            self.samples(engage=False),
            target_speed_rpm=1500.0,
            u_ref_pu=self.U_REF_PU,
            u_lim_pu=self.U_LIM_PU,
            id_min_pu=self.ID_MIN_PU,
            id_scale_a=self.ID_SCALE_A,
        )

        self.assertFalse(metrics["passed"])
        self.assertFalse(metrics["criteria"]["engaged_when_saturated"])

    def test_field_strengthening_fails_safety(self) -> None:
        metrics = analyze_field_weakening(
            self.samples(strengthen=True),
            target_speed_rpm=1500.0,
            u_ref_pu=self.U_REF_PU,
            u_lim_pu=self.U_LIM_PU,
            id_min_pu=self.ID_MIN_PU,
            id_scale_a=self.ID_SCALE_A,
        )

        self.assertFalse(metrics["criteria"]["no_field_strengthening"])


class FieldWeakeningSpeedComparisonTests(unittest.TestCase):
    @staticmethod
    def _run(steady_speed: float) -> list[SyntheticSample]:
        result = []
        for number in range(401):
            time_s = number * 0.01
            speed = steady_speed * (1.0 - math.exp(-time_s / 0.3))
            result.append(
                SyntheticSample(
                    number, time_s, {"speed_rpm": speed},
                )
            )
        return result

    def test_higher_speed_with_fw_passes(self) -> None:
        comparison = compare_field_weakening_speed(
            self._run(700.0), self._run(800.0), min_improvement_percent=1.0
        )

        self.assertTrue(
            comparison["criteria"]["field_weaking_extends_speed"]
        )
        self.assertGreater(comparison["speed_improvement_percent"], 1.0)

    def test_no_improvement_fails(self) -> None:
        comparison = compare_field_weakening_speed(
            self._run(800.0), self._run(800.0), min_improvement_percent=1.0
        )

        self.assertFalse(
            comparison["criteria"]["field_weaking_extends_speed"]
        )


if __name__ == "__main__":
    unittest.main()
