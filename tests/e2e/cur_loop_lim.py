from __future__ import annotations

import json
import time
from argparse import ArgumentParser, Namespace
from pathlib import Path

from motor_debug_core.project import ProjectPaths
from motor_debug_core.values import FixedPoint

from .artifacts import write_json
from .common import (
    HardwareSession,
    firmware_metadata,
    resolve_output,
    snapshot_dict,
)
from .cur_loop_lim_model import STATIC_LIMIT_PU, compute_dynamic_limit


NAME = "cur-loop-lim"
HELP = (
    "Verify dynamic current-controller voltage limits track pwm.U_lim, "
    "adc.Udc_meas and the DebugW1/DebugW2 knobs without running the motor."
)

# IQ24 без масштаба словаря — для переменных модуля curLoopLim, которых ещё
# нет в cood.xml (curLoopLim.u_dyn, curLoopLim.u_dyn_min).
_IQ24 = FixedPoint(q=24, scale=1.0)


def register(parser: ArgumentParser) -> None:
    parser.add_argument(
        "--settling-s", type=float, default=0.05,
        help="Pause after each RAM write so the 10 kHz loop picks it up.",
    )
    parser.add_argument(
        "--tolerance-pu", type=float, default=0.02,
        help="Absolute tolerance (pu) for expected-vs-measured limit checks.",
    )
    parser.add_argument(
        "--kdyn-probe", type=float, default=0.5,
        help="Secondary k_dyn value used to verify proportional response.",
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("artifacts/e2e/cur-loop-lim"),
    )


def _read_iq24(session: HardwareSession, expression: str) -> float:
    return _IQ24.decode(session.target.read_u32(expression))


def _write_iq24(session: HardwareSession, expression: str, value_pu: float) -> None:
    session.target.write_u32(expression, _IQ24.encode(value_pu))


def _snapshot_limits(session: HardwareSession) -> dict[str, float]:
    return {
        "id_max": _read_iq24(session, "pid_id.pid_out_max"),
        "id_min": _read_iq24(session, "pid_id.pid_out_min"),
        "iq_max": _read_iq24(session, "pid_iq.pid_out_max"),
        "iq_min": _read_iq24(session, "pid_iq.pid_out_min"),
        "u_dyn": _read_iq24(session, "curLoopLim.u_dyn"),
        "debug1": _read_iq24(session, "Debug1"),
    }


def _check_symmetric(
    limits: dict[str, float], tol: float, criteria: dict[str, bool]
) -> None:
    criteria["limits_symmetric_id"] = abs(limits["id_max"] + limits["id_min"]) <= tol
    criteria["limits_symmetric_iq"] = abs(limits["iq_max"] + limits["iq_min"]) <= tol
    criteria["limits_id_eq_iq"] = (
        abs(limits["id_max"] - limits["iq_max"]) <= tol
    )


def run(args: Namespace, paths: ProjectPaths) -> None:
    if args.settling_s <= 0:
        raise ValueError("--settling-s must be positive.")
    if args.tolerance_pu <= 0:
        raise ValueError("--tolerance-pu must be positive.")
    if not 0.0 < args.kdyn_probe <= 2.0:
        raise ValueError("--kdyn-probe must be in (0; 2.0].")
    output = resolve_output(paths.root, args.output)
    output.mkdir(parents=True, exist_ok=True)

    criteria: dict[str, bool] = {}
    measurements: list[dict[str, object]] = []
    initial_debug_w1 = 0
    initial_debug_w2 = 0
    restore_verified = False

    with HardwareSession(paths) as session:
        initial = session.preflight()
        initial_debug_w1 = session.target.read_u32("DebugW1")
        initial_debug_w2 = session.target.read_u32("DebugW2")

        u_lim = session.read_engineering("pwm.U_lim")
        # adc.Udc_meas в прошивке — сырой IQ24 в о.е. от Udc_nom (именно его
        # использует CurLoopLim_Calc). read_engineering тут не подходит:
        # словарь масштабирует его в сторонние единицы (ScaleNum с possible_value
        # около 36). Берём сырой IQ24, чтобы ожидаемое значение совпадало с
        # тем, что считает прошивка.
        udc_meas = _read_iq24(session, "adc.Udc_meas")
        udc_meas_eng = session.read_engineering("adc.Udc_meas")
        u_dyn_min = _read_iq24(session, "curLoopLim.u_dyn_min")
        hardware = session.hardware()
        print(
            f"pwm.U_lim={u_lim:.4f} pu, adc.Udc_meas={udc_meas:.4f} pu "
            f"(словарь={udc_meas_eng:.3f}), u_dyn_min={u_dyn_min:.4f} pu"
        )

        def record(label: str, **extra: object) -> dict[str, object]:
            limits = _snapshot_limits(session)
            entry = {"label": label, "limits": limits, **extra}
            measurements.append(entry)
            print(
                f"[{label}] id_max={limits['id_max']:+.4f} "
                f"u_dyn={limits['u_dyn']:+.4f} debug1={limits['debug1']:+.4f}"
            )
            return entry

        try:
            # 0. Boot-дефолт: DebugW1 не трогаем. Дефолт словаря (~0.001 о.е.,
            # raw 16777) должен трактоваться как ВЫКЛ благодаря порогу 0.5 —
            # пределы обязаны быть статикой 1.0. Без порога «любое ненулевое»
            # здесь ошибочно включилась бы динамика и занизила предел.
            boot = record("boot_default")
            criteria["boot_default_id_max_is_static"] = (
                abs(boot["limits"]["id_max"] - STATIC_LIMIT_PU)
                <= args.tolerance_pu
            )
            criteria["boot_default_iq_max_is_static"] = (
                abs(boot["limits"]["iq_max"] - STATIC_LIMIT_PU)
                <= args.tolerance_pu
            )

            # 1. По умолчанию (DebugW1 == 0) — статика.
            _write_iq24(session, "DebugW1", 0.0)
            _write_iq24(session, "DebugW2", 1.0)
            time.sleep(args.settling_s)
            off = record("disabled")
            criteria["disabled_id_max_is_static"] = (
                abs(off["limits"]["id_max"] - STATIC_LIMIT_PU) <= args.tolerance_pu
            )
            criteria["disabled_iq_max_is_static"] = (
                abs(off["limits"]["iq_max"] - STATIC_LIMIT_PU) <= args.tolerance_pu
            )
            criteria["disabled_u_dyn_reports_static"] = (
                abs(off["limits"]["u_dyn"] - STATIC_LIMIT_PU) <= args.tolerance_pu
            )
            _check_symmetric(off["limits"], args.tolerance_pu, criteria)

            # 2. Включение динамики, k_dyn = 1.0.
            _write_iq24(session, "DebugW1", 1.0)
            _write_iq24(session, "DebugW2", 1.0)
            time.sleep(args.settling_s)
            expected = compute_dynamic_limit(
                u_lim=u_lim, udc_meas=udc_meas, k_dyn=1.0,
                u_dyn_min=u_dyn_min, enabled=True,
            )
            on = record("enabled_kdyn_1", expected_pu=expected)
            criteria["enabled_u_dyn_matches_formula"] = (
                abs(on["limits"]["u_dyn"] - expected) <= args.tolerance_pu
            )
            criteria["enabled_id_max_matches_formula"] = (
                abs(on["limits"]["id_max"] - expected) <= args.tolerance_pu
            )
            criteria["enabled_iq_max_matches_formula"] = (
                abs(on["limits"]["iq_max"] - expected) <= args.tolerance_pu
            )
            criteria["enabled_debug1_matches_u_dyn"] = (
                abs(on["limits"]["debug1"] - on["limits"]["u_dyn"])
                <= args.tolerance_pu
            )
            _check_symmetric(on["limits"], args.tolerance_pu, criteria)

            # 3. Половинная чувствительность — предел уменьшается пропорционально.
            _write_iq24(session, "DebugW2", args.kdyn_probe)
            time.sleep(args.settling_s)
            expected_probe = compute_dynamic_limit(
                u_lim=u_lim, udc_meas=udc_meas, k_dyn=args.kdyn_probe,
                u_dyn_min=u_dyn_min, enabled=True,
            )
            probe = record(
                "enabled_kdyn_probe",
                expected_pu=expected_probe,
                kdyn=args.kdyn_probe,
            )
            criteria["kdyn_probe_u_dyn_matches_formula"] = (
                abs(probe["limits"]["u_dyn"] - expected_probe)
                <= args.tolerance_pu
            )
            # Если ожидаемое значение не упёрлось в u_dyn_min, предел должен
            # быть ниже значения при k_dyn=1.
            if expected_probe > u_dyn_min + args.tolerance_pu:
                criteria["kdyn_probe_lower_than_full"] = (
                    probe["limits"]["u_dyn"] < on["limits"]["u_dyn"]
                )

            # 4. Выключение — точный возврат к статике.
            _write_iq24(session, "DebugW1", 0.0)
            time.sleep(args.settling_s)
            revert = record("reverted")
            criteria["revert_id_max_is_static"] = (
                abs(revert["limits"]["id_max"] - STATIC_LIMIT_PU)
                <= args.tolerance_pu
            )
            criteria["revert_iq_max_is_static"] = (
                abs(revert["limits"]["iq_max"] - STATIC_LIMIT_PU)
                <= args.tolerance_pu
            )
        finally:
            # Возврат исходных значений ручек и гарантированный stop.
            session.drive.stop()
            time.sleep(args.settling_s)
            session.target.write_u32("DebugW1", initial_debug_w1)
            session.target.write_u32("DebugW2", initial_debug_w2)
            w1_restored = session.target.read_u32("DebugW1") == initial_debug_w1
            w2_restored = session.target.read_u32("DebugW2") == initial_debug_w2
            restore_verified = w1_restored and w2_restored

        restored = session.drive.snapshot()

    criteria["debug_knobs_restored"] = restore_verified

    report = {
        "test": NAME,
        "firmware": firmware_metadata(paths),
        "hardware": hardware,
        "setup": {
            "pwm_u_lim_pu": u_lim,
            "adc_udc_meas_pu": udc_meas,
            "adc_udc_meas_dictionary": udc_meas_eng,
            "u_dyn_min_pu": u_dyn_min,
            "static_limit_pu": STATIC_LIMIT_PU,
            "tolerance_pu": args.tolerance_pu,
            "kdyn_probe": args.kdyn_probe,
            "settling_s": args.settling_s,
            "initial_debug_w1_raw": initial_debug_w1,
            "initial_debug_w2_raw": initial_debug_w2,
        },
        "measurements": measurements,
        "snapshots": {
            "initial": snapshot_dict(initial),
            "restored": snapshot_dict(restored),
        },
        "metrics": {"criteria": criteria, "passed": all(criteria.values())},
    }
    json_path = output / "report.json"
    write_json(json_path, report)
    print(json.dumps({"criteria": criteria, "passed": all(criteria.values())},
                     ensure_ascii=False, indent=2))
    print(f"JSON: {json_path}")
    if not all(criteria.values()):
        raise RuntimeError("Dynamic current-loop limit acceptance criteria failed.")
