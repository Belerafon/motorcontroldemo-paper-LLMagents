from __future__ import annotations

import math
import unittest


ONE_OVER_SQRT3 = 1.0 / math.sqrt(3.0)
SQRT3_OVER_TWO = math.sqrt(3.0) / 2.0


def phase_potentials(alpha: float, beta: float) -> tuple[float, float, float]:
    alpha_scaled = alpha * ONE_OVER_SQRT3
    beta_scaled = beta * ONE_OVER_SQRT3
    return (
        alpha_scaled,
        beta_scaled * SQRT3_OVER_TWO - alpha_scaled / 2.0,
        -beta_scaled * SQRT3_OVER_TWO - alpha_scaled / 2.0,
    )


def centered_duties(
    alpha: float, beta: float, limit: float = 1.0
) -> tuple[tuple[float, float, float], bool]:
    magnitude = math.hypot(alpha, beta)
    limited = magnitude >= limit
    if magnitude and limited:
        scale = limit / magnitude
        alpha *= scale
        beta *= scale
    phases = phase_potentials(alpha, beta)
    common_mode = -(max(phases) + min(phases)) / 2.0
    return tuple(0.5 - (phase + common_mode) for phase in phases), limited


class CenteredSvpwmTests(unittest.TestCase):
    def test_zero_vector_is_centered(self) -> None:
        duties, limited = centered_duties(0.0, 0.0)
        self.assertEqual(duties, (0.5, 0.5, 0.5))
        self.assertFalse(limited)

    def test_no_phase_is_clamped_inside_linear_range(self) -> None:
        for angle_degrees in range(360):
            angle = math.radians(angle_degrees)
            duties, limited = centered_duties(
                0.999 * math.cos(angle),
                0.999 * math.sin(angle),
            )
            self.assertFalse(limited)
            self.assertGreater(min(duties), 0.0)
            self.assertLess(max(duties), 1.0)
            self.assertAlmostEqual(min(duties) + max(duties), 1.0, places=12)

    def test_unit_vector_reaches_bus_at_linear_limit(self) -> None:
        angle = math.radians(30.0)
        duties, limited = centered_duties(math.cos(angle), math.sin(angle))
        self.assertTrue(limited)
        self.assertAlmostEqual(min(duties), 0.0, places=12)
        self.assertAlmostEqual(max(duties), 1.0, places=12)

    def test_common_mode_preserves_line_to_line_voltage(self) -> None:
        phases = phase_potentials(0.72, -0.31)
        duties, _ = centered_duties(0.72, -0.31)
        reconstructed = tuple(0.5 - duty for duty in duties)
        self.assertAlmostEqual(
            reconstructed[0] - reconstructed[1],
            phases[0] - phases[1],
            places=12,
        )
        self.assertAlmostEqual(
            reconstructed[1] - reconstructed[2],
            phases[1] - phases[2],
            places=12,
        )

    def test_centered_svpwm_uses_more_dc_link_than_sinusoidal_pwm(self) -> None:
        alpha = 0.9
        sinusoidal_phase_a_duty = 0.5 - alpha * ONE_OVER_SQRT3
        centered, limited = centered_duties(alpha, 0.0)
        self.assertLess(sinusoidal_phase_a_duty, 0.0)
        self.assertFalse(limited)
        self.assertGreater(min(centered), 0.0)
        self.assertLess(max(centered), 1.0)

    def test_overcommand_is_scaled_to_circular_limit(self) -> None:
        limited_duties, limited = centered_duties(1.1, 0.0)
        boundary_duties, _ = centered_duties(1.0, 0.0)
        self.assertTrue(limited)
        for actual, expected in zip(limited_duties, boundary_duties):
            self.assertAlmostEqual(actual, expected, places=12)


if __name__ == "__main__":
    unittest.main()
