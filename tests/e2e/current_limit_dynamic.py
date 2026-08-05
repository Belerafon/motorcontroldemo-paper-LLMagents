from __future__ import annotations

import json
import time
from argparse import ArgumentParser, Namespace, SUPPRESS
from contextlib import contextmanager
from pathlib import Path
from typing import Any, Iterator

from motor_debug_core.project import ProjectPaths
from motor_debug_core.values import FixedPoint

from .artifacts import write_json
from .common import (
    HardwareSession,
    firmware_metadata,
    resolve_output,
    snapshot_dict,
)


NAME = "current-limit-dynamic"
HELP = (
    "Verify the dynamic current-loop voltage limit tracks pwm.U_lim and "
    "optional Udc derating (CTRL_STOP only, software model)."
)

# Все per-unit величины в модуле - IQ24, базис 1.0 = номинал.
_PU = FixedPoint(q=24, scale=1.0, unit="pu")

# Безопасная полоса Udc (о.е.): сверху - с запасом до защиты по перенапряжению
# (~1.1 о.е.), снизу - с запасом до защиты по снижению (~0.42 о.е.).
_UDC_SAFE_MIN = 0.40
_UDC_SAFE_MAX = 1.00

OFFSET_EXPR = "adc.Udc_meas_offset"


def register(parser: ArgumentParser) -> None:
    parser.add_argument("--start-drive", action="store_true", help=SUPPRESS)
    parser.add_argument(
        "--formula-tol-pu",
        type=float,
        default=0.03,
        help="Допуск совпадения U_max с pwm.U_lim*Scale*Kudc, о.е.",
    )
    parser.add_argument(
        "--track-tol-pu",
        type=float,
        default=0.02,
        help="Допуск слежения пределов pid_id/pid_iq за U_max, о.е.",
    )
    parser.add_argument(
        "--deration-track-tol-pu",
        type=float,
        default=0.04,
        help="Допуск формулы при активной дерейции по Udc, о.е.",
    )
    parser.add_argument(
        "--target-udc-low-pu",
        type=float,
        default=0.5,
        help="Целевой Udc (о.е.) внутри зоны дерейции.",
    )
    parser.add_argument(
        "--target-udc-high-pu",
        type=float,
        default=0.9,
        help="Целевой Udc (о.е.) выше порога насыщения дерейции.",
    )
    parser.add_argument(
        "--settle-s",
        type=float,
        default=0.15,
        help="Пауза на сходимость фильтра ЗПТ после изменения смещения АЦП, с.",
    )
    parser.add_argument(
        "--short-settle-s",
        type=float,
        default=0.06,
        help="Пауза после изменения Scale/Enable/дерейции, с.",
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("artifacts/e2e/current-limit-dynamic"),
    )


class LimitProbe:
    """Одномоментный снимок всех релевантных переменных модуля и регуляторов."""

    def __init__(self, session: HardwareSession):
        target = session.target
        self.u_lim = _PU.decode(target.read_u32("pwm.U_lim"))
        self.udc_meas = _PU.decode(target.read_u32("adc.Udc_meas"))
        self.udc_filt = _PU.decode(target.read_u32("cur_lim.Udc"))
        self.scale = _PU.decode(target.read_u32("cur_lim.Scale"))
        self.cap = _PU.decode(target.read_u32("cur_lim.U_max_cap"))
        self.enable = target.read_u16("cur_lim.Enable")
        self.derate_en = target.read_u16("cur_lim.UdcDerateEnable")
        self.derate_full = _PU.decode(target.read_u32("cur_lim.UdcDerateFull"))
        self.derate_zero = _PU.decode(target.read_u32("cur_lim.UdcDerateZero"))
        self.u_max = _PU.decode(target.read_u32("cur_lim.U_max"))
        self.debug1 = _PU.decode(target.read_u32("Debug1"))
        self.iq_max = _PU.decode(target.read_u32("pid_iq.pid_out_max"))
        self.iq_min = _PU.decode(target.read_u32("pid_iq.pid_out_min"))
        self.id_max = _PU.decode(target.read_u32("pid_id.pid_out_max"))
        self.id_min = _PU.decode(target.read_u32("pid_id.pid_out_min"))

    def kudc(self) -> float:
        if not self.derate_en:
            return 1.0
        if self.udc_filt <= self.derate_zero:
            return 0.0
        if self.udc_filt >= self.derate_full:
            return 1.0
        span = self.derate_full - self.derate_zero
        return (self.udc_filt - self.derate_zero) / span if span else 0.0

    def expected_u_max(self) -> float:
        raw = self.u_lim * self.scale * self.kudc()
        return max(0.0, min(raw, self.cap))

    def as_dict(self) -> dict[str, float]:
        return {
            "pwm.U_lim_pu": self.u_lim,
            "adc.Udc_meas_pu": self.udc_meas,
            "cur_lim.Udc_pu": self.udc_filt,
            "cur_lim.Scale_pu": self.scale,
            "cur_lim.U_max_cap_pu": self.cap,
            "cur_lim.Enable": self.enable,
            "cur_lim.UdcDerateEnable": self.derate_en,
            "cur_lim.UdcDerateFull_pu": self.derate_full,
            "cur_lim.UdcDerateZero_pu": self.derate_zero,
            "cur_lim.Kudc": self.kudc(),
            "cur_lim.U_max_pu": self.u_max,
            "Debug1_pu": self.debug1,
            "pid_iq.pid_out_max_pu": self.iq_max,
            "pid_iq.pid_out_min_pu": self.iq_min,
            "pid_id.pid_out_max_pu": self.id_max,
            "pid_id.pid_out_min_pu": self.id_min,
        }


def _wait(seconds: float) -> None:
    time.sleep(max(0.0, seconds))


def _read_s16(session: HardwareSession, expr: str) -> int:
    raw = session.target.read_u16(expr)
    return raw - 0x10000 if raw >= 0x8000 else raw


def _write_s16(session: HardwareSession, expr: str, value: int) -> None:
    session.target.write_u16(expr, value & 0xFFFF)


def _read_udc(session: HardwareSession) -> float:
    return _PU.decode(session.target.read_u32("cur_lim.Udc"))


def _estimate_offset_slope(
    session: HardwareSession,
    offset0: int,
    udc0: float,
    *,
    settle_s: float,
) -> float:
    """Безопасно оценивает крутизну dUdc/d(offset) малыми шагами.

    Шаги удваиваются от 1, пока сдвиг Udc не станет измеримо значимым; на каждом
    шаге контролируется, что Udc остаётся в безопасной полосе (чтобы не вызвать
    защиту по напряжению). Смещение восстанавливается перед возвратом.
    """
    delta = 1
    while delta <= 256:
        _write_s16(session, OFFSET_EXPR, offset0 + delta)
        _wait(settle_s)
        udc = _read_udc(session)
        if udc > _UDC_SAFE_MAX or udc < _UDC_SAFE_MIN:
            _write_s16(session, OFFSET_EXPR, offset0)
            _wait(settle_s)
            raise RuntimeError(
                f"Udc left safe band during slope probe: {udc:g} pu at "
                f"offset delta {delta}."
            )
        shift = udc - udc0
        if abs(shift) >= 0.02:
            _write_s16(session, OFFSET_EXPR, offset0)
            _wait(settle_s)
            return shift / delta
        delta *= 2
    _write_s16(session, OFFSET_EXPR, offset0)
    _wait(settle_s)
    raise RuntimeError("Udc is insensitive to adc.Udc_meas_offset.")


@contextmanager
def _udc_at(
    session: HardwareSession,
    target_udc_pu: float,
    *,
    settle_s: float,
) -> Iterator[dict[str, float]]:
    """Временно привести отфильтрованное Udc к целевому уровню через смещение АЦП.

    Безопасно: целевой уровень ограничивается безопасной полосой, крутизна
    оценивается малыми шагами, в конце смещение гарантированно восстанавливается.
    """
    target_udc_pu = max(
        _UDC_SAFE_MIN + 0.02, min(_UDC_SAFE_MAX - 0.02, target_udc_pu)
    )
    offset0 = _read_s16(session, OFFSET_EXPR)
    udc0 = _read_udc(session)
    slope = _estimate_offset_slope(session, offset0, udc0, settle_s=settle_s)
    needed_delta = int(round((target_udc_pu - udc0) / slope))
    new_offset = offset0 + needed_delta
    try:
        _write_s16(session, OFFSET_EXPR, new_offset)
        _wait(settle_s)
        achieved = _read_udc(session)
        if achieved > _UDC_SAFE_MAX or achieved < _UDC_SAFE_MIN:
            raise RuntimeError(
                f"Udc left safe band after offset step: {achieved:g} pu."
            )
        yield {
            "offset0_raw": offset0,
            "udc0_pu": udc0,
            "slope_pu_per_offset": slope,
            "target_udc_pu": target_udc_pu,
            "achieved_udc_pu": achieved,
            "applied_offset_raw": new_offset,
        }
    finally:
        session.drive.stop()
        _wait(settle_s)
        _write_s16(session, OFFSET_EXPR, offset0)
        _wait(settle_s)
        restored = _read_s16(session, OFFSET_EXPR)
        if restored != offset0:
            raise RuntimeError(
                f"RAM restoration failed for {OFFSET_EXPR}: "
                f"{restored} != {offset0}."
            )


@contextmanager
def _scoped_u16(
    session: HardwareSession, expression: str, value: int
) -> Iterator[int]:
    target = session.target
    original = target.read_u16(expression)
    target.write_u16(expression, value)
    try:
        yield original
    finally:
        session.drive.stop()
        time.sleep(0.05)
        target.write_u16(expression, original)
        restored = target.read_u16(expression)
        if restored != original:
            raise RuntimeError(
                f"RAM restoration failed for {expression}: "
                f"0x{restored:04X} != 0x{original:04X}."
            )


@contextmanager
def _scoped_iq24(
    session: HardwareSession, expression: str, value_pu: float
) -> Iterator[int]:
    target = session.target
    original = target.read_u32(expression)
    target.write_u32(expression, _PU.encode(value_pu))
    try:
        yield original
    finally:
        session.drive.stop()
        time.sleep(0.05)
        target.write_u32(expression, original)
        restored = target.read_u32(expression)
        if restored != original:
            raise RuntimeError(
                f"RAM restoration failed for {expression}: "
                f"0x{restored:08X} != 0x{original:08X}."
            )


def _check_protection_healthy(snapshot) -> None:
    if snapshot.protection_state != 1 or snapshot.fault1 or snapshot.fault2:
        raise RuntimeError(
            f"Protection not healthy during test: {snapshot.describe()}"
        )


def run(args: Namespace, paths: ProjectPaths) -> None:
    for value in (args.formula_tol_pu, args.track_tol_pu, args.deration_track_tol_pu):
        if value < 0:
            raise ValueError("Tolerances must not be negative.")
    if args.settle_s <= 0 or args.short_settle_s <= 0:
        raise ValueError("Settle times must be positive.")

    output = resolve_output(paths.root, args.output)
    output.mkdir(parents=True, exist_ok=True)

    observations: list[dict[str, Any]] = []
    criteria: dict[str, bool] = {}
    measurements: dict[str, Any] = {}

    with HardwareSession(paths) as session:
        initial = session.preflight()
        initial_offset = _read_s16(session, OFFSET_EXPR)
        initial_scale = _PU.decode(session.target.read_u32("cur_lim.Scale"))

        # --- Базовый снимок: динамический предел = pwm.U_lim*Scale ---
        base = LimitProbe(session)
        _check_protection_healthy(session.drive.snapshot())
        observations.append({"phase": "baseline", **base.as_dict()})

        criteria["enable_default_on"] = base.enable == 1
        criteria["derate_off_by_default"] = base.derate_en == 0
        criteria["u_max_within_cap"] = 0.0 < base.u_max <= base.cap + 1e-9
        criteria["formula_matches"] = (
            abs(base.u_max - base.expected_u_max()) <= args.formula_tol_pu
        )
        criteria["debug1_matches_u_max"] = (
            abs(base.debug1 - base.u_max) <= args.track_tol_pu
        )
        criteria["iq_limits_track_u_max"] = (
            abs(base.iq_max - base.u_max) <= args.track_tol_pu
            and abs(base.iq_min + base.u_max) <= args.track_tol_pu
        )
        criteria["id_limits_track_u_max"] = (
            abs(base.id_max - base.u_max) <= args.track_tol_pu
            and abs(base.id_min + base.u_max) <= args.track_tol_pu
        )
        measurements["baseline"] = base.as_dict()
        measurements["baseline_expected_u_max_pu"] = base.expected_u_max()

        # --- Коэффициент Scale ---
        with _scoped_iq24(session, "cur_lim.Scale", 0.5):
            _wait(args.short_settle_s)
            _check_protection_healthy(session.drive.snapshot())
            scaled = LimitProbe(session)
            observations.append({"phase": "scale_half", **scaled.as_dict()})
            measurements["scale_half"] = {
                "scale_pu": scaled.scale,
                "u_max_pu": scaled.u_max,
                "expected_u_max_pu": scaled.expected_u_max(),
                "formula_error_pu": abs(scaled.u_max - scaled.expected_u_max()),
            }
            criteria["scale_halves_u_max"] = (
                abs(scaled.scale - 0.5) <= 1e-6
                and abs(scaled.u_max - scaled.expected_u_max())
                <= args.formula_tol_pu
                and abs(scaled.iq_max - scaled.u_max) <= args.track_tol_pu
            )

        # --- Дерейция по напряжению ЗПТ (опциональная зависимость от Udc) ---
        # Включаем дерейцию и проверяем, что U_max снижается пропорционально Udc
        # внутри зоны [UdcDerateZero; UdcDerateFull] и насыщается вне её.
        with _scoped_u16(session, "cur_lim.UdcDerateEnable", 1):
            _wait(args.short_settle_s)
            _check_protection_healthy(session.drive.snapshot())
            derate_base = LimitProbe(session)
            observations.append(
                {"phase": "deration_on_baseline", **derate_base.as_dict()}
            )

            # Точка внутри зоны дерейции (нижняя часть рампы).
            with _udc_at(
                session, args.target_udc_low_pu, settle_s=args.settle_s
            ) as ctx_low:
                _check_protection_healthy(session.drive.snapshot())
                derate_low = LimitProbe(session)
                observations.append(
                    {"phase": "deration_on_udc_low", **derate_low.as_dict()}
                )

            # Точка выше порога насыщения: Kudc = 1, дерейции нет.
            with _udc_at(
                session, args.target_udc_high_pu, settle_s=args.settle_s
            ) as ctx_high:
                _check_protection_healthy(session.drive.snapshot())
                derate_high = LimitProbe(session)
                observations.append(
                    {"phase": "deration_on_udc_high", **derate_high.as_dict()}
                )

            measurements["deration"] = {
                "udc_derate_full_pu": derate_base.derate_full,
                "udc_derate_zero_pu": derate_base.derate_zero,
                "baseline": {
                    "udc_pu": derate_base.udc_filt,
                    "kudc": derate_base.kudc(),
                    "u_max_pu": derate_base.u_max,
                    "expected_u_max_pu": derate_base.expected_u_max(),
                },
                "udc_low": {
                    "shift_context": ctx_low,
                    "udc_pu": derate_low.udc_filt,
                    "kudc": derate_low.kudc(),
                    "u_max_pu": derate_low.u_max,
                    "expected_u_max_pu": derate_low.expected_u_max(),
                },
                "udc_high": {
                    "shift_context": ctx_high,
                    "udc_pu": derate_high.udc_filt,
                    "kudc": derate_high.kudc(),
                    "u_max_pu": derate_high.u_max,
                    "expected_u_max_pu": derate_high.expected_u_max(),
                },
            }
            # U_max ниже в нижней точке, чем в верхней, и выше порога насыщения
            # возвращается к полному U_lim*Scale.
            criteria["deration_reduces_u_max_at_low_udc"] = (
                derate_low.udc_filt < derate_high.udc_filt
                and derate_low.u_max < derate_high.u_max
                and derate_low.kudc() < derate_high.kudc()
            )
            criteria["deration_formula_matches_low"] = (
                abs(derate_low.u_max - derate_low.expected_u_max())
                <= args.deration_track_tol_pu
                and abs(derate_low.iq_max - derate_low.u_max)
                <= args.track_tol_pu
            )
            criteria["deration_saturates_at_high_udc"] = (
                derate_high.kudc() >= 1.0 - 1e-6
                and abs(derate_high.u_max - derate_high.u_lim * derate_high.scale)
                <= args.formula_tol_pu
            )

        # --- A/B: выключение динамического предела (cur_lim.Enable = 0) ---
        # При Enable=0 пределы регуляторов "замораживаются" и не следят за U_max
        # при изменении Udc. Чтобы заморозка была наблюдаема, внутри Enable=0
        # включаем дерейцию по Udc: тогда U_max меняется с Udc, а pid_iq - нет.
        with _scoped_u16(session, "cur_lim.Enable", 0):
            with _scoped_u16(session, "cur_lim.UdcDerateEnable", 1):
                _wait(args.short_settle_s)
                legacy_base = LimitProbe(session)
                observations.append(
                    {"phase": "legacy_enable_off", **legacy_base.as_dict()}
                )
                legacy_iq_max_frozen = legacy_base.iq_max

                with _udc_at(
                    session, args.target_udc_low_pu, settle_s=args.settle_s
                ) as ctx_legacy:
                    _check_protection_healthy(session.drive.snapshot())
                    legacy_low = LimitProbe(session)
                    observations.append(
                        {"phase": "legacy_enable_off_udc_step",
                         **legacy_low.as_dict()}
                    )
                    measurements["legacy_ab"] = {
                        "shift_context": ctx_legacy,
                        "enable_off_iq_max_frozen_pu": legacy_iq_max_frozen,
                        "enable_off_udc_step_iq_max_pu": legacy_low.iq_max,
                        "enable_off_udc_step_u_max_pu": legacy_low.u_max,
                        "enable_off_udc_step_udc_pu": legacy_low.udc_filt,
                    }
                    criteria["legacy_freezes_pid_limits"] = (
                        legacy_low.udc_filt != legacy_base.udc_filt
                        and abs(legacy_low.iq_max - legacy_iq_max_frozen)
                        <= args.track_tol_pu
                        and abs(legacy_low.iq_max - legacy_low.u_max)
                        > args.track_tol_pu
                    )

        # Возврат Enable=1 - предел снова "живой" и следит за U_max.
        _wait(args.short_settle_s)
        final = LimitProbe(session)
        observations.append({"phase": "final", **final.as_dict()})
        criteria["re_enable_tracks_again"] = (
            final.enable == 1
            and abs(final.iq_max - final.u_max) <= args.track_tol_pu
        )

        restored = session.drive.snapshot()
        restored_offset = _read_s16(session, OFFSET_EXPR)
        restored_scale = _PU.decode(session.target.read_u32("cur_lim.Scale"))
        criteria["temporary_settings_restored"] = (
            restored.control_state == 0
            and restored_offset == initial_offset
            and abs(restored_scale - initial_scale) <= 1e-6
            and final.enable == 1
            and final.derate_en == 0
        )
        snapshots = {
            "initial": snapshot_dict(initial),
            "restored": snapshot_dict(restored),
        }

    measurements["observations"] = observations
    report = {
        "test": NAME,
        "firmware": firmware_metadata(paths),
        "setup": {
            "formula_tol_pu": args.formula_tol_pu,
            "track_tol_pu": args.track_tol_pu,
            "deration_track_tol_pu": args.deration_track_tol_pu,
            "target_udc_low_pu": args.target_udc_low_pu,
            "target_udc_high_pu": args.target_udc_high_pu,
            "settle_s": args.settle_s,
            "short_settle_s": args.short_settle_s,
        },
        "snapshots": snapshots,
        "measurements": measurements,
        "criteria": criteria,
        "passed": all(criteria.values()),
    }
    json_path = output / "report.json"
    write_json(json_path, report)
    print(json.dumps({"criteria": criteria, "passed": report["passed"]},
                     ensure_ascii=False, indent=2))
    print(f"JSON: {json_path}")
    if not report["passed"]:
        failed = [name for name, ok in criteria.items() if not ok]
        raise RuntimeError(
            "Dynamic current-limit acceptance criteria failed: "
            + ", ".join(failed)
        )
