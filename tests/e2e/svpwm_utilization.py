"""Hardware e2e for the continuous SVPWM without bus clamping.

The new PWM type (``PWM_TYPE_SVPWM``) is exercised through the current-regulated
vector-control software motor model (run mode 5), which is safe because the
phase current is regulated by the speed/current controllers.  This validates
end-to-end that the new firmware PWM runs on the target, drives the model to
speed without faults, applies a non-zero zero-sequence (min/max) offset, and
leaves every phase duty strictly inside the PWM period (no bus clamping).
The analytic DC-link utilization gain over sinusoidal PWM is covered by
``tests/unit/test_svpwm.py``, which does not need a controller.
"""

from __future__ import annotations

import csv
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


NAME = "svpwm-utilization"
HELP = "Validate continuous SVPWM (no bus clamping) against the motor model."
TARGET_SPEED_RPM = 500.0
PWM_TYPE_SIN = 1
PWM_TYPE_SVPWM = 6

_IQ24 = FixedPoint(24, 1.0, "p.u.")
_RPM = FixedPoint(24, 500.0, "rpm")


def _encode_perunit(value: float) -> int:
    return _IQ24.encode(value)


def _decode_perunit(raw: int) -> float:
    return _IQ24.decode(raw)


def register(parser: ArgumentParser) -> None:
    parser.add_argument("--timeout", type=float, default=8.0)
    parser.add_argument("--dwell-s", type=float, default=0.2)
    parser.add_argument("--min-speed-rpm", type=float, default=450.0)
    parser.add_argument(
        "--min-offset-pu", type=float, default=0.005,
        help="Minimum observed zero-sequence offset to consider injection active.",
    )
    parser.add_argument("--u-lim", type=float, default=1.0)
    parser.add_argument("--offset-k", type=float, default=1.0)
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("artifacts/e2e/svpwm-utilization"),
    )


def run(args: Namespace, paths: ProjectPaths) -> None:
    if args.timeout <= 0 or args.dwell_s <= 0:
        raise ValueError("--timeout and --dwell-s must be positive.")
    if not 0.0 <= args.min_speed_rpm <= TARGET_SPEED_RPM:
        raise ValueError("--min-speed-rpm is out of range.")
    output = resolve_output(paths.root, args.output)
    output.mkdir(parents=True, exist_ok=True)

    with HardwareSession(paths) as session:
        target = session.target
        initial = session.preflight()

        original_run_mode = target.read_u32("sm_ctrl.run_mode")
        original_speed_ref = target.read_u32("refs.speed_ref")
        original_pwm_type = target.read_u16("pwm.PWM_type")
        original_offset_k = target.read_u32("pwm.SVPWM_offsetK")
        original_u_lim = target.read_u32("pwm.U_lim")

        try:
            target.write_u32(
                "sm_ctrl.run_mode", 5
            )
            target.write_u32(
                "refs.speed_ref", _RPM.encode(TARGET_SPEED_RPM)
            )
            target.write_u16("pwm.PWM_type", PWM_TYPE_SVPWM)
            target.write_u32("pwm.SVPWM_offsetK", _encode_perunit(args.offset_k))
            target.write_u32("pwm.U_lim", _encode_perunit(args.u_lim))

            before = session.verify_mode(5)
            k_pwm = target.read_u32("pwm.k_pwm")
            session.drive.start(validated_snapshot=before)

            samples: list[dict[str, float | int]] = []
            peak_speed = 0.0
            peak_offset = 0.0
            interior = True
            fault = None
            deadline = time.time() + args.timeout
            while time.time() < deadline:
                time.sleep(args.dwell_s)
                state = session.drive.snapshot()
                if state.fault1 or state.fault2:
                    fault = f"fault 0x{state.fault1:04X}/0x{state.fault2:04X}"
                    break
                speed = _RPM.decode(target.read_u32("cur_par.speed"))
                gamma_a = target.read_u32("pwm.GammaA")
                gamma_b = target.read_u32("pwm.GammaB")
                gamma_c = target.read_u32("pwm.GammaC")
                offset = _decode_perunit(target.read_u32("pwm.SVPWM_offset"))
                peak_speed = max(peak_speed, speed)
                peak_offset = max(peak_offset, abs(offset))
                if not all(
                    0 < gamma < k_pwm for gamma in (gamma_a, gamma_b, gamma_c)
                ):
                    interior = False
                samples.append(
                    {
                        "speed_rpm": round(speed, 3),
                        "offset_pu": round(offset, 6),
                        "gamma_a": gamma_a,
                        "gamma_b": gamma_b,
                        "gamma_c": gamma_c,
                        "k_pwm": k_pwm,
                    }
                )
            after = session.stop_and_wait()
        finally:
            session.drive.stop()
            time.sleep(0.1)
            for expression, value in (
                ("sm_ctrl.run_mode", original_run_mode),
                ("refs.speed_ref", original_speed_ref),
                ("pwm.U_lim", original_u_lim),
                ("pwm.SVPWM_offsetK", original_offset_k),
            ):
                target.write_u32(expression, value)
            target.write_u16("pwm.PWM_type", original_pwm_type)
            restored_run_mode = target.read_u32("sm_ctrl.run_mode")
            restored_pwm_type = target.read_u16("pwm.PWM_type")
            restored_u_lim = target.read_u32("pwm.U_lim")
            restored_offset_k = target.read_u32("pwm.SVPWM_offsetK")
            restored_speed = target.read_u32("refs.speed_ref")

        criteria = {
            "reached_target_speed": peak_speed >= args.min_speed_rpm,
            "injection_active": peak_offset >= args.min_offset_pu,
            "no_bus_clamping": interior,
            "no_fault": fault is None,
            "stopped_cleanly": (
                after.control_state == 0
                and after.protection_state == 1
                and not after.fault1
                and not after.fault2
            ),
            "parameters_restored": (
                restored_run_mode == original_run_mode
                and restored_pwm_type == original_pwm_type
                and restored_u_lim == original_u_lim
                and restored_offset_k == original_offset_k
                and restored_speed == original_speed_ref
            ),
        }
        metrics = {
            "peak_speed_rpm": round(peak_speed, 3),
            "peak_zero_sequence_offset_pu": round(peak_offset, 6),
            "pwm_type": PWM_TYPE_SVPWM,
            "offset_k": args.offset_k,
            "u_lim": args.u_lim,
            "criteria": criteria,
        }
        metrics["passed"] = all(criteria.values())

        csv_path = output / "capture.csv"
        json_path = output / "report.json"
        rows = [{"sample": i, **s} for i, s in enumerate(samples)]
        with csv_path.open("w", encoding="utf-8-sig", newline="") as stream:
            writer = csv.DictWriter(
                stream,
                fieldnames=[
                    "sample",
                    "speed_rpm",
                    "offset_pu",
                    "gamma_a",
                    "gamma_b",
                    "gamma_c",
                    "k_pwm",
                ],
                lineterminator="\n",
            )
            writer.writeheader()
            writer.writerows(rows)
        report = {
            "test": NAME,
            "firmware": firmware_metadata(paths),
            "hardware": session.hardware(),
            "setup": {
                "run_mode": 5,
                "target_speed_rpm": TARGET_SPEED_RPM,
                "pwm_type": PWM_TYPE_SVPWM,
                "offset_k": args.offset_k,
                "u_lim": args.u_lim,
                "dwell_s": args.dwell_s,
                "timeout_s": args.timeout,
            },
            "snapshots": {
                "initial": snapshot_dict(initial),
                "before": snapshot_dict(before),
                "after": snapshot_dict(after),
            },
            "metrics": metrics,
            "artifacts": {"csv": csv_path.name},
        }
        write_json(json_path, report)
        print(json.dumps(metrics, ensure_ascii=False, indent=2))
        print(f"JSON: {json_path}")
        print(f"CSV: {csv_path}")
        if not metrics["passed"]:
            raise RuntimeError(
                "SVPWM utilization acceptance criteria failed."
            )
