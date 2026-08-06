from __future__ import annotations

import unittest

from tests.e2e.cur_loop_lim_model import compute_dynamic_limit


class DynamicVoltageLimitTests(unittest.TestCase):
    """Pure-Python модель логики V_CurLoopLim.c (расчёт без железа)."""

    def test_disabled_returns_static_unit(self) -> None:
        self.assertAlmostEqual(
            compute_dynamic_limit(
                u_lim=0.866, udc_meas=0.9, k_dyn=1.0,
                u_dyn_min=0.05, enabled=False,
            ),
            1.0,
        )

    def test_nominal_udc_full_sensitivity_equals_u_lim(self) -> None:
        self.assertAlmostEqual(
            compute_dynamic_limit(
                u_lim=0.866, udc_meas=1.0, k_dyn=1.0,
                u_dyn_min=0.05, enabled=True,
            ),
            0.866,
        )

    def test_udc_sag_scales_limit_down(self) -> None:
        # Ud = 0.8 номинала -> предел 0.8 от U_lim.
        self.assertAlmostEqual(
            compute_dynamic_limit(
                u_lim=0.866, udc_meas=0.8, k_dyn=1.0,
                u_dyn_min=0.05, enabled=True,
            ),
            0.866 * 0.8,
        )

    def test_kdyn_halves_limit(self) -> None:
        self.assertAlmostEqual(
            compute_dynamic_limit(
                u_lim=0.866, udc_meas=1.0, k_dyn=0.5,
                u_dyn_min=0.05, enabled=True,
            ),
            0.866 * 0.5,
        )

    def test_udc_surge_clamped_to_u_lim(self) -> None:
        # При всплеске ЗПТ предел не превышает способность ШИМ.
        self.assertAlmostEqual(
            compute_dynamic_limit(
                u_lim=0.866, udc_meas=1.3, k_dyn=1.0,
                u_dyn_min=0.05, enabled=True,
            ),
            0.866,
        )

    def test_deep_sag_clamped_to_floor(self) -> None:
        self.assertAlmostEqual(
            compute_dynamic_limit(
                u_lim=0.866, udc_meas=0.01, k_dyn=1.0,
                u_dyn_min=0.05, enabled=True,
            ),
            0.05,
        )

    def test_negative_udc_treated_as_zero_then_floor(self) -> None:
        self.assertAlmostEqual(
            compute_dynamic_limit(
                u_lim=0.866, udc_meas=-0.5, k_dyn=1.0,
                u_dyn_min=0.05, enabled=True,
            ),
            0.05,
        )

    def test_invalid_kdyn_falls_back_to_one(self) -> None:
        for bad in (0.0, -0.1, 2.5):
            self.assertAlmostEqual(
                compute_dynamic_limit(
                    u_lim=0.866, udc_meas=1.0, k_dyn=bad,
                    u_dyn_min=0.05, enabled=True,
                ),
                0.866,
                msg=f"k_dyn={bad} should fall back to 1.0",
            )

    def test_zero_u_lim_falls_back_to_unit(self) -> None:
        # Нулевой/непроинициализированный U_lim -> защита 1.0; при номинальном
        # Ud предел равен этой защите (1.0), а не 0.
        self.assertAlmostEqual(
            compute_dynamic_limit(
                u_lim=0.0, udc_meas=1.0, k_dyn=1.0,
                u_dyn_min=0.05, enabled=True,
            ),
            1.0,
        )


if __name__ == "__main__":
    unittest.main()
