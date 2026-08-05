from __future__ import annotations

import time
from argparse import ArgumentParser, Namespace
from pathlib import Path

from motor_debug_core.project import ProjectPaths

from .artifacts import write_json
from .common import (
    HardwareSession,
    firmware_metadata,
    resolve_output,
    snapshot_dict,
)
from .metrics import delivered_voltage_ceiling, expected_current_reg_limit


NAME = "current-limit-dyn"
HELP = (
    "Verify the dynamic current-regulator voltage limit tracks pwm.U_lim, "
    "adc.Udc_meas and the Ud-compensation setting."
)

IQ24 = 1 << 24

# Symbols read/written as raw signed IQ24 per-unit words.
LIMIT_SYMS = (
    "pid_id.pid_out_max",
    "pid_id.pid_out_min",
    "pid_iq.pid_out_max",
    "pid_iq.pid_out_min",
)


def _signed_iq(raw: int) -> float:
    if raw >= 0x80000000:
        raw -= 0x100000000
    return raw / float(IQ24)


def _iq_raw(value_pu: float) -> int:
    raw = round(value_pu * IQ24)
    if not -(1 << 31) <= raw < (1 << 31):
        raise ValueError(f"Per-unit value out of IQ24 range: {value_pu}")
    return raw & 0xFFFFFFFF


def register(parser: ArgumentParser) -> None:
    parser.add_argument("--settle-s", type=float, default=0.08)
    parser.add_argument("--abs-tol-pu", type=float, default=0.01)
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("artifacts/e2e/current-limit-dyn"),
    )


def _read_pu(session: HardwareSession, sym: str) -> float:
    return _signed_iq(session.target.read_u32(sym))


def _snapshot_inputs(session: HardwareSession) -> dict[str, float | int]:
    u_lim = _read_pu(session, "pwm.U_lim")
    udc = _read_pu(session, "adc.Udc_meas")
    udcomp_k = _read_pu(session, "pwm.UdCompK")
    udcomp_ena = session.target.read_u32("pwm.UdCompEnable") & 1
    return {
        "u_lim_pu": u_lim,
        "udc_meas_pu": udc,
        "udcomp_k": udcomp_k,
        "udcomp_enable": int(udcomp_ena),
    }


def _check_limits(
    session: HardwareSession,
    inputs: dict[str, float | int],
    *,
    abs_tol_pu: float,
    criteria: dict[str, bool],
    measurements: list[dict[str, object]],
    label: str,
) -> None:
    expected = expected_current_reg_limit(**inputs)
    debug1 = _read_pu(session, "Debug1")
    debug2 = _read_pu(session, "Debug2")
    expected_delivered = delivered_voltage_ceiling(
        inputs["u_lim_pu"], inputs["udc_meas_pu"]
    )
    regs = {sym: _read_pu(session, sym) for sym in LIMIT_SYMS}
    time.sleep(0.005)
    regs2 = {sym: _read_pu(session, sym) for sym in LIMIT_SYMS}
    measurements.append(
        {
            "label": label,
            "inputs": inputs,
            "expected_max_pu": expected,
            "expected_delivered_ceiling_pu": expected_delivered,
            "debug1_pu": debug1,
            "debug2_delivered_pu": debug2,
            "limits_pu": regs,
        }
    )
    criteria[f"{label}:Debug1_matches_formula"] = abs(debug1 - expected) <= abs_tol_pu
    criteria[f"{label}:Debug2_matches_delivered_ceiling"] = (
        abs(debug2 - expected_delivered) <= abs_tol_pu
    )
    criteria[f"{label}:id_max_matches"] = abs(regs["pid_id.pid_out_max"] - expected) <= abs_tol_pu
    criteria[f"{label}:iq_max_matches"] = abs(regs["pid_iq.pid_out_max"] - expected) <= abs_tol_pu
    criteria[f"{label}:id_min_is_negmax"] = abs(regs["pid_id.pid_out_min"] + expected) <= abs_tol_pu
    criteria[f"{label}:iq_min_is_negmax"] = abs(regs["pid_iq.pid_out_min"] + expected) <= abs_tol_pu
    criteria[f"{label}:limits_settled"] = all(
        abs(regs[sym] - regs2[sym]) <= abs_tol_pu for sym in LIMIT_SYMS
    )


def run(args: Namespace, paths: ProjectPaths) -> None:
    if args.settle_s <= 0:
        raise ValueError("--settle-s must be positive.")
    if args.abs_tol_pu <= 0:
        raise ValueError("--abs-tol-pu must be positive.")
    output = resolve_output(paths.root, args.output)
    output.mkdir(parents=True, exist_ok=True)

    with HardwareSession(paths) as session:
        initial = session.preflight()
        hardware = session.hardware()

        orig_ulim = session.target.read_u32("pwm.U_lim")
        orig_udcomp_ena = session.target.read_u32("pwm.UdCompEnable")
        orig_debugw3 = session.target.read_u32("DebugW3")

        criteria: dict[str, bool] = {}
        measurements: list[dict[str, object]] = []

        try:
            # Dynamic mode is the default (DebugW3 == 0).
            session.target.write_u32("DebugW3", 0)

            # --- Branch 1: Ud-compensation enabled (default), sweep U_lim. ---
            session.target.write_u32("pwm.UdCompEnable", 1)
            ulim_sweep = (0.5, 0.866, 1.0)
            for u_lim_pu in ulim_sweep:
                session.target.write_u32("pwm.U_lim", _iq_raw(u_lim_pu))
                time.sleep(args.settle_s)
                inputs = _snapshot_inputs(session)
                _check_limits(
                    session,
                    inputs,
                    abs_tol_pu=args.abs_tol_pu,
                    criteria=criteria,
                    measurements=measurements,
                    label=f"udcomp_on_ulim_{u_lim_pu:g}",
                )

            # --- Branch 2: Ud-compensation disabled, limit must equal U_lim. ---
            session.target.write_u32("pwm.UdCompEnable", 0)
            for u_lim_pu in (0.6, 0.866):
                session.target.write_u32("pwm.U_lim", _iq_raw(u_lim_pu))
                time.sleep(args.settle_s)
                inputs = _snapshot_inputs(session)
                _check_limits(
                    session,
                    inputs,
                    abs_tol_pu=args.abs_tol_pu,
                    criteria=criteria,
                    measurements=measurements,
                    label=f"udcomp_off_ulim_{u_lim_pu:g}",
                )

            # --- A/B switch: legacy mode restores the compiled +-1.0 ceiling. ---
            session.target.write_u32("pwm.U_lim", orig_ulim)
            session.target.write_u32("pwm.UdCompEnable", 1)
            session.target.write_u32("DebugW3", _iq_raw(1.0))
            time.sleep(args.settle_s)
            legacy_id_max = _read_pu(session, "pid_id.pid_out_max")
            legacy_iq_max = _read_pu(session, "pid_iq.pid_out_max")
            criteria["legacy:id_max_is_unity"] = abs(legacy_id_max - 1.0) <= args.abs_tol_pu
            criteria["legacy:iq_max_is_unity"] = abs(legacy_iq_max - 1.0) <= args.abs_tol_pu
            measurements.append(
                {
                    "label": "legacy",
                    "id_max_pu": legacy_id_max,
                    "iq_max_pu": legacy_iq_max,
                }
            )

            # In legacy steady-state the dictionary/RAM value is respected:
            # a manual write must persist across cycles.
            session.target.write_u32("pid_id.pid_out_max", _iq_raw(0.9))
            time.sleep(args.settle_s)
            legacy_custom = _read_pu(session, "pid_id.pid_out_max")
            criteria["legacy:respects_ram_edit"] = (
                abs(legacy_custom - 0.9) <= args.abs_tol_pu
            )

            # --- Back to dynamic: limit resumes tracking. ---
            session.target.write_u32("DebugW3", 0)
            time.sleep(args.settle_s)
            inputs = _snapshot_inputs(session)
            _check_limits(
                session,
                inputs,
                abs_tol_pu=args.abs_tol_pu,
                criteria=criteria,
                measurements=measurements,
                label="dynamic_restored",
            )

            # Safety: no fault must be raised by any of the static reconfigures.
            final = session.drive.snapshot()
        finally:
            session.target.write_u32("DebugW3", orig_debugw3)
            session.drive.stop()
            time.sleep(0.1)
            session.target.write_u32("pwm.U_lim", orig_ulim)
            session.target.write_u32("pwm.UdCompEnable", orig_udcomp_ena)

        restored_ulim = session.target.read_u32("pwm.U_lim")
        restored_udcomp_ena = session.target.read_u32("pwm.UdCompEnable")
        restored_debugw3 = session.target.read_u32("DebugW3")

        criteria["no_fault_raised"] = (
            final.protection_state == 1 and not final.fault1 and not final.fault2
        )
        criteria["remained_ctrl_stop"] = final.control_state == 0
        criteria["u_lim_restored"] = restored_ulim == orig_ulim
        criteria["udcomp_enable_restored"] = restored_udcomp_ena == orig_udcomp_ena
        criteria["debugw3_restored"] = restored_debugw3 == orig_debugw3

        metrics = {
            "passed": all(criteria.values()),
            "criteria": criteria,
            "abs_tol_pu": args.abs_tol_pu,
            "measurements": measurements,
        }
        report = {
            "test": NAME,
            "firmware": firmware_metadata(paths),
            "hardware": hardware,
            "setup": {
                "settle_s": args.settle_s,
                "abs_tol_pu": args.abs_tol_pu,
                "ulim_sweep": [0.5, 0.866, 1.0],
                "original_u_lim_raw": orig_ulim,
                "original_udcomp_enable_raw": orig_udcomp_ena,
                "original_debugw3_raw": orig_debugw3,
            },
            "snapshots": {
                "initial": snapshot_dict(initial),
                "final": snapshot_dict(final),
            },
            "metrics": metrics,
        }
        json_path = output / "report.json"
        write_json(json_path, report)
        print(f"JSON: {json_path}")
        import json

        print(json.dumps(metrics, ensure_ascii=False, indent=2))
        if not metrics["passed"]:
            raise RuntimeError(
                "Dynamic current-regulator limit acceptance criteria failed."
            )
