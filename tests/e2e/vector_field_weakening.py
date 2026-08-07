from __future__ import annotations

import json
import time
from argparse import ArgumentParser, Namespace, SUPPRESS
from contextlib import nullcontext
from pathlib import Path

from motor_debug_core.datalogger import Channel
from motor_debug_core.project import ProjectPaths

from .artifacts import plot_field_weakening, write_csv, write_json
from .common import (
    HardwareSession,
    firmware_metadata,
    resolve_output,
    snapshot_dict,
)
from .metrics import analyze_field_weakening


NAME = "vector-field-weakening"
HELP = (
    "Validate automatic field weakening: negative d-axis current lets the "
    "vector drive exceed the voltage-limited speed ceiling."
)

DEBUG_VARS = ("DebugW1", "DebugW2", "DebugW3", "DebugW4")


def register(parser: ArgumentParser) -> None:
    parser.add_argument(
        "--start-drive",
        action="store_true",
        help=SUPPRESS,
    )
    parser.add_argument("--target-speed-rpm", type=float, default=1500.0)
    parser.add_argument(
        "--current-limit-a",
        type=float,
        default=3.0,
        help="Temporarily raise refs.Iq_ref (amps) so the drive is "
        "voltage-limited rather than current-limited (the demo's default "
        "1 A limit stalls the motor in the current-limited regime, where "
        "field weakening has nothing to act on). The limit is restored "
        "after the test.",
    )
    parser.add_argument("--timeout", type=float, default=8.0)
    parser.add_argument("--sample-divider", type=int, default=200)
    parser.add_argument("--coast-wait-s", type=float, default=2.0)
    parser.add_argument("--fw-kp", type=float, default=0.5)
    parser.add_argument("--fw-ki", type=float, default=0.001)
    parser.add_argument("--max-fw-current-fraction", type=float, default=0.2)
    parser.add_argument("--min-fw-id-fraction", type=float, default=0.05)
    parser.add_argument("--fw-speed-gain-fraction", type=float, default=0.02)
    parser.add_argument("--max-id-noise-a", type=float, default=0.02)
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("artifacts/e2e/vector-field-weakening"),
    )


def _iq24(value: float) -> int:
    """Encode an IQ24 per-unit value (DebugW gains are raw IQ24 longs)."""
    return int(round(value * (1 << 24)))


def _set_debug(session: HardwareSession, values: dict[str, int]) -> None:
    for expression, raw in values.items():
        session.target.write_u32(expression, raw)


def run(args: Namespace, paths: ProjectPaths) -> None:
    if not 1 <= args.sample_divider <= 255:
        raise ValueError("--sample-divider must be in range 1..255.")
    if args.target_speed_rpm <= 0:
        raise ValueError("--target-speed-rpm must be positive.")
    if args.timeout <= 0 or args.coast_wait_s < 0:
        raise ValueError("Timing parameters must be valid.")
    if not 0 < args.max_fw_current_fraction <= 1:
        raise ValueError("--max-fw-current-fraction must be in (0, 1].")
    if min(
        args.min_fw_id_fraction,
        args.fw_speed_gain_fraction,
        args.max_id_noise_a,
    ) < 0:
        raise ValueError("Acceptance limits must not be negative.")
    output = resolve_output(paths.root, args.output)
    output.mkdir(parents=True, exist_ok=True)

    with HardwareSession(paths) as session:
        initial = session.preflight()
        iq_limit = abs(session.read_engineering("refs.Iq_ref", unit="A"))
        if iq_limit <= 0:
            raise RuntimeError("Positive refs.Iq_ref current limit is required.")
        if args.current_limit_a is not None:
            if args.current_limit_a <= 0:
                raise ValueError("--current-limit-a must be positive.")
            iq_limit = args.current_limit_a
        debug_original = {
            name: session.target.read_u32(name) for name in DEBUG_VARS
        }
        fw_gains = {
            "DebugW2": _iq24(args.fw_kp),
            "DebugW3": _iq24(args.fw_ki),
            "DebugW4": _iq24(args.max_fw_current_fraction),
        }
        hardware = session.hardware()
        channels = [
            Channel(
                "speed_rpm",
                "cur_par.speed",
                session.dictionary.codec("cur_par.speed", unit="rpm"),
            ),
            Channel(
                "id_ref_a",
                "pid_id.pid_ref_reg3",
                session.dictionary.codec("pid_id.pid_ref_reg3", unit="A"),
            ),
            Channel(
                "id_a",
                "pid_id.pid_fdb_reg3",
                session.dictionary.codec("pid_id.pid_fdb_reg3", unit="A"),
            ),
            Channel(
                "voltage_v",
                "pwm.U_mag",
                session.dictionary.codec("pwm.U_mag", unit="V"),
            ),
        ]

        iq_context = (
            session.temporary_engineering_value(
                "refs.Iq_ref", args.current_limit_a, unit="A"
            )
            if args.current_limit_a is not None
            else nullcontext()
        )
        with iq_context:
            with session.temporary_engineering_value(
                "sm_ctrl.run_mode", 5
            ) as original_run_mode_raw:
                    with session.temporary_engineering_value(
                        "refs.speed_ref", args.target_speed_rpm, unit="rpm"
                    ) as original_speed_ref_raw:
                        baseline_capture = None
                        fw_capture = None
                        try:
                            _set_debug(session, fw_gains)
                            session.target.write_u32("DebugW1", 0)
                            baseline_before = session.verify_mode(5)
                            baseline_capture = session.capture_triggered(
                                channels,
                                divider=args.sample_divider,
                                trigger_bit=4,
                                timeout_s=args.timeout,
                                validated_snapshot=baseline_before,
                            )
                            time.sleep(args.coast_wait_s)

                            session.target.write_u32("DebugW1", 1)
                            fw_before = session.verify_mode(5)
                            fw_capture = session.capture_triggered(
                                channels,
                                divider=args.sample_divider,
                                trigger_bit=4,
                                timeout_s=args.timeout,
                                validated_snapshot=fw_before,
                            )
                        finally:
                            _set_debug(session, debug_original)

        assert baseline_capture is not None and fw_capture is not None
        fw_diagnostics = {
            name: session.dictionary.codec(name).decode(
                session.target.read_u32(name)
            )
            for name in ("DebugF1", "DebugF2", "DebugF3")
        }
        restored = session.drive.snapshot()
        if (
            restored.run_mode != original_run_mode_raw
            or restored.speed_ref_raw != original_speed_ref_raw
        ):
            raise RuntimeError(
                "Mode or speed reference was not restored after the test."
            )

    metrics = analyze_field_weakening(
        baseline_capture.samples,
        fw_capture.samples,
        target_speed_rpm=args.target_speed_rpm,
        iq_limit_a=iq_limit,
        min_fw_id_fraction=args.min_fw_id_fraction,
        fw_speed_gain_fraction=args.fw_speed_gain_fraction,
        max_id_noise_a=args.max_id_noise_a,
    )
    metrics["criteria"]["runtime_entered_vector_mode_baseline"] = (
        baseline_capture.runtime.control_state == 5
        and baseline_capture.runtime.run_mode == 5
    )
    metrics["criteria"]["runtime_entered_vector_mode_fw"] = (
        fw_capture.runtime.control_state == 5
        and fw_capture.runtime.run_mode == 5
    )
    metrics["criteria"]["run_mode_restored"] = (
        restored.run_mode == original_run_mode_raw
    )
    metrics["passed"] = all(metrics["criteria"].values())

    baseline_csv = output / "baseline.csv"
    fw_csv = output / "field-weakening.csv"
    png_path = output / "speed-vs-id.png"
    json_path = output / "report.json"
    write_csv(baseline_csv, baseline_capture.samples)
    write_csv(fw_csv, fw_capture.samples)
    plot_field_weakening(
        png_path,
        baseline_capture.samples,
        fw_capture.samples,
        args.target_speed_rpm,
    )
    report = {
        "test": NAME,
        "firmware": firmware_metadata(paths),
        "hardware": hardware,
        "setup": {
            "required_run_mode": 5,
            "target_speed_rpm": args.target_speed_rpm,
            "current_limit_a": iq_limit,
            "current_limit_override_a": args.current_limit_a,
            "fw_kp": args.fw_kp,
            "fw_ki": args.fw_ki,
            "max_fw_current_fraction": args.max_fw_current_fraction,
            "trigger_bit": 4,
            "trigger_timeout_s": args.timeout,
            "sample_divider": args.sample_divider,
            "coast_wait_s": args.coast_wait_s,
            "debug_variables": DEBUG_VARS,
            "debug_original_raw": debug_original,
            "fw_gains_raw": fw_gains,
            "fw_diagnostics": fw_diagnostics,
            "original_run_mode_raw": original_run_mode_raw,
            "original_speed_reference_raw": original_speed_ref_raw,
        },
        "snapshots": {
            "initial": snapshot_dict(initial),
            "baseline_before": snapshot_dict(baseline_before),
            "baseline_runtime": snapshot_dict(baseline_capture.runtime),
            "baseline_after": snapshot_dict(baseline_capture.after),
            "fw_before": snapshot_dict(fw_before),
            "fw_runtime": snapshot_dict(fw_capture.runtime),
            "fw_after": snapshot_dict(fw_capture.after),
            "restored": snapshot_dict(restored),
        },
        "metrics": metrics,
        "artifacts": {
            "baseline_csv": baseline_csv.name,
            "fw_csv": fw_csv.name,
            "plot": png_path.name,
        },
    }
    write_json(json_path, report)
    print(json.dumps(metrics, ensure_ascii=False, indent=2))
    print(f"JSON: {json_path}")
    print(f"Baseline CSV: {baseline_csv}")
    print(f"FW CSV: {fw_csv}")
    print(f"PNG: {png_path}")
    if not metrics["passed"]:
        raise RuntimeError("Field weakening acceptance criteria failed.")
