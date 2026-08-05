from __future__ import annotations

import math
import unittest
from dataclasses import dataclass

from tests.e2e.metrics import analyze_current_mode4, analyze_power_limit, analyze_vector_500


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
    def _sample(t: float, power: float, limit: float) -> SyntheticSample:
        return SyntheticSample(
            number=int(t * 100),
            time_s=t,
            values={
                "power_w": power,
                "speed_rpm": 500.0,
                "iq_ref_a": limit,
                "limit_a": limit,
            },
        )

    def phases(self) -> list[dict[str, object]]:
        return [
            {"name": "baseline", "start_s": 0.0, "end_s": 0.8,
             "setpoint_w": None, "active": False},
            {"name": "setpoint_1", "start_s": 0.8, "end_s": 2.6,
             "setpoint_w": 6.0, "active": True},
            {"name": "setpoint_2", "start_s": 2.6, "end_s": 4.3,
             "setpoint_w": 3.5, "active": True},
            {"name": "disabled", "start_s": 4.3, "end_s": 5.8,
             "setpoint_w": None, "active": False},
        ]

    def samples(self, *, sp1_w=6.0, sp2_w=3.5, limit_active=0.5) -> list[SyntheticSample]:
        result: list[SyntheticSample] = []
        # 100 Hz synthetic capture
        for k in range(581):
            t = k * 0.01
            if t < 0.8:
                power, limit = 10.0, 1.0
            elif t < 2.6:
                # exponential settle from 10 W to sp1 with the limiter acting
                settled = sp1_w
                power = settled + (10.0 - settled) * math.exp(-(t - 0.8) / 0.2)
                limit = limit_active
            elif t < 4.3:
                settled = sp2_w
                power = settled + (sp1_w - settled) * math.exp(-(t - 2.6) / 0.2)
                limit = limit_active
            else:
                settled = 10.0
                power = settled + (sp2_w - settled) * math.exp(-(t - 4.3) / 0.2)
                limit = 1.0
            result.append(self._sample(t, power, limit))
        return result

    def test_well_tracked_power_passes(self) -> None:
        metrics = analyze_power_limit(
            self.samples(),
            phases=self.phases(),
            natural_power_w=10.0,
            iq_ref_a=1.0,
        )
        self.assertTrue(metrics["passed"])
        self.assertTrue(metrics["criteria"]["setpoint_1_holds_setpoint"])
        self.assertTrue(metrics["criteria"]["setpoint_2_holds_setpoint"])
        self.assertTrue(metrics["criteria"]["setpoint_1_limiter_active"])
        self.assertTrue(metrics["criteria"]["distinct_setpoint_tracking"])

    def test_poor_setpoint_tracking_fails(self) -> None:
        # second setpoint never settles near its demand
        metrics = analyze_power_limit(
            self.samples(sp2_w=8.0),
            phases=self.phases(),
            natural_power_w=10.0,
            iq_ref_a=1.0,
        )
        self.assertFalse(metrics["passed"])
        self.assertFalse(metrics["criteria"]["setpoint_2_holds_setpoint"])

    def test_limiter_not_active_fails(self) -> None:
        metrics = analyze_power_limit(
            self.samples(limit_active=1.0),
            phases=self.phases(),
            natural_power_w=10.0,
            iq_ref_a=1.0,
        )
        self.assertFalse(metrics["passed"])
        self.assertFalse(metrics["criteria"]["setpoint_1_limiter_active"])


if __name__ == "__main__":
    unittest.main()
