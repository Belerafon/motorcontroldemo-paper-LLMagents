from __future__ import annotations

import json
import time
from argparse import ArgumentParser, Namespace, SUPPRESS
from contextlib import contextmanager
from pathlib import Path
from typing import Iterator

from motor_debug_core.datalogger import Channel
from motor_debug_core.project import ProjectPaths

from .artifacts import plot_pwm_zero_seq, write_csv, write_json
from .common import (
    HardwareSession,
    firmware_metadata,
    resolve_output,
    snapshot_dict,
)
from .metrics import analyze_pwm_zero_seq


NAME = "pwm-zero-seq"
HELP = (
    "Validate the centered zero-sequence vector PWM on the software motor "
    "model (no bus tie, better DC-bus utilization than sinusoidal PWM)."
)
TARGET_SPEED_RPM = 500.0
PWM_TYPE_ZERO_SEQ = 6
# pwm.U_lim is an IQ24 quantity; 1.0 is the maximal linear range of the
# centered zero-sequence modulator (Udc/sqrt(3)).
U_LIM_FULL = 1 << 24


def register(parser: ArgumentParser) -> None:
    parser.add_argument(
        "--start-drive",
        action="store_true",
        help=SUPPRESS,
    )
    parser.add_argument("--timeout", type=float, default=6.0)
    parser.add_argument("--sample-divider", type=int, default=120)
    parser.add_argument("--min-speed-fraction", type=float, default=0.90)
    parser.add_argument("--max-centering-error-percent", type=float, default=2.0)
    parser.add_argument("--bus-tie-margin-ticks", type=float, default=2.0)
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("artifacts/e2e/pwm-zero-seq"),
    )


@contextmanager
def temporary_pwm_type(
    session: HardwareSession, value: int
) -> Iterator[int]:
    target = session.target
    original = target.read_u16("pwm.PWM_type")
    target.write_u16("pwm.PWM_type", value)
    if target.read_u16("pwm.PWM_type") != value:
        raise RuntimeError(
            f"Failed to set pwm.PWM_type: expected {value}, got "
            f"{target.read_u16('pwm.PWM_type')}."
        )
    try:
        yield original
    finally:
        session.drive.stop()
        time.sleep(0.1)
        target.write_u16("pwm.PWM_type", original)
        if target.read_u16("pwm.PWM_type") != original:
            raise RuntimeError(
                f"RAM restoration failed for pwm.PWM_type: expected "
                f"{original}, got {target.read_u16('pwm.PWM_type')}."
            )


@contextmanager
def temporary_u_lim(session: HardwareSession, value: int) -> Iterator[int]:
    target = session.target
    original = target.read_u32("pwm.U_lim")
    target.write_u32("pwm.U_lim", value)
    if target.read_u32("pwm.U_lim") != value:
        raise RuntimeError(
            f"Failed to set pwm.U_lim: expected 0x{value:08X}, got "
            f"0x{target.read_u32('pwm.U_lim'):08X}."
        )
    try:
        yield original
    finally:
        session.drive.stop()
        time.sleep(0.1)
        target.write_u32("pwm.U_lim", original)
        if target.read_u32("pwm.U_lim") != original:
            raise RuntimeError(
                f"RAM restoration failed for pwm.U_lim: expected "
                f"0x{original:08X}, got 0x{target.read_u32('pwm.U_lim'):08X}."
            )


def run(args: Namespace, paths: ProjectPaths) -> None:
    if not 1 <= args.sample_divider <= 255:
        raise ValueError("--sample-divider must be in range 1..255.")
    if args.timeout <= 0:
        raise ValueError("--timeout must be positive.")
    if min(
        args.min_speed_fraction,
        args.max_centering_error_percent,
        args.bus_tie_margin_ticks,
    ) < 0:
        raise ValueError("Acceptance limits must not be negative.")
    output = resolve_output(paths.root, args.output)
    output.mkdir(parents=True, exist_ok=True)

    with HardwareSession(paths) as session:
        initial = session.preflight()
        current_limit = abs(session.read_engineering("refs.Iq_ref", unit="A"))
        if current_limit <= 0:
            raise RuntimeError("Positive refs.Iq_ref current limit is required.")
        hardware = session.hardware()

        with session.temporary_engineering_value(
            "sm_ctrl.run_mode", 5
        ) as original_run_mode_raw:
            with session.temporary_engineering_value(
                "refs.speed_ref", TARGET_SPEED_RPM, unit="rpm"
            ) as original_speed_ref_raw:
                with temporary_pwm_type(
                    session, PWM_TYPE_ZERO_SEQ
                ) as original_pwm_type_raw:
                    with temporary_u_lim(session, U_LIM_FULL) as original_u_lim_raw:
                        k_pwm = session.target.read_u32("pwm.k_pwm")
                        if k_pwm <= 0:
                            raise RuntimeError(
                                f"Unexpected pwm.k_pwm value: {k_pwm}."
                            )
                        channels = [
                            Channel(
                                "gamma_a",
                                "pwm.GammaA",
                                session.dictionary.codec("pwm.GammaA"),
                            ),
                            Channel(
                                "gamma_b",
                                "pwm.GammaB",
                                session.dictionary.codec("pwm.GammaB"),
                            ),
                            Channel(
                                "gamma_c",
                                "pwm.GammaC",
                                session.dictionary.codec("pwm.GammaC"),
                            ),
                            Channel(
                                "speed_rpm",
                                "cur_par.speed",
                                session.dictionary.codec(
                                    "cur_par.speed", unit="rpm"
                                ),
                            ),
                        ]
                        before = session.verify_mode(5)
                        capture = session.capture_triggered(
                            channels,
                            divider=args.sample_divider,
                            trigger_bit=4,
                            timeout_s=args.timeout,
                            validated_snapshot=before,
                        )
        restored = session.drive.snapshot()
        if (
            restored.run_mode != original_run_mode_raw
            or restored.speed_ref_raw != original_speed_ref_raw
        ):
            raise RuntimeError(
                "Mode or speed reference was not restored after the test."
            )

    metrics = analyze_pwm_zero_seq(
        capture.samples,
        k_pwm=k_pwm,
        target_speed_rpm=TARGET_SPEED_RPM,
        min_speed_fraction=args.min_speed_fraction,
        max_centering_error_percent=args.max_centering_error_percent,
        bus_tie_margin_ticks=args.bus_tie_margin_ticks,
    )
    metrics["criteria"]["runtime_entered_vector_mode"] = (
        capture.runtime.control_state == 5 and capture.runtime.run_mode == 5
    )
    metrics["criteria"]["speed_reference_restored"] = (
        restored.speed_ref_raw == original_speed_ref_raw
    )
    metrics["criteria"]["run_mode_restored"] = (
        restored.run_mode == original_run_mode_raw
    )
    metrics["passed"] = all(metrics["criteria"].values())

    csv_path = output / "capture.csv"
    png_path = output / "pwm-envelopes.png"
    json_path = output / "report.json"
    write_csv(csv_path, capture.samples)
    plot_pwm_zero_seq(png_path, capture.samples, k_pwm)
    report = {
        "test": NAME,
        "firmware": firmware_metadata(paths),
        "hardware": hardware,
        "setup": {
            "required_run_mode": 5,
            "target_speed_rpm": TARGET_SPEED_RPM,
            "current_limit_a": current_limit,
            "pwm_type": PWM_TYPE_ZERO_SEQ,
            "u_lim_raw": U_LIM_FULL,
            "trigger_bit": 4,
            "trigger_timeout_s": args.timeout,
            "sample_divider": args.sample_divider,
            "k_pwm_ticks": k_pwm,
            "original_pwm_type_raw": original_pwm_type_raw,
            "original_u_lim_raw": original_u_lim_raw,
            "original_run_mode_raw": original_run_mode_raw,
            "original_speed_reference_raw": original_speed_ref_raw,
        },
        "snapshots": {
            "initial": snapshot_dict(initial),
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
        raise RuntimeError(
            "Centered zero-sequence PWM acceptance criteria failed."
        )
