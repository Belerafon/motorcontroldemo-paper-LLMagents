"""Analytic validation of the new continuous SVPWM firmware algorithm.

The PWM module in V_PWM_Module.c is a pure function of
``UalphaRef``/``UbetaRef``, ``U_lim`` and ``k_pwm``: the DC-link utilization
and the absence of bus clamping are entirely determined by the modulation
algorithm, not by the motor.  These tests re-implement the exact fixed-point
formulas (SIN and continuous SVPWM with min/max zero-sequence injection) in
floating point and check the claimed properties:

- the sinusoidal PWM reaches the linear limit at a per-unit amplitude of
  ``1/sqrt(3) * sqrt(3) = 0.866`` (the inscribed circle);
- the continuous SVPWM reaches the full per-unit amplitude ``1.0``, i.e.
  about 15.5 % more DC-link voltage, without pinning any phase to a rail;
- the zero-sequence offset is non-zero and disabled at ``SVPWM_offsetK = 0``.
"""

from __future__ import annotations

import math
import unittest

# Exact mirror of the firmware normalization constant _IQ(1/0.866/2) = 1/sqrt(3).
_K_SQRT3 = 1.0 / 0.866 / 2.0
_SQRT3_2 = 0.8660254


def _clamp(value: float, low: float, high: float) -> float:
    return max(low, min(high, value))


def _phase_voltages(
    ualpha: float, ubeta: float, k_scale: float
) -> tuple[float, float, float]:
    """Inverse Clarke in duty-driver units, matching PWM_Module_Sin_Update."""
    ualpha *= k_scale
    ubeta *= k_scale
    va = ualpha
    vb = _SQRT3_2 * ubeta - ualpha / 2.0
    vc = -_SQRT3_2 * ubeta - ualpha / 2.0
    return va, vb, vc


def _limit_vector(
    ualpha: float, ubeta: float, u_lim: float
) -> tuple[float, float]:
    magnitude = math.hypot(ualpha, ubeta)
    if magnitude >= u_lim and magnitude > 0:
        scale = u_lim / magnitude
        return ualpha * scale, ubeta * scale
    return ualpha, ubeta


def sin_duties(
    ualpha: float, ubeta: float, u_lim: float
) -> tuple[float, float, float]:
    ualpha, ubeta = _limit_vector(ualpha, ubeta, u_lim)
    va, vb, vc = _phase_voltages(ualpha, ubeta, _K_SQRT3)
    return 0.5 - va, 0.5 - vb, 0.5 - vc


def svpwm_duties(
    ualpha: float,
    ubeta: float,
    u_lim: float,
    offset_k: float = 1.0,
) -> tuple[tuple[float, float, float], float]:
    """Continuous SVPWM, mirror of PWM_Module_SVPWM_Update.

    Returns ``((duty_a, duty_b, duty_c), offset)``.
    """
    ualpha, ubeta = _limit_vector(ualpha, ubeta, u_lim)
    va, vb, vc = _phase_voltages(ualpha, ubeta, _K_SQRT3)
    vmax = max(va, vb, vc)
    vmin = min(va, vb, vc)
    offset = (vmax + vmin) * 0.5 * offset_k
    va -= offset
    vb -= offset
    vc -= offset
    return (0.5 - va, 0.5 - vb, 0.5 - vc), offset


def max_interior_amplitude(pwm_func, u_lim: float) -> float:
    """Largest per-unit amplitude whose duties stay strictly inside (0, 1)."""
    best = 0.0
    for amplitude in (a / 1000.0 for a in range(0, 1001)):
        interior = True
        for angle in (a / 120.0 * 2.0 * math.pi for a in range(120)):
            duties = pwm_func(
                amplitude * math.cos(angle),
                amplitude * math.sin(angle),
                u_lim,
            )
            if any(d <= 0.0 or d >= 1.0 for d in duties):
                interior = False
                break
        if interior:
            best = amplitude
    return best


class SinusoidalPwmTests(unittest.TestCase):
    def test_linear_limit_at_zero_point_eight_six_six(self) -> None:
        self.assertAlmostEqual(
            max_interior_amplitude(sin_duties, 1.0), 0.866, delta=0.002
        )

    def test_clamps_above_the_linear_limit(self) -> None:
        duties = sin_duties(0.95, 0.0, 1.0)
        self.assertTrue(any(d <= 0.0 or d >= 1.0 for d in duties))


class ContinuousSvpwmTests(unittest.TestCase):
    def test_reaches_full_per_unit_range(self) -> None:
        self.assertAlmostEqual(
            max_interior_amplitude(self._svpwm_duties_only, 1.0),
            1.0,
            delta=0.002,
        )

    @staticmethod
    def _svpwm_duties_only(ualpha, ubeta, u_lim):
        duties, _ = svpwm_duties(ualpha, ubeta, u_lim)
        return duties

    def test_utilization_gain_over_sinusoidal(self) -> None:
        svpwm_max = max_interior_amplitude(self._svpwm_duties_only, 1.0)
        sin_max = max_interior_amplitude(sin_duties, 1.0)
        self.assertGreater(svpwm_max, 0.98)
        self.assertAlmostEqual(sin_max, 0.866, delta=0.002)
        self.assertAlmostEqual(svpwm_max / sin_max, 1.0 / 0.866, delta=0.01)

    def test_no_bus_clamping_in_linear_range(self) -> None:
        for amplitude in (0.5, 0.8, 0.95, 0.99):
            with self.subTest(amplitude=amplitude):
                for angle in (a / 120.0 * 2.0 * math.pi for a in range(120)):
                    duties, _ = svpwm_duties(
                        amplitude * math.cos(angle),
                        amplitude * math.sin(angle),
                        1.0,
                    )
                    self.assertTrue(
                        all(0.0 < d < 1.0 for d in duties),
                        f"bus clamping at amplitude {amplitude}, angle {angle}",
                    )

    def test_sinusoidal_clamps_where_svpwm_does_not(self) -> None:
        duties = sin_duties(0.95, 0.0, 1.0)
        svpwm, _ = svpwm_duties(0.95, 0.0, 1.0)
        self.assertTrue(any(d <= 0.0 or d >= 1.0 for d in duties))
        self.assertTrue(all(0.0 < d < 1.0 for d in svpwm))

    def test_injects_nonzero_zero_sequence(self) -> None:
        (_, _, _), offset = svpwm_duties(0.5, 0.0, 1.0)
        self.assertNotAlmostEqual(offset, 0.0)

    def test_zero_offset_k_reproduces_sinusoidal(self) -> None:
        ualpha, ubeta = 0.5, 0.3
        sa, sb, sc = sin_duties(ualpha, ubeta, 1.0)
        (pa, pb, pc), _ = svpwm_duties(ualpha, ubeta, 1.0, offset_k=0.0)
        for expected, actual in ((sa, pa), (sb, pb), (sc, pc)):
            self.assertAlmostEqual(actual, expected)


if __name__ == "__main__":
    unittest.main()
