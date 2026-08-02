from __future__ import annotations

import json
from argparse import ArgumentParser, Namespace, SUPPRESS
from pathlib import Path

from motor_debug_core.datalogger import Channel
from motor_debug_core.project import ProjectPaths
from motor_debug_core.values import FixedPoint

from .artifacts import plot_field_weaken, write_csv, write_json
from .common import (
    HardwareSession,
    firmware_metadata,
    resolve_output,
    snapshot_dict,
)
from .metrics import analyze_field_weaken


NAME = "field-weaken"
HELP = "Validate field-weakening acceleration above nominal speed."
TARGET_SPEED_RPM = 750.0


def register(parser: ArgumentParser) -> None:
    parser.add_argument(
        "--start-drive",
        action="store_true",
        help=SUPPRESS,
    )
    parser.add_argument("--timeout", type=float, default=6.0)
    parser.add_argument("--sample-divider", type=int, default=120)
    parser.add_argument("--max-time-to-95-s", type=float, default=2.5)
    parser.add_argument("--max-overshoot-percent", type=float, default=15.0)
    parser.add_argument("--max-settling-s", type=float, default=3.0)
    parser.add_argument("--max-steady-error-percent", type=float, default=3.0)
    parser.add_argument("--max-speed-ripple-rpm", type=float, default=30.0)
    parser.add_argument("--fw-gain", type=float, default=0.1)
    parser.add_argument("--fw-max-id-a", type=float, default=0.5)
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("artifacts/e2e/field-weaken"),
    )


def run(args: Namespace, paths: ProjectPaths) -> None:
    if not 1 <= args.sample_divider <= 255:
        raise ValueError("--sample-divider must be in range 1..255.")
    if args.timeout <= 0:
        raise ValueError("--timeout must be positive.")
    if min(
        args.max_time_to_95_s,
        args.max_overshoot_percent,
        args.max_settling_s,
        args.max_steady_error_percent,
        args.max_speed_ripple_rpm,
    ) < 0:
        raise ValueError("Acceptance limits must not be negative.")
    if args.fw_gain <= 0:
        raise ValueError("--fw-gain must be positive.")
    if args.fw_max_id_a <= 0:
        raise ValueError("--fw-max-id-a must be positive.")
    output = resolve_output(paths.root, args.output)
    output.mkdir(parents=True, exist_ok=True)

    with HardwareSession(paths) as session:
        initial = session.preflight()
        speed_nom = session.read_engineering("drv_params.speed_nom", unit="rpm")
        if speed_nom <= 0:
            raise RuntimeError("Positive drv_params.speed_nom is required.")
        current_limit = abs(session.read_engineering("refs.Iq_ref", unit="A"))
        if current_limit <= 0:
            raise RuntimeError("Positive refs.Iq_ref current limit is required.")
        ramp_time = session.read_engineering("rmp.T", unit="s")
        speed_controller = session.read_controller("pid_spd")
        hardware = session.hardware()

        target_speed_normalized = TARGET_SPEED_RPM / speed_nom
        fw_gain = args.fw_gain
        fw_max_id_raw = args.fw_max_id_a / 5.0

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
                "fw_id_ref_a",
                "DebugW4",
                session.dictionary.codec("pid_id.pid_ref_reg3", unit="A"),
            ),
            Channel(
                "debug_w1",
                "DebugW1",
                session.dictionary.codec("DebugW1"),
            ),
        ]
        # Read DebugW2 and DebugW3 directly for diagnostics (before writes)
        debug_w2_raw = session.target.read_u32("DebugW2")
        debug_w3_raw = session.target.read_u32("DebugW3")
        print(f"Pre-write diagnostics: DebugW2_raw=0x{debug_w2_raw:08X}, DebugW3_raw=0x{debug_w3_raw:08X}")

        original_run_mode_raw = session.target.read_u32("sm_ctrl.run_mode")
        run_mode_codec = session.dictionary.codec("sm_ctrl.run_mode")
        session.target.write_u32("sm_ctrl.run_mode", run_mode_codec.encode(5))
        original_speed_ref_raw = session.target.read_u32("refs.speed_ref")
        speed_ref_codec = session.dictionary.codec("refs.speed_ref", unit="rpm")
        session.target.write_u32("refs.speed_ref", speed_ref_codec.encode(TARGET_SPEED_RPM))
        original_debug_w1 = session.target.read_u32("DebugW1")
        session.target.write_u32("DebugW1", 1)
        original_debug_w2 = session.target.read_u32("DebugW2")
        original_debug_w3 = session.target.read_u32("DebugW3")
        fw_gain_raw = FixedPoint(q=24, scale=1.0).encode(fw_gain)
        fw_max_id_encoded = FixedPoint(q=24, scale=1.0).encode(fw_max_id_raw)
        session.target.write_u32("DebugW2", fw_gain_raw)
        session.target.write_u32("DebugW3", fw_max_id_encoded)
        debug_w2_inner = session.target.read_u32("DebugW2")
        debug_w3_inner = session.target.read_u32("DebugW3")
        debug_w4_inner = session.target.read_u32("DebugW4")
        print(
            f"In-capture diagnostics: "
            f"DebugW2_raw=0x{debug_w2_inner:08X}, "
            f"DebugW3_raw=0x{debug_w3_inner:08X}, "
            f"DebugW4_raw=0x{debug_w4_inner:08X}"
        )
        debug_w2_inner = session.target.read_u32("DebugW2")
        debug_w3_inner = session.target.read_u32("DebugW3")
        debug_w4_inner = session.target.read_u32("DebugW4")
        print(
            f"In-capture diagnostics: "
            f"DebugW2_raw=0x{debug_w2_inner:08X}, "
            f"DebugW3_raw=0x{debug_w3_inner:08X}, "
            f"DebugW4_raw=0x{debug_w4_inner:08X}"
        )
        try:
            before = session.verify_mode(5)
            capture = session.capture_triggered(
                channels,
                divider=args.sample_divider,
                trigger_bit=4,
                timeout_s=args.timeout,
                validated_snapshot=before,
            )
        finally:
            session.target.write_u32("DebugW2", original_debug_w2)
            session.target.write_u32("DebugW3", original_debug_w3)
            session.target.write_u32("DebugW1", original_debug_w1)
            session.target.write_u32("sm_ctrl.run_mode", original_run_mode_raw)
            session.target.write_u32("refs.speed_ref", original_speed_ref_raw)
        restored = session.drive.snapshot()
        if (
            restored.run_mode != original_run_mode_raw
            or restored.speed_ref_raw != original_speed_ref_raw
        ):
            raise RuntimeError(
                "Mode or speed reference was not restored after the test."
            )

    metrics = analyze_field_weaken(
        capture.samples,
        target_speed_rpm=TARGET_SPEED_RPM,
        current_limit_a=current_limit,
        max_time_to_95_s=args.max_time_to_95_s,
        max_overshoot_percent=args.max_overshoot_percent,
        max_settling_s=args.max_settling_s,
        max_steady_error_percent=args.max_steady_error_percent,
        max_speed_ripple_rpm=args.max_speed_ripple_rpm,
        fw_max_id_a=args.fw_max_id_a,
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
    metrics["criteria"]["field_weakening_detected"] = any(
        s.values.get("fw_id_ref_a", 0) < -1e-6 for s in capture.samples
    )
    metrics["criteria"]["debug_w1_enabled"] = any(
        s.values.get("debug_w1", 0) > 0.5 for s in capture.samples
    )
    metrics["criteria"]["capture_window_covers_limits"] = (
        metrics["window_s"]
        >= max(args.max_time_to_95_s, args.max_settling_s) + 0.1
    )
    metrics["passed"] = all(metrics["criteria"].values())

    csv_path = output / "capture.csv"
    png_path = output / "field-weaken.png"
    json_path = output / "report.json"
    write_csv(csv_path, capture.samples)
    plot_field_weaken(png_path, capture.samples)
    report = {
        "test": NAME,
        "firmware": firmware_metadata(paths),
        "hardware": hardware,
        "setup": {
            "required_run_mode": 5,
            "target_speed_rpm": TARGET_SPEED_RPM,
            "current_limit_a": current_limit,
            "ramp_time_s": ramp_time,
            "speed_nom_rpm": speed_nom,
            "fw_gain": args.fw_gain,
            "fw_max_id_a": args.fw_max_id_a,
            "trigger_bit": 4,
            "trigger_timeout_s": args.timeout,
            "sample_divider": args.sample_divider,
            "speed_controller": speed_controller,
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
        raise RuntimeError("Field-weakening acceptance criteria failed.")
