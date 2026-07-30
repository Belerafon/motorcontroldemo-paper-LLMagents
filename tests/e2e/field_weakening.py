from __future__ import annotations

import json
from argparse import ArgumentParser, Namespace
from pathlib import Path

from motor_debug_core.datalogger import Channel
from motor_debug_core.project import ProjectPaths
from motor_debug_core.values import FixedPoint, SignedInt32

from .artifacts import plot_field_weakening, write_csv, write_json
from .common import (
    HardwareSession,
    firmware_metadata,
    resolve_output,
    snapshot_dict,
)
from .metrics import analyze_field_weakening


NAME = "field-weakening"
HELP = "Validate automatic negative Id above nominal PM motor speed."
RUN_MODE = 5


def register(parser: ArgumentParser) -> None:
    parser.add_argument("--target-speed-rpm", type=float, default=750.0)
    parser.add_argument("--timeout", type=float, default=8.0)
    parser.add_argument("--sample-divider", type=int, default=180)
    parser.add_argument("--max-time-to-95-s", type=float, default=3.0)
    parser.add_argument("--max-steady-speed-error-percent", type=float, default=5.0)
    parser.add_argument("--min-negative-id-a", type=float, default=0.05)
    parser.add_argument("--max-current-margin-percent", type=float, default=5.0)
    parser.add_argument("--max-voltage-margin-percent", type=float, default=8.0)
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("artifacts/e2e/field-weakening"),
    )


def run(args: Namespace, paths: ProjectPaths) -> None:
    if not 1 <= args.sample_divider <= 255:
        raise ValueError("--sample-divider must be in range 1..255.")
    if args.timeout <= 0:
        raise ValueError("--timeout must be positive.")
    if min(
        args.max_time_to_95_s,
        args.max_steady_speed_error_percent,
        args.min_negative_id_a,
        args.max_current_margin_percent,
        args.max_voltage_margin_percent,
    ) < 0:
        raise ValueError("Acceptance limits must not be negative.")

    output = resolve_output(paths.root, args.output)
    output.mkdir(parents=True, exist_ok=True)
    pu_codec = FixedPoint(q=24, unit="pu")
    integer_codec = SignedInt32()

    with HardwareSession(paths) as session:
        initial = session.preflight()
        nominal_speed_rpm = float(session.target.read_u16("drv_params.speed_nom"))
        protection_current_a = abs(
            session.read_engineering("sm_prot.Imax_protect", unit="A")
        )
        voltage_limit_pu = session.read_engineering("pwm.U_lim", unit="pu")
        hardware = session.hardware()
        if args.target_speed_rpm <= nominal_speed_rpm:
            raise RuntimeError(
                "Field-weakening target must be above nominal motor speed."
            )
        if protection_current_a <= 0 or voltage_limit_pu <= 0:
            raise RuntimeError("Positive current and voltage limits are required.")

        channels = [
            Channel(
                "speed_rpm",
                "cur_par.speed",
                session.dictionary.codec("cur_par.speed", unit="rpm"),
            ),
            Channel(
                "id_a",
                "pid_id.pid_fdb_reg3",
                session.dictionary.codec("pid_id.pid_fdb_reg3", unit="A"),
            ),
            Channel(
                "iq_a",
                "pid_iq.pid_fdb_reg3",
                session.dictionary.codec("pid_iq.pid_fdb_reg3", unit="A"),
            ),
            Channel(
                "voltage_demand_pu",
                "Debug2",
                pu_codec,
            ),
        ]

        with session.temporary_engineering_value(
            "sm_ctrl.run_mode", RUN_MODE
        ) as original_run_mode_raw:
            with session.temporary_engineering_value(
                "DebugW2", 1
            ) as original_enable_raw:
                with session.temporary_engineering_value(
                    "refs.speed_ref", args.target_speed_rpm, unit="rpm"
                ) as original_speed_ref_raw:
                    before = session.verify_mode(RUN_MODE)
                    capture = session.capture_triggered(
                        channels,
                        divider=args.sample_divider,
                        trigger_bit=4,
                        timeout_s=args.timeout,
                        validated_snapshot=before,
                    )

        restored = session.drive.snapshot()
        residual_id_ref_pu = pu_codec.decode(
            session.target.read_u32("field_weakening.id_ref")
        )
        restored_enable_raw = session.target.read_u32("DebugW2")
        restored_enable = integer_codec.decode(restored_enable_raw)
        enable_setting_restored = restored_enable_raw == original_enable_raw
        if (
            restored.run_mode != original_run_mode_raw
            or restored.speed_ref_raw != original_speed_ref_raw
            or not enable_setting_restored
        ):
            raise RuntimeError("Temporary field-weakening settings were not restored.")

    metrics = analyze_field_weakening(
        capture.samples,
        target_speed_rpm=args.target_speed_rpm,
        nominal_speed_rpm=nominal_speed_rpm,
        protection_current_a=protection_current_a,
        voltage_limit_pu=voltage_limit_pu,
        max_time_to_95_s=args.max_time_to_95_s,
        max_steady_speed_error_percent=args.max_steady_speed_error_percent,
        min_negative_id_a=args.min_negative_id_a,
        max_current_margin_percent=args.max_current_margin_percent,
        max_voltage_margin_percent=args.max_voltage_margin_percent,
    )
    metrics["criteria"]["runtime_entered_vector_mode"] = (
        capture.runtime.control_state == RUN_MODE
        and capture.runtime.run_mode == RUN_MODE
    )
    metrics["criteria"]["run_mode_restored"] = (
        restored.run_mode == original_run_mode_raw
    )
    metrics["criteria"]["speed_reference_restored"] = (
        restored.speed_ref_raw == original_speed_ref_raw
    )
    metrics["criteria"]["enable_setting_restored"] = (
        enable_setting_restored
    )
    metrics["criteria"]["id_reference_reset_after_stop"] = (
        abs(residual_id_ref_pu) <= 1.0 / (1 << 20)
    )
    metrics["passed"] = all(metrics["criteria"].values())

    csv_path = output / "capture.csv"
    png_path = output / "field-weakening.png"
    json_path = output / "report.json"
    write_csv(csv_path, capture.samples)
    plot_field_weakening(
        png_path,
        capture.samples,
        target_speed_rpm=args.target_speed_rpm,
        voltage_limit_pu=voltage_limit_pu,
    )
    report = {
        "test": NAME,
        "firmware": firmware_metadata(paths),
        "hardware": hardware,
        "setup": {
            "required_run_mode": RUN_MODE,
            "target_speed_rpm": args.target_speed_rpm,
            "nominal_speed_rpm": nominal_speed_rpm,
            "protection_current_a": protection_current_a,
            "voltage_limit_pu": voltage_limit_pu,
            "trigger_bit": 4,
            "trigger_timeout_s": args.timeout,
            "sample_divider": args.sample_divider,
            "original_run_mode_raw": original_run_mode_raw,
            "original_speed_reference_raw": original_speed_ref_raw,
            "original_enable_raw": original_enable_raw,
            "restored_enable": restored_enable,
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
