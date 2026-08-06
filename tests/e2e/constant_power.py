from __future__ import annotations

import json
from argparse import ArgumentParser, Namespace, SUPPRESS
from pathlib import Path

from motor_debug_core.datalogger import Channel
from motor_debug_core.project import ProjectPaths

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
    "Validate constant-power acceleration: the q-axis current reference is "
    "clamped to P / speed so that Iq x speed stays at the power limit."
)
TARGET_SPEED_RPM = 500.0
_IQ_ONE = 0x01000000


def register(parser: ArgumentParser) -> None:
    parser.add_argument(
        "--start-drive",
        action="store_true",
        help=SUPPRESS,
    )
    parser.add_argument("--timeout", type=float, default=8.0)
    parser.add_argument("--sample-divider", type=int, default=160)
    parser.add_argument("--power-fraction", type=float, default=0.5)
    parser.add_argument("--max-power-spread-percent", type=float, default=20.0)
    parser.add_argument("--max-power-error-percent", type=float, default=15.0)
    parser.add_argument("--min-binding-samples", type=int, default=20)
    parser.add_argument("--min-binding-speed-span-pu", type=float, default=0.15)
    parser.add_argument("--current-reduction-percent", type=float, default=5.0)
    parser.add_argument(
        "--saturation-margin-percent", type=float, default=5.0
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("artifacts/e2e/constant-power"),
    )


def run(args: Namespace, paths: ProjectPaths) -> None:
    if not 1 <= args.sample_divider <= 255:
        raise ValueError("--sample-divider must be in range 1..255.")
    if args.timeout <= 0:
        raise ValueError("--timeout must be positive.")
    if not 0 < args.power_fraction < 1.0:
        raise ValueError("--power-fraction must be in range (0, 1).")
    if min(
        args.max_power_spread_percent,
        args.max_power_error_percent,
        args.current_reduction_percent,
    ) < 0:
        raise ValueError("Acceptance limits must not be negative.")
    if args.min_binding_samples <= 0:
        raise ValueError("--min-binding-samples must be positive.")
    if args.min_binding_speed_span_pu <= 0:
        raise ValueError("--min-binding-speed-span-pu must be positive.")
    output = resolve_output(paths.root, args.output)
    output.mkdir(parents=True, exist_ok=True)

    with HardwareSession(paths) as session:
        initial = session.preflight()
        current_base_a = session.dictionary.codec(
            "pid_iq.pid_fdb_reg3", unit="A"
        ).decode(_IQ_ONE)
        speed_base_rpm = session.dictionary.codec(
            "cur_par.speed", unit="rpm"
        ).decode(_IQ_ONE)
        current_limit_a = session.read_engineering("refs.Iq_ref", unit="A")
        if current_limit_a <= 0:
            raise RuntimeError("Positive refs.Iq_ref current limit is required.")
        iq_max_pu = current_limit_a / current_base_a
        power_limit_pu = iq_max_pu * args.power_fraction
        crossover_pu = power_limit_pu / iq_max_pu
        if crossover_pu >= 1.0:
            raise RuntimeError(
                f"Power fraction {args.power_fraction} puts the constant-power "
                f"crossover at {crossover_pu:.2f} pu speed; it must be below "
                f"the 1.0 pu target."
            )
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
                "iq_ref_a",
                "pid_iq.pid_ref_reg3",
                session.dictionary.codec("pid_iq.pid_ref_reg3", unit="A"),
            ),
            Channel(
                "iq_a",
                "pid_iq.pid_fdb_reg3",
                session.dictionary.codec("pid_iq.pid_fdb_reg3", unit="A"),
            ),
        ]

        with session.temporary_engineering_value(
            "sm_ctrl.run_mode", 5
        ) as original_run_mode_raw:
            with session.temporary_engineering_value(
                "refs.speed_ref", TARGET_SPEED_RPM, unit="rpm"
            ) as original_speed_ref_raw:
                with session.temporary_raw(
                    "DebugW4", 1.0
                ) as original_enable_raw:
                    with session.temporary_raw(
                        "DebugW2", power_limit_pu
                    ) as original_power_raw:
                        before = session.verify_mode(5)
                        capture = session.capture_triggered(
                            channels,
                            divider=args.sample_divider,
                            trigger_bit=4,
                            timeout_s=args.timeout,
                            validated_snapshot=before,
                        )
        restored = session.drive.snapshot()
        restored_power_raw = session.target.read_u32("DebugW2")
        restored_enable_raw = session.target.read_u32("DebugW4")
        if (
            restored.run_mode != original_run_mode_raw
            or restored.speed_ref_raw != original_speed_ref_raw
            or restored_power_raw != original_power_raw
            or restored_enable_raw != original_enable_raw
        ):
            raise RuntimeError(
                "Constant-power temporary settings were not restored."
            )

    metrics = analyze_constant_power(
        capture.samples,
        power_limit_pu=power_limit_pu,
        current_base_a=current_base_a,
        speed_base_rpm=speed_base_rpm,
        iq_max_pu=iq_max_pu,
        target_speed_rpm=TARGET_SPEED_RPM,
        max_power_spread_percent=args.max_power_spread_percent,
        max_power_error_percent=args.max_power_error_percent,
        min_binding_samples=args.min_binding_samples,
        min_binding_speed_span_pu=args.min_binding_speed_span_pu,
        current_reduction_percent=args.current_reduction_percent,
        saturation_margin_percent=args.saturation_margin_percent,
    )
    metrics["criteria"]["runtime_entered_vector_mode"] = (
        capture.runtime.control_state == 5 and capture.runtime.run_mode == 5
    )
    metrics["criteria"]["temporary_settings_restored"] = (
        restored.run_mode == original_run_mode_raw
        and restored.speed_ref_raw == original_speed_ref_raw
        and restored_power_raw == original_power_raw
        and restored_enable_raw == original_enable_raw
    )
    metrics["passed"] = all(metrics["criteria"].values())

    csv_path = output / "capture.csv"
    png_path = output / "constant-power.png"
    json_path = output / "report.json"
    write_csv(csv_path, capture.samples)
    plot_constant_power(
        png_path,
        capture.samples,
        current_base_a=current_base_a,
        speed_base_rpm=speed_base_rpm,
        power_limit_pu=power_limit_pu,
    )
    report = {
        "test": NAME,
        "firmware": firmware_metadata(paths),
        "hardware": hardware,
        "setup": {
            "required_run_mode": 5,
            "target_speed_rpm": TARGET_SPEED_RPM,
            "current_limit_a": current_limit_a,
            "current_base_a": current_base_a,
            "speed_base_rpm": speed_base_rpm,
            "iq_max_pu": iq_max_pu,
            "power_fraction": args.power_fraction,
            "power_limit_pu": power_limit_pu,
            "crossover_speed_pu": crossover_pu,
            "trigger_bit": 4,
            "trigger_timeout_s": args.timeout,
            "sample_divider": args.sample_divider,
            "speed_controller": speed_controller,
            "original_run_mode_raw": original_run_mode_raw,
            "original_speed_reference_raw": original_speed_ref_raw,
            "original_power_limit_raw": original_power_raw,
            "original_enable_raw": original_enable_raw,
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
