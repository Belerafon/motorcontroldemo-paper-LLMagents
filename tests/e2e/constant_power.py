from __future__ import annotations

import json
from argparse import ArgumentParser, Namespace, SUPPRESS
from pathlib import Path

from motor_debug_core.datalogger import Channel
from motor_debug_core.project import ProjectPaths
from motor_debug_core.values import FixedPoint

from .artifacts import plot_constant_power, write_csv, write_json
from .common import (
    HardwareSession,
    firmware_metadata,
    resolve_output,
    snapshot_dict,
)
from .metrics import analyze_constant_power


NAME = "constant-power"
HELP = (
    "Validate constant-power acceleration: speed*Iq is held at a settable, "
    "dynamically maintained power limit."
)
TARGET_SPEED_RPM = 500.0
SPEED_BASE_RPM = 500.0
CURRENT_BASE_A = 5.0


def register(parser: ArgumentParser) -> None:
    parser.add_argument(
        "--start-drive",
        action="store_true",
        help=SUPPRESS,
    )
    parser.add_argument("--timeout", type=float, default=8.0)
    # The in-firmware software model accelerates very fast (to 500 rpm in ~0.1 s),
    # so a small divider is needed to resolve the constant-power speed window.
    parser.add_argument("--sample-divider", type=int, default=10)
    parser.add_argument("--power-limit-pu", type=float, default=0.06)
    parser.add_argument("--max-time-to-95-s", type=float, default=2.5)
    parser.add_argument("--max-power-exceed-percent", type=float, default=15.0)
    parser.add_argument("--engage-tolerance-percent", type=float, default=25.0)
    parser.add_argument(
        "--max-power-relative-std-percent", type=float, default=15.0
    )
    parser.add_argument("--max-power-offset-percent", type=float, default=20.0)
    parser.add_argument("--max-current-shape-factor", type=float, default=0.85)
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("artifacts/e2e/constant-power"),
    )


def run(args: Namespace, paths: ProjectPaths) -> None:
    if not 1 <= args.sample_divider <= 255:
        raise ValueError("--sample-divider must be in range 1..255.")
    if args.timeout <= 0 or args.power_limit_pu <= 0:
        raise ValueError("--timeout and --power-limit-pu must be positive.")
    output = resolve_output(paths.root, args.output)
    output.mkdir(parents=True, exist_ok=True)

    with HardwareSession(paths) as session:
        initial = session.preflight()
        current_limit_a = abs(session.read_engineering("refs.Iq_ref", unit="A"))
        if current_limit_a <= 0:
            raise RuntimeError("Positive refs.Iq_ref current limit is required.")
        ramp_time = session.read_engineering("rmp.T", unit="s")
        speed_controller = session.read_controller("pid_spd")
        hardware = session.hardware()
        channels = [
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
                "iq_a",
                "pid_iq.pid_fdb_reg3",
                session.dictionary.codec("pid_iq.pid_fdb_reg3", unit="A"),
            ),
            # DebugW3 is the firmware's per-unit power (speed_pu * iq_pu).
            Channel(
                "power_pu",
                "DebugW3",
                FixedPoint(q=24, scale=1.0, unit="pu"),
            ),
        ]

        # DebugW1/W2 have dual object-dictionary formats (IQ and integer), so the
        # dictionary codec would mis-encode a fractional per-unit power limit.
        # They are written here directly as IQ24 raw words and restored verbatim.
        debugw1_orig = session.target.read_u32("DebugW1")
        debugw2_orig = session.target.read_u32("DebugW2")
        power_limit_raw = round(args.power_limit_pu * (1 << 24))

        with session.temporary_engineering_value(
            "sm_ctrl.run_mode", 5
        ) as original_run_mode_raw:
            with session.temporary_engineering_value(
                "refs.speed_ref", TARGET_SPEED_RPM, unit="rpm"
            ) as original_speed_ref_raw:
                before = session.verify_mode(5)
                session.target.write_u32("DebugW2", 1)
                session.target.write_u32("DebugW1", power_limit_raw)
                if session.target.read_u32("DebugW1") != power_limit_raw:
                    raise RuntimeError("DebugW1 write verification failed.")
                try:
                    capture = session.capture_triggered(
                        channels,
                        divider=args.sample_divider,
                        trigger_bit=4,
                        timeout_s=args.timeout,
                        validated_snapshot=before,
                    )
                finally:
                    session.target.write_u32("DebugW1", debugw1_orig)
                    session.target.write_u32("DebugW2", debugw2_orig)
        restored = session.drive.snapshot()
        if (
            restored.run_mode != original_run_mode_raw
            or restored.speed_ref_raw != original_speed_ref_raw
        ):
            raise RuntimeError(
                "Mode or speed reference was not restored after the test."
            )

    metrics = analyze_constant_power(
        capture.samples,
        target_speed_rpm=TARGET_SPEED_RPM,
        power_limit_pu=args.power_limit_pu,
        speed_base_rpm=SPEED_BASE_RPM,
        current_base_a=CURRENT_BASE_A,
        max_time_to_95_s=args.max_time_to_95_s,
        max_power_exceed_percent=args.max_power_exceed_percent,
        engage_tolerance_percent=args.engage_tolerance_percent,
        max_power_relative_std_percent=args.max_power_relative_std_percent,
        max_power_offset_percent=args.max_power_offset_percent,
        max_current_shape_factor=args.max_current_shape_factor,
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
    png_path = output / "power-transition.png"
    json_path = output / "report.json"
    write_csv(csv_path, capture.samples)
    plot_constant_power(png_path, capture.samples, power_limit_pu=args.power_limit_pu)
    report = {
        "test": NAME,
        "firmware": firmware_metadata(paths),
        "hardware": hardware,
        "setup": {
            "required_run_mode": 5,
            "target_speed_rpm": TARGET_SPEED_RPM,
            "speed_base_rpm": SPEED_BASE_RPM,
            "current_base_a": CURRENT_BASE_A,
            "current_limit_a": current_limit_a,
            "power_limit_pu": args.power_limit_pu,
            "ramp_time_s": ramp_time,
            "trigger_bit": 4,
            "trigger_timeout_s": args.timeout,
            "sample_divider": args.sample_divider,
            "speed_controller": speed_controller,
            "original_run_mode_raw": original_run_mode_raw,
            "original_speed_reference_raw": original_speed_ref_raw,
            "original_debugw1_raw": debugw1_orig,
            "original_debugw2_raw": debugw2_orig,
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
        raise RuntimeError("Constant-power acceptance criteria failed.")
