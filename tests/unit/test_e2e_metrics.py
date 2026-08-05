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
        reach_target: bool = True,
        fw_active: bool = True,
        voltage_under_control: bool = True,
        id_within_limit: bool = True,
    ) -> list[SyntheticSample]:
        result: list[SyntheticSample] = []
        u_lim_pu = 1.0
        target = 1000.0
        id_limit = 1.0
        for number in range(2001):
            time_s = number * 0.005
            # Smooth ramp from 0 to target in ~3 s.
            ramp = 1.0 - math.exp(-time_s / 0.6)
            speed = target * ramp if reach_target else 0.5 * target * ramp
            # FW kicks in as voltage approaches the limit; without it the
            # voltage would saturate and stay at U_lim.
            if fw_active:
                id_ref = -0.30 * ramp
            else:
                id_ref = 0.0
            id_actual = id_ref * (1.0 - math.exp(-time_s / 0.05))
            if not id_within_limit:
                id_actual = -1.30 * ramp
            if voltage_under_control:
                u_mag = (0.85 + 0.03 * math.sin(time_s)) * u_lim_pu * ramp
            else:
                u_mag = 1.30 * u_lim_pu * ramp
            result.append(
                SyntheticSample(
                    number,
                    time_s,
                    {
                        "speed_rpm": speed,
                        "id_ref_a": id_ref,
                        "id_actual_a": id_actual,
                        "u_mag_pu": u_mag,
                    },
                )
            )
        return result

    def test_well_tuned_field_weakening_passes(self) -> None:
        metrics = analyze_field_weakening(
            self.samples(),
            target_speed_rpm=1000.0,
            u_lim_pu=1.0,
            id_limit_a=1.0,
        )

        self.assertTrue(metrics["passed"])
        self.assertLess(metrics["peak_negative_id_ref_a"], 0.0)
        self.assertLessEqual(
            metrics["steady_u_mag_mean_pu"], 1.10 * 1.0
        )

    def test_motor_that_never_reaches_target_fails(self) -> None:
        metrics = analyze_field_weakening(
            self.samples(reach_target=False),
            target_speed_rpm=1000.0,
            u_lim_pu=1.0,
            id_limit_a=1.0,
        )

        self.assertFalse(metrics["passed"])
        self.assertFalse(metrics["criteria"]["reached_95_percent"])

    def test_disabled_field_weakening_fails_activation_check(self) -> None:
        metrics = analyze_field_weakening(
            self.samples(fw_active=False),
            target_speed_rpm=1000.0,
            u_lim_pu=1.0,
            id_limit_a=1.0,
        )

        self.assertFalse(metrics["passed"])
        self.assertFalse(metrics["criteria"]["field_weakening_activated"])

    def test_uncontrolled_voltage_fails_limit_check(self) -> None:
        metrics = analyze_field_weakening(
            self.samples(voltage_under_control=False),
            target_speed_rpm=1000.0,
            u_lim_pu=1.0,
            id_limit_a=1.0,
        )

        self.assertFalse(metrics["passed"])
        self.assertFalse(metrics["criteria"]["voltage_kept_below_limit"])

    def test_overcurrent_id_fails_limit_check(self) -> None:
        metrics = analyze_field_weakening(
            self.samples(id_within_limit=False),
            target_speed_rpm=1000.0,
            u_lim_pu=1.0,
            id_limit_a=1.0,
        )

        self.assertFalse(metrics["passed"])
        self.assertFalse(metrics["criteria"]["id_within_limit"])


if __name__ == "__main__":
    unittest.main()
