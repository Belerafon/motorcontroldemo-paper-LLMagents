from __future__ import annotations

import math
import unittest
from dataclasses import dataclass

from tests.e2e.metrics import (
    analyze_current_mode4,
    analyze_decoupling,
    analyze_vector_500,
    calibrate_decoupling_gain,
    decouple_metric,
)


@dataclass(frozen=True)
class SyntheticSample:
    number: int
    time_s: float
    values: dict[str, float]
    raw: dict[str, int] | None = None


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


class DecouplingCalibrationTests(unittest.TestCase):
    @staticmethod
    def raw_samples(
        gain: float,
        speed_pu: float = 0.8,
        iq_pu: float = 0.5,
        count: int = 50,
    ) -> list[SyntheticSample]:
        scale = 1 << 24
        speed_raw = int(round(speed_pu * scale)) & 0xFFFF_FFFF
        iq_raw = int(round(iq_pu * scale)) & 0xFFFF_FFFF
        ud_raw = int(round(-gain * speed_pu * iq_pu * scale)) & 0xFFFF_FFFF
        result = []
        for number in range(count):
            result.append(
                SyntheticSample(
                    number,
                    number * 0.001,
                    {"speed_rpm": speed_pu, "ud_v": -gain * speed_pu * iq_pu, "iq_a": iq_pu},
                    {"speed_rpm": speed_raw, "ud_v": ud_raw, "iq_a": iq_raw},
                )
            )
        return result

    def test_recovers_known_coupling_gain(self) -> None:
        calibrated = calibrate_decoupling_gain(
            self.raw_samples(gain=0.02)
        )

        self.assertAlmostEqual(calibrated, 0.02, delta=0.005)

    def test_negative_speed_direction_preserves_sign(self) -> None:
        calibrated = calibrate_decoupling_gain(
            self.raw_samples(gain=0.02, speed_pu=-0.8)
        )

        self.assertAlmostEqual(calibrated, 0.02, delta=0.005)


class DecoupleMetricTests(unittest.TestCase):
    @staticmethod
    def samples(
        *,
        ud_high: float,
        ud_low: float,
        with_dec: bool = False,
    ) -> list[SyntheticSample]:
        result = []
        for number in range(101):
            time_s = number * 0.01
            speed = 600.0 if number >= 50 else 100.0
            ud = ud_high if number >= 50 else ud_low
            values: dict[str, float] = {
                "speed_rpm": speed,
                "ud_v": ud,
                "iq_a": 0.5,
                "id_a": 0.02,
            }
            raw = {
                "speed_rpm": int(round(speed / 500.0 * (1 << 24))),
                "ud_v": int(round(ud / 20.0 * (1 << 24))),
                "iq_a": int(round(0.5 * (1 << 24))),
            }
            if with_dec:
                values["dec_v"] = ud_high
            result.append(SyntheticSample(number, time_s, values, raw))
        return result

    def test_mean_abs_ud_uses_high_speed_window(self) -> None:
        metric = decouple_metric(
            self.samples(ud_high=3.0, ud_low=0.1),
            target_speed_rpm=500.0,
        )

        self.assertEqual(metric["high_speed_points"], 51)
        self.assertAlmostEqual(metric["mean_abs_ud_v"], 3.0)
        self.assertEqual(metric["peak_abs_speed_rpm"], 600.0)


class AnalyzeDecouplingTests(unittest.TestCase):
    @staticmethod
    def metric(
        mean_ud: float,
        *,
        coupling_coeff: float | None = -3.0,
        dec: float | None = None,
    ) -> dict[str, object]:
        return {
            "peak_abs_speed_rpm": 510.0,
            "coupling_coeff": coupling_coeff,
            "mean_abs_ud_v": mean_ud,
            "peak_abs_ud_v": mean_ud,
            "peak_abs_id_a": 0.1,
            "peak_abs_dec_v": dec,
        }

    def test_decoupling_on_relieves_d_regulator(self) -> None:
        off = self.metric(mean_ud=3.0)
        on = self.metric(mean_ud=0.9, coupling_coeff=-0.9, dec=3.0)

        metrics = analyze_decoupling(off, on, target_speed_rpm=500.0)

        self.assertTrue(metrics["passed"])
        self.assertTrue(metrics["criteria"]["d_regulator_relieved"])
        self.assertLess(metrics["coupling_coeff_ratio"], 0.6)

    def test_no_improvement_fails_acceptance(self) -> None:
        off = self.metric(mean_ud=3.0)
        on = self.metric(mean_ud=2.8, coupling_coeff=-2.8, dec=3.0)

        metrics = analyze_decoupling(off, on, target_speed_rpm=500.0)

        self.assertFalse(metrics["passed"])
        self.assertFalse(metrics["criteria"]["d_regulator_relieved"])

    def test_inactive_decoupling_fails_acceptance(self) -> None:
        off = self.metric(mean_ud=3.0)
        on = self.metric(mean_ud=0.9, coupling_coeff=-0.9, dec=0.0)

        metrics = analyze_decoupling(off, on, target_speed_rpm=500.0)

        self.assertFalse(metrics["criteria"]["decoupling_active"])


if __name__ == "__main__":
    unittest.main()
