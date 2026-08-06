from __future__ import annotations

import json
from argparse import ArgumentParser, Namespace, SUPPRESS
from pathlib import Path

from motor_debug_core.datalogger import Channel
from motor_debug_core.project import ProjectPaths
from motor_debug_core.values import FixedPoint

from .artifacts import plot_field_weakening, write_csv, write_json
from .common import (
    HardwareSession,
    firmware_metadata,
    resolve_output,
    snapshot_dict,
)
from .metrics import analyze_field_weakening


NAME = "field-weakening"
HELP = (
    "Validate automatic field weakening: negative d-axis current lets the "
    "motor exceed the voltage-limited speed."
)
TARGET_SPEED_RPM = 740.0


def register(parser: ArgumentParser) -> None:
    parser.add_argument(
        "--start-drive",
        action="store_true",
        help=SUPPRESS,
    )
    parser.add_argument("--timeout", type=float, default=4.0)
    parser.add_argument("--sample-divider", type=int, default=100)
    parser.add_argument("--target-speed", type=float, default=TARGET_SPEED_RPM)
    parser.add_argument("--min-id-weakening-a", type=float, default=1.0)
    parser.add_argument("--min-voltage-over-limit", type=float, default=0.02)
    parser.add_argument("--enabled-speed-margin-rpm", type=float, default=20.0)
    parser.add_argument("--disabled-speed-margin-rpm", type=float, default=30.0)
    parser.add_argument("--min-speed-gain-rpm", type=float, default=20.0)
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("artifacts/e2e/field-weakening"),
    )


def _channels(session: HardwareSession) -> list[Channel]:
    return [
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
            "fw_err_pu",
            "field_weak.err",
            FixedPoint(24, 1.0, "pu"),
        ),
    ]


def run(args: Namespace, paths: ProjectPaths) -> None:
    if not 1 <= args.sample_divider <= 255:
        raise ValueError("--sample-divider must be in range 1..255.")
    if args.timeout <= 0:
        raise ValueError("--timeout must be positive.")
    if args.target_speed <= 0:
        raise ValueError("--target-speed must be positive.")
    if min(
        args.min_id_weakening_a,
        args.min_voltage_over_limit,
        args.enabled_speed_margin_rpm,
        args.disabled_speed_margin_rpm,
        args.min_speed_gain_rpm,
    ) < 0:
        raise ValueError("Acceptance limits must not be negative.")
    output = resolve_output(paths.root, args.output)
    output.mkdir(parents=True, exist_ok=True)

    with HardwareSession(paths) as session:
        initial = session.preflight()
        hardware = session.hardware()
        channels = _channels(session)
        original_enabled_raw = session.target.read_u32("field_weak.enabled")

        # Временное отключение ослабления: опорный прогон упирается в предел
        # напряжения и не достигает целевой скорости.
        session.target.write_u32("field_weak.enabled", 0)
        with session.temporary_engineering_value(
            "sm_ctrl.run_mode", 5
        ) as original_run_mode_raw:
            with session.temporary_engineering_value(
                "refs.speed_ref", args.target_speed, unit="rpm"
            ) as original_speed_ref_raw:
                disabled_before = session.verify_mode(5)
                disabled_capture = session.capture_triggered(
                    channels,
                    divider=args.sample_divider,
                    trigger_bit=4,
                    timeout_s=args.timeout,
                    validated_snapshot=disabled_before,
                )
        # Включение ослабления: отрицательный ток по оси d размагничивает и
        # позволяет превысить базовую (напряженно-ограниченную) скорость.
        session.target.write_u32("field_weak.enabled", 1)
        with session.temporary_engineering_value(
            "sm_ctrl.run_mode", 5
        ) as mode5_original_run_mode_raw:
            if mode5_original_run_mode_raw != original_run_mode_raw:
                raise RuntimeError(
                    "Run mode changed unexpectedly between weakening captures."
                )
            with session.temporary_engineering_value(
                "refs.speed_ref", args.target_speed, unit="rpm"
            ) as target_original_speed_ref_raw:
                enabled_before = session.verify_mode(5)
                enabled_capture = session.capture_triggered(
                    channels,
                    divider=args.sample_divider,
                    trigger_bit=4,
                    timeout_s=args.timeout,
                    validated_snapshot=enabled_before,
                )
        restored = session.drive.snapshot()
        restored_enabled_raw = session.target.read_u32("field_weak.enabled")
        if (
            restored.run_mode != original_run_mode_raw
            or restored.speed_ref_raw != original_speed_ref_raw
            or restored_enabled_raw != original_enabled_raw
        ):
            raise RuntimeError(
                "Temporary field-weakening test settings were not restored."
            )

    metrics = analyze_field_weakening(
        enabled_capture.samples,
        disabled_capture.samples,
        target_speed_rpm=args.target_speed,
        min_id_weakening_a=args.min_id_weakening_a,
        min_voltage_over_limit=args.min_voltage_over_limit,
        enabled_speed_margin_rpm=args.enabled_speed_margin_rpm,
        disabled_speed_margin_rpm=args.disabled_speed_margin_rpm,
        min_speed_gain_rpm=args.min_speed_gain_rpm,
    )
    metrics["criteria"]["runtime_entered_vector_mode"] = (
        enabled_capture.runtime.control_state == 5
        and enabled_capture.runtime.run_mode == 5
        and disabled_capture.runtime.control_state == 5
        and disabled_capture.runtime.run_mode == 5
    )
    metrics["criteria"]["temporary_settings_restored"] = (
        restored.run_mode == original_run_mode_raw
        and restored.speed_ref_raw == original_speed_ref_raw
        and restored_enabled_raw == original_enabled_raw
    )
    metrics["passed"] = all(metrics["criteria"].values())

    csv_enabled = output / "enabled.csv"
    csv_disabled = output / "disabled.csv"
    png_path = output / "speed-comparison.png"
    json_path = output / "report.json"
    write_csv(csv_enabled, enabled_capture.samples)
    write_csv(csv_disabled, disabled_capture.samples)
    plot_field_weakening(
        png_path,
        enabled_capture.samples,
        disabled_capture.samples,
        target_speed_rpm=args.target_speed,
    )
    report = {
        "test": NAME,
        "firmware": firmware_metadata(paths),
        "hardware": hardware,
        "setup": {
            "required_run_mode": 5,
            "target_speed_rpm": args.target_speed,
            "trigger_bit": 4,
            "trigger_timeout_s": args.timeout,
            "sample_divider": args.sample_divider,
            "original_run_mode_raw": original_run_mode_raw,
            "original_speed_reference_raw": original_speed_ref_raw,
            "original_field_weak_enabled_raw": original_enabled_raw,
        },
        "snapshots": {
            "initial": snapshot_dict(initial),
            "disabled_before": snapshot_dict(disabled_capture.before),
            "disabled_runtime": snapshot_dict(disabled_capture.runtime),
            "disabled_after": snapshot_dict(disabled_capture.after),
            "enabled_before": snapshot_dict(enabled_capture.before),
            "enabled_runtime": snapshot_dict(enabled_capture.runtime),
            "enabled_after": snapshot_dict(enabled_capture.after),
            "restored": snapshot_dict(restored),
        },
        "metrics": metrics,
        "artifacts": {
            "enabled_csv": csv_enabled.name,
            "disabled_csv": csv_disabled.name,
            "plot": png_path.name,
        },
    }
    write_json(json_path, report)
    print(json.dumps(metrics, ensure_ascii=False, indent=2))
    print(f"JSON: {json_path}")
    print(f"CSV: {csv_enabled}")
    print(f"CSV: {csv_disabled}")
    print(f"PNG: {png_path}")
    if not metrics["passed"]:
        raise RuntimeError("Field-weakening acceptance criteria failed.")
