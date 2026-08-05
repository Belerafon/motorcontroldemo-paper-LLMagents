from __future__ import annotations

import math
import unittest

"""
Reference implementation of the new centered-SVPWM PWM type
(PWM_TYPE_SVPWM_CENTERED, see Vsrc/V_PWM_Module.c:
PWM_Module_SVPWM_Centered_Update) and of the existing sinusoidal PWM
(PWM_Module_Sin_Update), kept in real numbers.

Both functions consume UalphaRef/UbetaRef in the firmware units used by
TPWM_Module: UalphaRef == 1.0 corresponds to the linear-range phase
voltage peak Udc/sqrt(3) (i.e. to the inscribed-circle radius of the
basic-vector hexagon). They return the three Gamma compare values in the
same units as NT_PWMx->CMPA, i.e. integers in [0, k_pwm + 1].

The tests below validate the algebraic properties claimed in the firmware
comments without touching hardware. They are the analytic counterpart of
the hardware e2e scenario tests/e2e/pwm_svpwm.py.
"""

K_PWM = 10000
_SQRT3 = math.sqrt(3.0)
_SVPWM_LINEAR_LIMIT = 1.0 / 0.866  # = 2/sqrt(3), basic-vector normalization
_NORM_INPUT_SCALE = 0.866          # multiply applied inside PWM_Module_NormInput
_SIN_PWM_SCALE = 1.0 / 0.866 / 2.0  # = 1/sqrt(3), applied inside PWM_Module_Sin_Update
_TWO_THIRDS = 2.0 / 3.0
_HALF_SQRT3 = _SQRT3 / 2.0


def _saturate_gamma(value: float) -> float:
    if value < 0:
        return 0.0
    if value > K_PWM:
        return K_PWM + 1
    return value


def _norm_input(ualpha_ref: float, ubeta_ref: float) -> tuple[float, float]:
    """Mirrors PWM_Module_NormInput without Ud compensation/magnitude limit
    envelope, matching the implementation for the linear range."""
    magnitude = math.hypot(ualpha_ref, ubeta_ref)
    limit = _SVPWM_LINEAR_LIMIT
    if magnitude >= limit:
        scale = limit / magnitude
        ualpha_ref *= scale
        ubeta_ref *= scale
    ualpha_ref *= _NORM_INPUT_SCALE
    ubeta_ref *= _NORM_INPUT_SCALE
    return ualpha_ref, ubeta_ref


def sin_pwm(ualpha_ref: float, ubeta_ref: float) -> tuple[float, float, float]:
    """Reference for PWM_Module_Sin_Update."""
    ualpha = ualpha_ref
    ubeta = ubeta_ref
    magnitude = math.hypot(ualpha, ubeta)
    limit = 0.866
    if magnitude >= limit:
        scale = limit / magnitude
        ualpha *= scale
        ubeta *= scale
    ualpha *= _SIN_PWM_SCALE
    ubeta *= _SIN_PWM_SCALE

    phase_a = 0.5 - ualpha
    phase_b = 0.5 - (ubeta * _HALF_SQRT3 - ualpha * 0.5)
    phase_c = 0.5 - (-ubeta * _HALF_SQRT3 - ualpha * 0.5)
    gamma_a = _saturate_gamma(K_PWM * phase_a)
    gamma_b = _saturate_gamma(K_PWM * phase_b)
    gamma_c = _saturate_gamma(K_PWM * phase_c)
    return gamma_a, gamma_b, gamma_c


def svpwm_centered(
    ualpha_ref: float, ubeta_ref: float, *, k_inj: float = 1.0
) -> tuple[float, float, float]:
    """Reference for PWM_Module_SVPWM_Centered_Update.

    k_inj mirrors the runtime knob Debug1 (IQ24, 0..1):
      * 1.0 - full min/max zero-sequence injection (centered SVPWM);
      * 0.0 - no injection, must reproduce sin_pwm.
    """
    ualpha, ubeta = _norm_input(ualpha_ref, ubeta_ref)
    k_inj_clamped = max(0.0, min(1.0, k_inj))

    ua = ualpha
    ub = -0.5 * ualpha + _HALF_SQRT3 * ubeta
    uc = -0.5 * ualpha - _HALF_SQRT3 * ubeta

    vmax = max(ua, ub, uc)
    vmin = min(ua, ub, uc)
    vzero = -0.5 * (vmax + vmin) * k_inj_clamped

    uan = ua + vzero
    ubn = ub + vzero
    ucn = uc + vzero

    phase_a = 0.5 - uan * _TWO_THIRDS
    phase_b = 0.5 - ubn * _TWO_THIRDS
    phase_c = 0.5 - ucn * _TWO_THIRDS
    gamma_a = _saturate_gamma(K_PWM * phase_a)
    gamma_b = _saturate_gamma(K_PWM * phase_b)
    gamma_c = _saturate_gamma(K_PWM * phase_c)
    return gamma_a, gamma_b, gamma_c


def gamma_to_duty(gamma: float) -> float:
    """High-side duty cycle in [0, 1] for a given CMPA value (k_pwm - gamma)/k_pwm."""
    return (K_PWM - gamma) / K_PWM


def gamma_to_van(gamma_a: float, gamma_b: float, gamma_c: float) -> float:
    """Phase-A line-to-neutral voltage in Udc units implied by the three Gamma
    compare values, assuming a balanced 3-phase inverter with midpoint-referenced
    phase voltages (duty - 0.5) * Udc."""
    duty_a = gamma_to_duty(gamma_a)
    duty_b = gamma_to_duty(gamma_b)
    duty_c = gamma_to_duty(gamma_c)
    van_midpoint = duty_a - 0.5
    vbn_midpoint = duty_b - 0.5
    vcn_midpoint = duty_c - 0.5
    common = (van_midpoint + vbn_midpoint + vcn_midpoint) / 3.0
    return van_midpoint - common


def gamma_to_phase_voltage_magnitude(
    gamma_a: float, gamma_b: float, gamma_c: float
) -> float:
    """Magnitude (peak phase voltage) of the balanced 3-phase voltage vector
    synthesized by the three Gamma compare values, in Udc units."""
    duty_a = gamma_to_duty(gamma_a)
    duty_b = gamma_to_duty(gamma_b)
    duty_c = gamma_to_duty(gamma_c)
    van = duty_a - 0.5
    vbn = duty_b - 0.5
    vcn = duty_c - 0.5
    common = (van + vbn + vcn) / 3.0
    van -= common
    vbn -= common
    vcn -= common
    # alpha-beta (amplitude invariant): alpha = va, beta = (vb - vc)/sqrt(3)
    alpha = van
    beta = (vbn - vcn) / _SQRT3
    return math.hypot(alpha, beta)


def gamma_to_vab(gamma_a: float, gamma_b: float, gamma_c: float) -> float:
    duty_a = gamma_to_duty(gamma_a)
    duty_b = gamma_to_duty(gamma_b)
    return (duty_a - duty_b)


class CenteredSvpwmMathTests(unittest.TestCase):
    def test_zero_input_gives_fifty_percent_duty_on_all_phases(self) -> None:
        gammas = svpwm_centered(0.0, 0.0)
        for gamma in gammas:
            self.assertAlmostEqual(gamma, K_PWM / 2)
            self.assertAlmostEqual(gamma_to_duty(gamma), 0.5)

    def test_k_inj_zero_matches_sin_pwm(self) -> None:
        # _NORM_INPUT_SCALE uses the firmware literal 0.866 (not sqrt(3)/2),
        # so SVPWM(k_inj=0) differs from sin_pwm by at most a few PWM LSB even
        # at UalphaRef = 0.866. The reference behaviour (SPWM-equivalent) is
        # still preserved within the quantization floor of the compare registers.
        for ualpha, ubeta in [
            (0.0, 0.0),
            (0.3, 0.0),
            (0.5, 0.2),
            (0.7, -0.4),
            (-0.6, 0.6),
            (0.866, 0.0),
        ]:
            with self.subTest(ualpha=ualpha, ubeta=ubeta):
                ref = sin_pwm(ualpha, ubeta)
                got = svpwm_centered(ualpha, ubeta, k_inj=0.0)
                for actual, expected in zip(got, ref):
                    self.assertAlmostEqual(actual, expected, delta=1.0)

    def test_phase_voltage_matches_sin_pwm_for_same_reference(self) -> None:
        for ualpha, ubeta in [(0.5, 0.0), (0.3, 0.4), (0.6, -0.3), (-0.55, 0.55)]:
            with self.subTest(ualpha=ualpha, ubeta=ubeta):
                sin_van = gamma_to_van(*sin_pwm(ualpha, ubeta))
                sv_van = gamma_to_van(*svpwm_centered(ualpha, ubeta, k_inj=1.0))
                self.assertAlmostEqual(sin_van, sv_van, places=3)

    def test_svpwm_delivers_more_phase_voltage_than_sin_pwm_at_unity(self) -> None:
        # At |UalphaRef| = 1.0 (above the SPWM linear limit of 0.866 but inside
        # the centered-SVPWM linear limit of 1/0.866), centered SVPWM must
        # deliver more phase voltage than sin PWM fed the same reference,
        # because sin PWM has already entered its input clamp.
        for angle_deg in range(0, 360, 30):
            angle = math.radians(angle_deg)
            ualpha = math.cos(angle)
            ubeta = math.sin(angle)
            sin_v = gamma_to_phase_voltage_magnitude(*sin_pwm(ualpha, ubeta))
            sv_v = gamma_to_phase_voltage_magnitude(
                *svpwm_centered(ualpha, ubeta, k_inj=1.0)
            )
            with self.subTest(angle=angle_deg):
                self.assertGreater(sv_v, sin_v + 1e-3)

    def test_no_phase_is_clamped_inside_linear_range(self) -> None:
        # Magnitudes strictly below the inscribed-circle radius (1.0 in the
        # basic-vector normalization) are inside the SVPWM linear hexagon for
        # every angle, so every Gamma must stay off both rails.
        for angle_deg in range(0, 360, 10):
            angle = math.radians(angle_deg)
            for magnitude in (0.2, 0.5, 0.8, 0.95):
                ualpha = magnitude * math.cos(angle)
                ubeta = magnitude * math.sin(angle)
                gammas = svpwm_centered(ualpha, ubeta, k_inj=1.0)
                with self.subTest(angle=angle_deg, magnitude=magnitude):
                    for gamma in gammas:
                        self.assertGreater(gamma, 1.0)
                        self.assertLess(gamma, K_PWM)

    def test_saturation_at_linear_limit_is_basic_vector(self) -> None:
        # At UalphaRef = 1/0.866 along +alpha, the requested vector is exactly
        # basic vector V1 = (100): phase A clamped to the high rail (Gamma = 0),
        # phases B,C clamped to the low rail (Gamma = k_pwm at the CMPA = TBPRD
        # boundary; the existing No_SV implementation produces the same value).
        gammas = svpwm_centered(_SVPWM_LINEAR_LIMIT, 0.0, k_inj=1.0)
        self.assertAlmostEqual(min(gammas), 0.0, delta=1.0)
        self.assertAlmostEqual(max(gammas), K_PWM, delta=1.0)

    def test_voltage_scales_linearly_with_reference_below_limit(self) -> None:
        vzero_input = gamma_to_van(*svpwm_centered(0.0, 0.0, k_inj=1.0))
        self.assertAlmostEqual(vzero_input, 0.0, places=6)
        v_small = gamma_to_van(*svpwm_centered(0.2, 0.0, k_inj=1.0))
        v_large = gamma_to_van(*svpwm_centered(0.8, 0.0, k_inj=1.0))
        self.assertAlmostEqual(v_large, 4.0 * v_small, delta=1e-3)

    def test_zero_sequence_injection_is_third_harmonic_of_phase_voltage(self) -> None:
        # For balanced sinusoidal UalphaRef/UbetaRef, the centered-SVPWM common
        # mode is the third harmonic of the phase voltage with relative
        # amplitude 1/4 (k_inj = 1): Vzero = -cos(3*theta)/4 in UalphaNorm units.
        magnitude = 0.6
        max_ratio = 0.0
        for angle_deg in range(0, 360, 5):
            angle = math.radians(angle_deg)
            ualpha = magnitude * math.cos(angle)
            ubeta = magnitude * math.sin(angle)
            ualpha_n, ubeta_n = _norm_input(ualpha, ubeta)
            ua = ualpha_n
            ub = -0.5 * ualpha_n + _HALF_SQRT3 * ubeta_n
            uc = -0.5 * ualpha_n - _HALF_SQRT3 * ubeta_n
            vmax = max(ua, ub, uc)
            vmin = min(ua, ub, uc)
            vzero = -0.5 * (vmax + vmin)
            max_phase = max(abs(ua), abs(ub), abs(uc))
            if max_phase > 1e-6:
                max_ratio = max(max_ratio, abs(vzero) / max_phase)
        # Theoretical ratio: 0.25 (1/4). Numerical floor from 0.866 literal
        # adds a small slack.
        self.assertLess(max_ratio, 0.26)
        self.assertGreater(max_ratio, 0.24)

    def test_k_inj_above_one_is_clamped_to_full_svpwm(self) -> None:
        for k_inj in (1.0, 1.5, 2.0, 100.0):
            with self.subTest(k_inj=k_inj):
                gammas = svpwm_centered(0.5, 0.3, k_inj=k_inj)
                reference = svpwm_centered(0.5, 0.3, k_inj=1.0)
                for actual, expected in zip(gammas, reference):
                    self.assertAlmostEqual(actual, expected, places=4)

    def test_k_inj_negative_is_clamped_to_zero(self) -> None:
        for k_inj in (0.0, -0.5, -10.0):
            with self.subTest(k_inj=k_inj):
                gammas = svpwm_centered(0.5, 0.3, k_inj=k_inj)
                reference = svpwm_centered(0.5, 0.3, k_inj=0.0)
                for actual, expected in zip(gammas, reference):
                    self.assertAlmostEqual(actual, expected, places=4)


if __name__ == "__main__":
    unittest.main()
