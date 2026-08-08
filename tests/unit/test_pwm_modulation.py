from __future__ import annotations

import math
import sys
import unittest
from pathlib import Path


TOOLKIT = (
    Path(__file__).resolve().parents[2]
     / ".opencode/skills/motorcontrol-demo/scripts"
)
sys.path.insert(0, str(TOOLKIT))


# Python mirror of the firmware PWM transfer functions (float domain).
# Bases match V_PWM_Module.c: UalphaRef = 1.0 corresponds to a phase
# amplitude of Udc/sqrt(3) (maximal linear range of a vector modulator).
# Sinusoidal PWM reaches 0.866 (Udc/2), the centered zero-sequence PWM
# reaches 1.0 (Udc/sqrt(3)).

_KINV = 1.0 / 0.866
_HALF = 1.0 / 0.866 / 2.0
_SQ3 = 0.8660254037844386


def _lim(ua: float, ub: float, u_lim: float) -> tuple[float, float, int]:
    mag = math.hypot(ua, ub)
    if mag >= u_lim:
        if mag > 0.0:
            k = u_lim / mag
            ua, ub = k * ua, k * ub
        return ua, ub, 1
    return ua, ub, 0


def sin_duties(ua: float, ub: float, u_lim: float) -> tuple[tuple[float, float, float], int]:
    ua, ub, limited = _lim(ua, ub, u_lim)
    ua_n = ua * _HALF
    ub_n = ub * _HALF
    pt_a = 0.5 - ua_n
    pt_b = 0.5 - (_SQ3 * ub_n - ua_n * 0.5)
    pt_c = 0.5 - (-_SQ3 * ub_n - ua_n * 0.5)
    return (pt_a, pt_b, pt_c), limited


def zero_seq_duties(
    ua: float, ub: float, u_lim: float
) -> tuple[tuple[float, float, float], int]:
    if u_lim > 1.0:
        u_lim = 1.0
    ua, ub, limited = _lim(ua, ub, u_lim)
    ua_n = ua * _HALF
    ub_n = ub * _HALF
    pt_a = 0.5 - ua_n
    pt_b = 0.5 - (_SQ3 * ub_n - ua_n * 0.5)
    pt_c = 0.5 - (-_SQ3 * ub_n - ua_n * 0.5)
    maxp = max(pt_a, pt_b, pt_c)
    minp = min(pt_a, pt_b, pt_c)
    offset = 0.5 - (maxp + minp) / 2.0
    return (pt_a + offset, pt_b + offset, pt_c + offset), limited


class ZeroSeqPwmTests(unittest.TestCase):
    def test_same_fundamental_as_sinusoidal(self) -> None:
        # Zero-sequence injection adds only a common-mode offset, so the
        # line-to-line (and therefore phase) voltages must match sinusoidal.
        for mag in (0.1, 0.3, 0.5, 0.866):
            for ang_deg in (0.0, 12.0, 30.0, 47.0, 61.0, 90.0, 150.0):
                ua = mag * math.cos(math.radians(ang_deg))
                ub = mag * math.sin(math.radians(ang_deg))
                (sa, sb, sc), _ = sin_duties(ua, ub, u_lim=0.866)
                (za, zb, zc), _ = zero_seq_duties(ua, ub, u_lim=0.866)
                for ref, dut in ((sa, za), (sb, zb), (sc, zc)):
                    # Equal difference between any two phases.
                    self.assertAlmostEqual(sa - sb, za - zb, places=6)
                    self.assertAlmostEqual(sa - sc, za - zc, places=6)
                    self.assertAlmostEqual(sb - sc, zb - zc, places=6)
                    self.assertTrue(dut)

    def test_no_bus_tie_in_linear_range(self) -> None:
        # All three phases must switch continuously (no phase tied to a rail)
        # for any unsaturated operating point.
        for mag in (0.05, 0.3, 0.6, 0.9, 0.99):
            for ang_deg in (0.0, 10.0, 20.0, 29.0, 45.0, 59.0, 70.0, 89.0):
                ua = mag * math.cos(math.radians(ang_deg))
                ub = mag * math.sin(math.radians(ang_deg))
                duties, limited = zero_seq_duties(ua, ub, u_lim=1.0)
                self.assertEqual(limited, 0)
                for d in duties:
                    self.assertGreater(d, 1e-9)
                    self.assertLess(d, 1.0 - 1e-9)

    def test_envelope_centered_about_half(self) -> None:
        # The min-max injection centers the duty envelope: min + max == 1.0,
        # i.e. the middle phase sits at the midpoint of the DC rail.
        for mag in (0.2, 0.5, 0.866, 0.98):
            for ang_deg in (0.0, 15.0, 33.0, 61.0, 90.0):
                ua = mag * math.cos(math.radians(ang_deg))
                ub = mag * math.sin(math.radians(ang_deg))
                duties, _ = zero_seq_duties(ua, ub, u_lim=1.0)
                self.assertAlmostEqual(
                    min(duties) + max(duties), 1.0, places=6
                )

    def test_better_dc_utilization_than_sinusoidal(self) -> None:
        # Sinusoidal PWM saturates (duty leaves [0,1]) above a phase amplitude
        # of 0.866*Udc/sqrt(3) = Udc/2; the zero-sequence PWM stays linear up
        # to 1.0*Udc/sqrt(3), i.e. 2/sqrt(3) = 1.155x more DC-bus utilization.
        for mag in (0.866, 0.9, 0.95, 0.99):
            ua = mag
            ub = 0.0
            z_duties, z_limited = zero_seq_duties(ua, ub, u_lim=1.0)
            self.assertEqual(z_limited, 0)
            self.assertTrue(all(1e-9 < d < 1.0 - 1e-9 for d in z_duties))
        # At the theoretical maximum (mag == 1.0) the modulator starts to
        # limit but the phase duties still stay within the clamped [0,1].
        ua = 1.0
        ub = 0.0
        z_duties, _ = zero_seq_duties(ua, ub, u_lim=1.0)
        self.assertTrue(all(-1e-9 <= d <= 1.0 + 1e-9 for d in z_duties))
        # The same input is clipped to 0.866 by the sinusoidal limit and the
        # peak phase ties to the rail (duty reaches 0 or 1), which is exactly
        # the reduced DC-bus utilization the zero-sequence PWM avoids.
        sin_duties_full, sin_limited = sin_duties(1.0, 0.0, u_lim=0.866)
        self.assertEqual(sin_limited, 1)
        self.assertTrue(any(d <= 1e-9 or d >= 1.0 - 1e-9 for d in sin_duties_full))

    def test_zero_input_yields_half_duty(self) -> None:
        duties, limited = zero_seq_duties(0.0, 0.0, u_lim=1.0)
        self.assertEqual(limited, 0)
        for d in duties:
            self.assertAlmostEqual(d, 0.5, places=6)


if __name__ == "__main__":
    unittest.main()
