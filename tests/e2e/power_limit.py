from __future__ import annotations

import json
import math
import struct
import time
from argparse import ArgumentParser, Namespace, SUPPRESS
from contextlib import contextmanager
from pathlib import Path
from typing import Iterator

from motor_debug_core.datalogger import Channel
from motor_debug_core.project import ProjectPaths
from motor_debug_core.values import FixedPoint

from .artifacts import plot_power_limit, write_csv, write_json
from .common import (
    HardwareSession,
    firmware_metadata,
    resolve_output,
    snapshot_dict,
)
from .metrics import analyze_power_limit


NAME = "power-limit"
HELP = "Validate power-limited (constant power) acceleration in vector mode."

TARGET_SPEED_RPM = 500.0


def register(parser: ArgumentParser) -> None:
    parser.add_argument(
        "--start-drive",
        action="store_true",
        help=SUPPRESS,
    )
    parser.add_argument("--timeout", type=float, default=6.0)
    parser.add_argument("--sample-divider", type=int, default=8)
    parser.add_argument("--power-limit-fraction", type=float, default=0.5)
    parser.add_argument("--ramp-time-s", type=float, default=0.02)
    parser.add_argument("--power-limit-margin-percent", type=float, default=30.0)
    parser.add_argument("--min-limited-power-percent", type=float, default=80.0)
    parser.add_argument(
        "--max-constant-power-variation-percent", type=float, default=40.0
    )
    parser.add_argument("--min-limited-window-s", type=float, default=0.02)
    parser.add_argument("--min-speed-fraction", type=float, default=0.5)
    parser.add_argument("--current-limit-headroom", type=float, default=1.5)
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("artifacts/e2e/power-limit"),
    )


def _read_f32(session: HardwareSession, expression: str) -> float:
    return struct.unpack("<f", struct.pack("<I", session.target.read_u32(expression)))[
        0
    ]


@contextmanager
def _temporary_raw(
    session: HardwareSession, expression: str, raw_value: int
) -> Iterator[int]:
    """Set a raw RAM value and restore it, stopping the drive in the end.

    Debug variables are declared twice in cood.xml (IQ and integer), so the
    dictionary codec would encode a fractional power limit to zero. The firmware
    reads them as Q24, therefore we write raw Q24 words directly.
    """
    original = session.target.read_u32(expression)
    session.target.write_u32(expression, raw_value)
    if session.target.read_u32(expression) != raw_value:
        raise RuntimeError(
            f"Write verification failed for {expression}: "
            f"0x{session.target.read_u32(expression):08X} != 0x{raw_value:08X}."
        )
    try:
        yield original
    finally:
        session.drive.stop()
        time.sleep(0.1)
        session.target.write_u32(expression, original)
        if session.target.read_u32(expression) != original:
            raise RuntimeError(
                f"RAM restoration failed for {expression}: "
                f"0x{session.target.read_u32(expression):08X} != "
                f"0x{original:08X}."
            )


def _configure(session: HardwareSession, args: Namespace) -> dict[str, object]:
    power_nom_raw = session.target.read_u32("drv_params.power")
    power_nom_w = power_nom_raw / 64.0  # drv_params.power хранится в формате Q6
    if power_nom_w <= 0:
        raise RuntimeError("drv_params.power must be positive.")
    current_limit_a = abs(session.read_engineering("refs.Iq_ref", unit="A"))
    imax_protect_a = abs(session.read_engineering("sm_prot.Imax_protect", unit="A"))
    if imax_protect_a <= 0:
        raise RuntimeError("sm_prot.Imax_protect must be positive.")
    psi_pm = _read_f32(session, "model.motorInternals.m")
    pole_pairs = _read_f32(session, "model.motorInternals.pp")
    omega_target_rad_s = TARGET_SPEED_RPM * 2.0 * math.pi / 60.0

    working_iq_a = current_limit_a * max(1.0, args.current_limit_headroom)
    working_iq_a = min(working_iq_a, 0.8 * imax_protect_a)

    max_power_at_target_w = 1.5 * pole_pairs * psi_pm * working_iq_a * omega_target_rad_s

    return {
        "power_nom_w": power_nom_w,
        "power_nom_raw": power_nom_raw,
        "current_limit_a": current_limit_a,
        "imax_protect_a": imax_protect_a,
        "working_iq_a": working_iq_a,
        "psi_pm": psi_pm,
        "pole_pairs": pole_pairs,
        "omega_target_rad_s": omega_target_rad_s,
        "max_power_at_target_w": max_power_at_target_w,
        "power_limit_fraction": args.power_limit_fraction,
        "ramp_time_s": args.ramp_time_s,
    }


def _channels(
    session: HardwareSession, power_nom_w: float
) -> list[Channel]:
    return [
        Channel(
            "speed_rpm",
            "cur_par.speed",
            session.dictionary.codec("cur_par.speed", unit="rpm"),
        ),
        Channel(
            "speed_ref_rpm",
            "rmp.output",
            session.dictionary.codec("rmp.output", unit="rpm"),
        ),
        Channel(
            "iq_ref_a",
            "pid_iq.pid_ref_reg3",
            session.dictionary.codec("pid_iq.pid_ref_reg3", unit="A"),
        ),
        Channel(
            "power_w",
            "DebugW4",
            FixedPoint(q=24, scale=power_nom_w, unit="W"),
        ),
    ]


def _natural_peak_power(samples) -> float:
    if not samples:
        raise RuntimeError("Calibration capture returned no samples.")
    return max(float(sample.values["power_w"]) for sample in samples)


def run(args: Namespace, paths: ProjectPaths) -> None:
    if not 1 <= args.sample_divider <= 255:
        raise ValueError("--sample-divider must be in range 1..255.")
    if args.timeout <= 0:
        raise ValueError("--timeout must be positive.")
    if min(
        args.power_limit_fraction,
        args.ramp_time_s,
        args.power_limit_margin_percent,
        args.min_limited_power_percent,
        args.max_constant_power_variation_percent,
        args.min_limited_window_s,
        args.min_speed_fraction,
        args.current_limit_headroom,
    ) < 0:
        raise ValueError("Acceptance limits must not be negative.")
    if not 0 < args.power_limit_fraction < 1:
        raise ValueError("--power-limit-fraction must be in (0, 1).")
    if args.ramp_time_s <= 0:
        raise ValueError("--ramp-time-s must be positive.")
    output = resolve_output(paths.root, args.output)
    output.mkdir(parents=True, exist_ok=True)

    with HardwareSession(paths) as session:
        initial = session.preflight()
        hardware = session.hardware()
        setup = _configure(session, args)
        power_nom_w = float(setup["power_nom_w"])
        working_iq_a = float(setup["working_iq_a"])
        default_ramp_time_s = session.read_engineering("rmp.T", unit="s")
        channels = _channels(session, power_nom_w)

        with session.temporary_engineering_value(
            "rmp.T", args.ramp_time_s, unit="s"
        ) as original_ramp_raw:
            with session.temporary_engineering_value(
                "refs.Iq_ref", working_iq_a, unit="A"
            ) as original_iq_ref_raw:
                with session.temporary_engineering_value(
                    "sm_ctrl.run_mode", 5
                ) as original_run_mode_raw:
                    with session.temporary_engineering_value(
                        "refs.speed_ref", TARGET_SPEED_RPM, unit="rpm"
                    ) as original_speed_ref_raw:
                        # Калибровка: разгон без ограничителя, измеряем пик мощности.
                        with _temporary_raw(
                            session, "DebugW1", 0
                        ) as original_dbg1_raw:
                            cal_before = session.verify_mode(5)
                            calibration = session.capture_triggered(
                                channels,
                                divider=args.sample_divider,
                                trigger_bit=4,
                                timeout_s=args.timeout,
                                validated_snapshot=cal_before,
                            )
                        natural_peak_w = _natural_peak_power(calibration.samples)
                        power_limit_w = natural_peak_w * args.power_limit_fraction
                        power_limit_w = max(power_limit_w, 0.5)
                        power_limit_w = min(power_limit_w, 0.9 * power_nom_w)
                        power_limit_pu = power_limit_w / power_nom_w
                        if power_limit_w <= 0:
                            raise RuntimeError(
                                "Calibration produced no measurable power; "
                                "the motor did not accelerate."
                            )
                        # Основной прогон: ограничитель мощности включён.
                        # DebugW1/DebugW2 пишем как сырые Q24 слова.
                        with _temporary_raw(
                            session, "DebugW1", 1 << 24
                        ) as original_dbg1_main_raw:
                            with _temporary_raw(
                                session,
                                "DebugW2",
                                int(round(power_limit_pu * (1 << 24))),
                            ) as original_dbg2_raw:
                                before = session.verify_mode(5)
                                capture = session.capture_triggered(
                                    channels,
                                    divider=args.sample_divider,
                                    trigger_bit=4,
                                    timeout_s=args.timeout,
                                    validated_snapshot=before,
                                )
        restored = session.drive.snapshot()
        restored_iq_ref_raw = session.target.read_u32("refs.Iq_ref")
        restored_dbg1_raw = session.target.read_u32("DebugW1")
        restored_dbg2_raw = session.target.read_u32("DebugW2")
        restored_ramp_raw = session.target.read_u32("rmp.T")
        if (
            restored.run_mode != original_run_mode_raw
            or restored.speed_ref_raw != original_speed_ref_raw
            or restored_iq_ref_raw != original_iq_ref_raw
            or restored_dbg1_raw != original_dbg1_raw
            or restored_dbg2_raw != original_dbg2_raw
            or restored_ramp_raw != original_ramp_raw
        ):
            raise RuntimeError(
                "Temporary power-limit settings were not restored in RAM."
            )

    setup["natural_peak_w"] = natural_peak_w
    setup["power_limit_w"] = power_limit_w
    setup["power_limit_pu"] = power_limit_pu

    metrics = analyze_power_limit(
        capture.samples,
        power_limit_w=power_limit_w,
        target_speed_rpm=TARGET_SPEED_RPM,
        power_limit_margin_percent=args.power_limit_margin_percent,
        min_limited_power_percent=args.min_limited_power_percent,
        max_constant_power_variation_percent=(
            args.max_constant_power_variation_percent
        ),
        min_limited_window_s=args.min_limited_window_s,
        min_speed_fraction=args.min_speed_fraction,
    )
    metrics["criteria"]["runtime_entered_vector_mode"] = (
        capture.runtime.control_state == 5 and capture.runtime.run_mode == 5
    )
    metrics["criteria"]["temporary_settings_restored"] = (
        restored.run_mode == original_run_mode_raw
        and restored.speed_ref_raw == original_speed_ref_raw
        and restored_iq_ref_raw == original_iq_ref_raw
        and restored_dbg1_raw == original_dbg1_raw
        and restored_dbg2_raw == original_dbg2_raw
        and restored_ramp_raw == original_ramp_raw
    )
    metrics["passed"] = all(metrics["criteria"].values())

    csv_path = output / "capture.csv"
    png_path = output / "power-transition.png"
    json_path = output / "report.json"
    write_csv(csv_path, capture.samples)
    plot_power_limit(png_path, capture.samples, power_limit_w=power_limit_w)
    report = {
        "test": NAME,
        "firmware": firmware_metadata(paths),
        "hardware": hardware,
        "setup": {
            "required_run_mode": 5,
            "target_speed_rpm": TARGET_SPEED_RPM,
            "default_ramp_time_s": default_ramp_time_s,
            "trigger_bit": 4,
            "trigger_timeout_s": args.timeout,
            "sample_divider": args.sample_divider,
            **setup,
            "original_run_mode_raw": original_run_mode_raw,
            "original_speed_reference_raw": original_speed_ref_raw,
            "original_iq_reference_raw": original_iq_ref_raw,
            "original_debug_w1_raw": original_dbg1_raw,
            "original_debug_w2_raw": original_dbg2_raw,
            "original_ramp_raw": original_ramp_raw,
        },
        "snapshots": {
            "initial": snapshot_dict(initial),
            "calibration_before": snapshot_dict(calibration.before),
            "calibration_runtime": snapshot_dict(calibration.runtime),
            "calibration_after": snapshot_dict(calibration.after),
            "before": snapshot_dict(capture.before),
            "runtime": snapshot_dict(capture.runtime),
            "after": snapshot_dict(capture.after),
            "restored": snapshot_dict(restored),
        },
        "metrics": metrics,
        "artifacts": {"csv": csv_path.name, "plot": png_path.name},
    }
    write_json(json_path, report)
    print(json.dumps(metrics, ensure_ascii=False, indent=2))
    print(f"JSON: {json_path}")
    print(f"CSV: {csv_path}")
    print(f"PNG: {png_path}")
    if not metrics["passed"]:
        raise RuntimeError("Power-limit acceptance criteria failed.")
