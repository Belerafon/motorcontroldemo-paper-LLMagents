from __future__ import annotations

"""Pure-Python модель динамики предела напряжения токовых регуляторов.

Файл намеренно не зависит от motor_debug_core и JTAG-транспорта, чтобы его
можно было проверять unit-тестами без контроллера (см. tests/unit).
Логика повторяет CurLoopLim_Calc из Vsrc/V_CurLoopLim.c.
"""

# Скомпилированный статический предел из PIDREG3_DEFAULTS (старое поведение).
STATIC_LIMIT_PU = 1.0


def compute_dynamic_limit(
    *,
    u_lim: float,
    udc_meas: float,
    k_dyn: float,
    u_dyn_min: float,
    enabled: bool,
) -> float:
    """Возвращает предел выхода токовых регуляторов в о.е. от U_nom.

    При enabled=False это статика 1.0 (PIDREG3_DEFAULTS); при enabled=True —
    U_lim * Udc * k_dyn с защитным ограничением [u_dyn_min; U_lim].
    """
    if not enabled:
        return 1.0
    if k_dyn <= 0.0 or k_dyn > 2.0:
        k_dyn = 1.0
    if udc_meas < 0.0:
        udc_meas = 0.0
    if u_lim <= 0.0:
        u_lim = 1.0
    u = u_lim * udc_meas * k_dyn
    if u > u_lim:
        u = u_lim
    if u < u_dyn_min:
        u = u_dyn_min
    return u
