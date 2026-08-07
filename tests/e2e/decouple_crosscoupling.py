from __future__ import annotations

import json
from argparse import ArgumentParser, Namespace, SUPPRESS
from pathlib import Path

from motor_debug_core.datalogger import Channel
from motor_debug_core.drive import SafetySnapshot
from motor_debug_core.project import ProjectPaths
from motor_debug_core.values import FixedPoint, Float32

from .artifacts import plot_decoupling, write_csv, write_json
from .common import (
    HardwareSession,
    firmware_metadata,
    resolve_output,
    snapshot_dict,
)
from .metrics import analyze_decoupling, calibrate_decoupling_gain, decouple_metric


NAME = "decouple-crosscoupling"
HELP = (
    "Accelerate the vector drive and verify that d/q current-regulator "
    "cross-coupling compensation relieves the d-axis regulator at high speed."
)
TARGET_SPEED_RPM = 500.0


def register(parser: ArgumentParser) -> None:
    parser.add_argument(
        "--start-drive",
        action="store_true",
        help=SUPPRESS,
    )
    parser.add_argument("--timeout", type=float, default=6.0)
    parser.add_argument("--sample-divider", type=int, default=60)
    parser.add_argument("--max-ud-ratio", type=float, default=0.6)
    parser.add_argument("--max-id-a", type=float, default=1.0)
    parser.add_argument("--min-coupling-v", type=float, default=0.5)
    parser.add_argument(
        "--load-nm",
        type=float,
        default=0.5,
        help="Sustained load torque so that the q-axis current stays high "
        "enough for the cross-coupling to be clearly measurable at speed.",
    )
    parser.add_argument(
        "--gain-k",
        type=float,
        default=None,
        help="Per-unit d-axis coupling gain to apply. "
        "By default it is calibrated from the decoupling-off run.",
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("artifacts/e2e/decouple-crosscoupling"),
    )


def _read_float(session: HardwareSession, expression: str) -> float:
    return Float32().decode(session.target.read_u32(expression))


def _sync_motor_parameters(session: HardwareSession) -> dict[str, object]:
    motor_type = session.target.read_u16("model.MotorType") & 0x7
    if motor_type != 1:  # MODEL_SYNC_MOTOR
        raise RuntimeError(
            "Cross-coupling decoupling test requires the synchronous-machine "
            f"model, got MotorType={motor_type}."
        )
    lsd = _read_float(session, "model.motorInternals.lsd")
    lsq = _read_float(session, "model.motorInternals.lsq")
    if lsd <= 0 or lsq <= 0:
        raise RuntimeError(
            "Synchronous-machine model has no positive d/q inductance "
            f"(Lsd={lsd:.3f} mH, Lsq={lsq:.3f} mH)."
        )
    return {"motor_type": motor_type, "lsd_mh": lsd, "lsq_mh": lsq}


def _iq24_raw(value: float) -> int:
    return int(round(value * (1 << 24))) & 0xFFFF_FFFF


def _set_debug(
    session: HardwareSession,
    w1: float,
    w2: float,
    w3: float,
) -> dict[str, int]:
    # DebugW* are plain IQ24 words; their dictionary entries are duplicated
    # (IQ and integer), so write the raw IQ24 value directly.
    originals = {}
    for name, value in (("DebugW1", w1), ("DebugW2", w2), ("DebugW3", w3)):
        originals[name] = session.target.read_u32(name)
        raw = _iq24_raw(value)
        session.target.write_u32(name, raw)
        if session.target.read_u32(name) != raw:
            raise RuntimeError(f"Failed to set {name}.")
    return originals


def _restore_debug(session: HardwareSession, originals: dict[str, int]) -> None:
    for name, value in originals.items():
        session.target.write_u32(name, value)
        if session.target.read_u32(name) != value:
            raise RuntimeError(f"Failed to restore {name}.")


def _capture(
    session: HardwareSession,
    args: Namespace,
    before: SafetySnapshot,
    channels: list[Channel],
) -> tuple[object, object]:
    result = session.capture_triggered(
        channels,
        divider=args.sample_divider,
        trigger_bit=4,
        timeout_s=args.timeout,
        validated_snapshot=before,
    )
    return result


def run(args: Namespace, paths: ProjectPaths) -> None:
    if not 1 <= args.sample_divider <= 255:
        raise ValueError("--sample-divider must be in range 1..255.")
    if args.timeout <= 0:
        raise ValueError("--timeout must be positive.")
    if min(args.max_ud_ratio, args.max_id_a, args.min_coupling_v) < 0:
        raise ValueError("Acceptance limits must not be negative.")
    if args.gain_k is not None and args.gain_k <= 0:
        raise ValueError("--gain-k must be positive when provided.")
    output = resolve_output(paths.root, args.output)
    output.mkdir(parents=True, exist_ok=True)

    with HardwareSession(paths) as session:
        initial = session.preflight()
        motor_parameters = _sync_motor_parameters(session)
        hardware = session.hardware()
        debug_original = _set_debug(session, 0.0, 0.0, 0.0)
        speed_codec = session.dictionary.codec("cur_par.speed", unit="rpm")
        current_codec = session.dictionary.codec(
            "pid_id.pid_fdb_reg3", unit="A"
        )
        voltage_codec = session.dictionary.codec(
            "pid_id.pid_out_reg3", unit="V"
        )
        dec_codec = FixedPoint(24, 1.0)
        off_channels = [
            Channel("speed_rpm", "cur_par.speed", speed_codec),
            Channel("id_a", "pid_id.pid_fdb_reg3", current_codec),
            Channel("iq_a", "pid_iq.pid_fdb_reg3", current_codec),
            Channel("ud_v", "pid_id.pid_out_reg3", voltage_codec),
        ]
        on_channels = [
            Channel("speed_rpm", "cur_par.speed", speed_codec),
            Channel("iq_a", "pid_iq.pid_fdb_reg3", current_codec),
            Channel("ud_v", "pid_id.pid_out_reg3", voltage_codec),
            Channel("dec_v", "DebugW4", dec_codec),
        ]

        if args.load_nm < 0:
            raise ValueError("--load-nm must not be negative.")
        load_original = _read_float(session, "model.load")
        if args.load_nm:
            session.target.write_u32(
                "model.load", Float32().encode(args.load_nm)
            )
        try:
            with session.temporary_engineering_value(
                "sm_ctrl.run_mode", 5
            ) as original_run_mode_raw:
                with session.temporary_engineering_value(
                    "refs.speed_ref", TARGET_SPEED_RPM, unit="rpm"
                ) as original_speed_ref_raw:
                    before = session.verify_mode(5)
                    off_result = _capture(session, args, before, off_channels)

                    if args.gain_k is not None:
                        gain_k = args.gain_k
                    else:
                        gain_k = calibrate_decoupling_gain(off_result.samples)
                    _set_debug(session, 1.0, gain_k, 0.0)
                    before = session.verify_mode(5)
                    on_result = _capture(session, args, before, on_channels)
        finally:
            _restore_debug(session, debug_original)
            session.target.write_u32(
                "model.load", Float32().encode(load_original)
            )
        restored = session.drive.snapshot()
        if (
            restored.run_mode != original_run_mode_raw
            or restored.speed_ref_raw != original_speed_ref_raw
        ):
            raise RuntimeError(
                "Mode or speed reference was not restored after the test."
            )

    off_metric = decouple_metric(
        off_result.samples,
        target_speed_rpm=TARGET_SPEED_RPM,
    )
    on_metric = decouple_metric(
        on_result.samples,
        target_speed_rpm=TARGET_SPEED_RPM,
    )
    metrics = analyze_decoupling(
        off_metric,
        on_metric,
        target_speed_rpm=TARGET_SPEED_RPM,
        max_ud_ratio=args.max_ud_ratio,
        max_id_a=args.max_id_a,
        min_coupling_v=args.min_coupling_v,
    )
    metrics["criteria"]["runtime_entered_vector_mode"] = (
        on_result.runtime.control_state == 5
        and on_result.runtime.run_mode == 5
    )
    metrics["criteria"]["gain_calibrated"] = args.gain_k is None
    metrics["passed"] = all(metrics["criteria"].values())

    off_csv_path = output / "capture-off.csv"
    on_csv_path = output / "capture-on.csv"
    png_path = output / "crosscoupling.png"
    json_path = output / "report.json"
    write_csv(off_csv_path, off_result.samples)
    write_csv(on_csv_path, on_result.samples)
    plot_decoupling(png_path, off_result.samples, on_result.samples)
    report = {
        "test": NAME,
        "firmware": firmware_metadata(paths),
        "hardware": hardware,
        "setup": {
            "required_run_mode": 5,
            "target_speed_rpm": TARGET_SPEED_RPM,
            "load_nm": args.load_nm,
            "gain_k": gain_k,
            "gain_kemf": 0.0,
            "motor": motor_parameters,
            "trigger_bit": 4,
            "trigger_timeout_s": args.timeout,
            "sample_divider": args.sample_divider,
            "original_run_mode_raw": original_run_mode_raw,
            "original_speed_reference_raw": original_speed_ref_raw,
        },
        "snapshots": {
            "initial": snapshot_dict(initial),
            "before": snapshot_dict(before),
            "runtime": snapshot_dict(on_result.runtime),
            "after": snapshot_dict(on_result.after),
            "restored": snapshot_dict(restored),
        },
        "metrics": metrics,
        "artifacts": {
            "csv_off": off_csv_path.name,
            "csv_on": on_csv_path.name,
            "plot": png_path.name,
        },
    }
    write_json(json_path, report)
    print(json.dumps(metrics, ensure_ascii=False, indent=2))
    print(f"JSON: {json_path}")
    print(f"CSV OFF: {off_csv_path}")
    print(f"CSV ON: {on_csv_path}")
    print(f"PNG: {png_path}")
    if not metrics["passed"]:
        raise RuntimeError(
            "Current-regulator cross-coupling decoupling criteria failed."
        )
