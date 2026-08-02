from __future__ import annotations

import json
import struct
import time
from argparse import ArgumentParser, Namespace, SUPPRESS
from pathlib import Path

from motor_debug_core.project import ProjectPaths
from motor_debug_core.values import FixedPoint, Float32

from .artifacts import write_json
from .common import HardwareSession, firmware_metadata, resolve_output, snapshot_dict
from .metrics import analyze_dynamic_limits

NAME = "dynamic-limits"
HELP = "Verify that current regulator limits scale with DC link voltage."

IQ24 = FixedPoint(q=24)
F32 = Float32()


def _read_udc_meas(target) -> float:
    return IQ24.decode(target.read_u32("adc.Udc_meas"))


def _read_udc_internal(target) -> float:
    return F32.decode(target.read_u32("model.motorInternals.udc"))


def _write_udc_internal(target, value: float) -> None:
    target.write_u32("model.motorInternals.udc", F32.encode(value))


def register(parser: ArgumentParser) -> None:
    parser.add_argument(
        "--start-drive",
        action="store_true",
        help=SUPPRESS,
    )
    parser.add_argument("--timeout", type=float, default=6.0)
    parser.add_argument("--voltage-scale", type=float, default=0.5)
    parser.add_argument("--limit-tolerance", type=float, default=0.08)
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("artifacts/e2e/dynamic-limits"),
    )


def run(args: Namespace, paths: ProjectPaths) -> None:
    if args.voltage_scale <= 0 or args.voltage_scale > 1.0:
        raise ValueError("--voltage-scale must be in (0, 1].")
    if args.limit_tolerance < 0:
        raise ValueError("--limit-tolerance must not be negative.")
    output = resolve_output(paths.root, args.output)
    output.mkdir(parents=True, exist_ok=True)

    with HardwareSession(paths) as session:
        initial = session.preflight()
        hardware = session.hardware()

        with session.temporary_engineering_value(
            "sm_ctrl.run_mode", 5
        ) as original_run_mode_raw:
            with session.temporary_engineering_value(
                "refs.speed_ref", 200.0, unit="rpm"
            ) as original_speed_ref_raw:
                before = session.verify_mode(5)
                session.drive.start(validated_snapshot=before)
                time.sleep(0.3)

                udc_before = _read_udc_meas(session.target)
                pid_iq_baseline = session.read_controller("pid_iq")
                pid_id_baseline = session.read_controller("pid_id")
                pid_spd_baseline = session.read_controller("pid_spd")
                pwm_ulim_before = session.read_engineering("pwm.U_lim")
                iq_ref_before = session.read_engineering("refs.Iq_ref", unit="A")

                scaled_pwm_ulim = pwm_ulim_before * args.voltage_scale
                pwm_ulim_original_raw = session.target.read_u32("pwm.U_lim")
                pwm_ulim_codec = session.dictionary.codec("pwm.U_lim")
                pwm_ulim_scaled_raw = pwm_ulim_codec.encode(scaled_pwm_ulim)
                session.target.write_u32("pwm.U_lim", pwm_ulim_scaled_raw)
                time.sleep(0.1)
                pid_iq_pwm_scaled = session.read_controller("pid_iq")
                pid_id_pwm_scaled = session.read_controller("pid_id")
                pwm_ulim_after = session.read_engineering("pwm.U_lim")
                session.target.write_u32("pwm.U_lim", pwm_ulim_original_raw)

                scaled_iq_ref = iq_ref_before * args.voltage_scale
                iq_ref_original_raw = session.target.read_u32("refs.Iq_ref")
                iq_ref_codec = session.dictionary.codec("refs.Iq_ref", unit="A")
                iq_ref_scaled_raw = iq_ref_codec.encode(scaled_iq_ref)
                session.target.write_u32("refs.Iq_ref", iq_ref_scaled_raw)
                time.sleep(0.1)
                pid_spd_iq_scaled = session.read_controller("pid_spd")
                iq_ref_after = session.read_engineering("refs.Iq_ref", unit="A")
                session.target.write_u32("refs.Iq_ref", iq_ref_original_raw)

                original_udc = _read_udc_internal(session.target)
                _write_udc_internal(session.target, original_udc * args.voltage_scale)
                time.sleep(0.3)
                udc_after = _read_udc_meas(session.target)
                pid_iq_udc_scaled = session.read_controller("pid_iq")
                pid_id_udc_scaled = session.read_controller("pid_id")
                pid_spd_udc_scaled = session.read_controller("pid_spd")
                _write_udc_internal(session.target, original_udc)
                time.sleep(0.1)

                session.drive.stop()
                time.sleep(0.1)

        restored = session.drive.snapshot()
        if restored.run_mode != original_run_mode_raw:
            raise RuntimeError("Run mode was not restored after the test.")

    metrics = analyze_dynamic_limits(
        udc_before=udc_before,
        udc_after=udc_after,
        pwm_ulim_before=pwm_ulim_before,
        pwm_ulim_after=pwm_ulim_after,
        iq_ref_before=iq_ref_before,
        iq_ref_after=iq_ref_after,
        pid_iq_baseline=pid_iq_baseline,
        pid_iq_pwm_scaled=pid_iq_pwm_scaled,
        pid_iq_udc_scaled=pid_iq_udc_scaled,
        pid_id_baseline=pid_id_baseline,
        pid_id_pwm_scaled=pid_id_pwm_scaled,
        pid_id_udc_scaled=pid_id_udc_scaled,
        pid_spd_baseline=pid_spd_baseline,
        pid_spd_iq_scaled=pid_spd_iq_scaled,
        pid_spd_udc_scaled=pid_spd_udc_scaled,
        voltage_scale=args.voltage_scale,
        limit_tolerance=args.limit_tolerance,
    )

    metrics["criteria"]["runtime_entered_vector_mode"] = (
        restored.run_mode == 5 and restored.run_mode == 5
    )
    metrics["criteria"]["run_mode_restored"] = (
        restored.run_mode == original_run_mode_raw
    )
    metrics["passed"] = all(metrics["criteria"].values())

    json_path = output / "report.json"
    report = {
        "test": NAME,
        "firmware": firmware_metadata(paths),
        "hardware": hardware,
        "setup": {
            "voltage_scale": args.voltage_scale,
            "limit_tolerance": args.limit_tolerance,
            "original_udc_internal_v": original_udc,
        },
        "snapshots": {
            "initial": snapshot_dict(initial),
            "before": snapshot_dict(before),
            "restored": snapshot_dict(restored),
        },
        "metrics": metrics,
        "artifacts": {"json": json_path.name},
    }
    write_json(json_path, report)
    print(json.dumps(metrics, ensure_ascii=False, indent=2))
    print(f"JSON: {json_path}")
    if not metrics["passed"]:
        raise RuntimeError("Dynamic limits acceptance criteria failed.")
