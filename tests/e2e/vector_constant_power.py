from __future__ import annotations

import json
from argparse import ArgumentParser, Namespace, SUPPRESS
from pathlib import Path

from motor_debug_core.datalogger import Channel
from motor_debug_core.project import ProjectPaths

from .artifacts import plot_vector_500, write_csv, write_json
from .common import (
    HardwareSession,
    firmware_metadata,
    resolve_output,
    snapshot_dict,
)
from .metrics import analyze_vector_500


NAME = "vector-constant-power"
HELP = "Validate constant-power acceleration for vector control."
TARGET_SPEED_RPM = 500.0


def register(parser: ArgumentParser) -> None:
    parser.add_argument(
        "--start-drive",
        action="store_true",
        help=SUPPRESS,
    )
    parser.add_argument("--timeout", type=float, default=6.0)
    parser.add_argument("--sample-divider", type=int, default=120)
    parser.add_argument("--max-time-to-95-s", type=float, default=2.0)
    parser.add_argument("--max-overshoot-percent", type=float, default=15.0)
    parser.add_argument("--max-settling-s", type=float, default=2.0)
    parser.add_argument("--max-steady-error-percent", type=float, default=5.0)
    parser.add_argument("--max-speed-ripple-rpm", type=float, default=30.0)
    parser.add_argument("--current-limit-margin-percent", type=float, default=40.0)
    parser.add_argument("--max-power-cv-percent", type=float, default=100.0)
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("artifacts/e2e/vector-constant-power"),
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
        args.current_limit_margin_percent,
        args.max_power_cv_percent,
    ) < 0:
        raise ValueError("Acceptance limits must not be negative.")
    output = resolve_output(paths.root, args.output)
    output.mkdir(parents=True, exist_ok=True)

    with HardwareSession(paths) as session:
        initial = session.preflight()
        current_limit = abs(session.read_engineering("refs.Iq_ref", unit="A"))
        if current_limit <= 0:
            raise RuntimeError("Positive refs.Iq_ref current limit is required.")
        ramp_time = session.read_engineering("rmp.T", unit="s")
        speed_controller = session.read_controller("pid_spd")
        hardware = session.hardware()

        half_speed_codec = session.dictionary.codec(
            "cur_par.speed", unit="rpm"
        )
        half_speed_iq = half_speed_codec.encode(TARGET_SPEED_RPM / 2.0)
        power_limit_raw = int(
            round(half_speed_iq * current_limit / 5.0)
        )
        power_limit_engineering = power_limit_raw / 16777216.0

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
            Channel(
                "iq_ref_a",
                "pid_iq.pid_ref_reg3",
                session.dictionary.codec("pid_iq.pid_ref_reg3", unit="A"),
            ),
        ]

        original_debug3 = session.target.read_u16("Debug3")
        original_debugw1 = session.target.read_u32("DebugW1")
        original_load_raw = session.target.read_u32("model.load")
        try:
            session.target.write_u16("Debug3", 1)
            session.target.write_u32("DebugW1", power_limit_raw)
            session.target.write_u32("model.load", encode_float(0.1))

            with session.temporary_engineering_value(
                "sm_ctrl.run_mode", 5
            ) as original_run_mode_raw:
                with session.temporary_engineering_value(
                    "refs.speed_ref", TARGET_SPEED_RPM, unit="rpm"
                ) as original_speed_ref_raw:
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
        finally:
            session.target.write_u16("Debug3", original_debug3)
            session.target.write_u32("DebugW1", original_debugw1)
            session.target.write_u32("model.load", original_load_raw)

    metrics = analyze_vector_500(
        capture.samples,
        target_speed_rpm=TARGET_SPEED_RPM,
        current_limit_a=current_limit,
        max_time_to_95_s=args.max_time_to_95_s,
        max_overshoot_percent=args.max_overshoot_percent,
        max_settling_s=args.max_settling_s,
        max_steady_error_percent=args.max_steady_error_percent,
        max_speed_ripple_rpm=args.max_speed_ripple_rpm,
        current_limit_margin_percent=args.current_limit_margin_percent,
    )

    power_cv = _analyze_constant_power(
        capture.samples,
        power_limit_raw=power_limit_raw,
    )
    metrics["power_limit_iq"] = power_limit_raw
    metrics["power_limit_engineering"] = power_limit_engineering
    metrics.update(power_cv)
    metrics["criteria"]["runtime_entered_vector_mode"] = (
        capture.runtime.control_state == 5 and capture.runtime.run_mode == 5
    )
    metrics["criteria"]["speed_reference_restored"] = (
        restored.speed_ref_raw == original_speed_ref_raw
    )
    metrics["criteria"]["run_mode_restored"] = (
        restored.run_mode == original_run_mode_raw
    )
    metrics["criteria"]["capture_window_covers_limits"] = (
        metrics["window_s"]
        >= max(args.max_time_to_95_s, args.max_settling_s) + 0.1
    )
    metrics["criteria"]["constant_power_region_detected"] = (
        power_cv["acceleration_region_power_cv_percent"]
        <= args.max_power_cv_percent
    )
    metrics["passed"] = all(metrics["criteria"].values())

    csv_path = output / "capture.csv"
    png_path = output / "speed-transition.png"
    json_path = output / "report.json"
    write_csv(csv_path, capture.samples)
    plot_vector_500(png_path, capture.samples)
    report = {
        "test": NAME,
        "firmware": firmware_metadata(paths),
        "hardware": hardware,
        "setup": {
            "required_run_mode": 5,
            "target_speed_rpm": TARGET_SPEED_RPM,
            "current_limit_a": current_limit,
            "ramp_time_s": ramp_time,
            "trigger_bit": 4,
            "trigger_timeout_s": args.timeout,
            "sample_divider": args.sample_divider,
            "speed_controller": speed_controller,
            "power_limit_iq": power_limit_raw,
            "power_limit_engineering": power_limit_engineering,
            "model_load_nm": 0.1,
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
        raise RuntimeError("Vector constant power acceptance criteria failed.")


def _analyze_constant_power(
    samples,
    *,
    power_limit_raw: int,
) -> dict:
    speed_rpm = [float(s.values["speed_rpm"]) for s in samples]
    iq_a = [float(s.values["iq_a"]) for s in samples]
    time_s = [float(s.time_s) for s in samples]

    acceleration_power = [
        iq * speed for iq, speed in zip(iq_a, speed_rpm)
    ]

    mean_power = sum(acceleration_power) / len(acceleration_power)
    if mean_power <= 0:
        cv_percent = 0.0
    else:
        variance = sum(
            (p - mean_power) ** 2 for p in acceleration_power
        ) / len(acceleration_power)
        std_power = variance ** 0.5
        cv_percent = std_power / mean_power * 100.0

    acceleration_stop = _estimate_acceleration_stop(speed_rpm)
    if acceleration_stop is not None:
        region_power = acceleration_power[:acceleration_stop]
        region_mean = sum(region_power) / len(region_power)
        if region_mean <= 0:
            region_cv_percent = 0.0
        else:
            region_variance = sum(
                (p - region_mean) ** 2 for p in region_power
            ) / len(region_power)
            region_std = region_variance ** 0.5
            region_cv_percent = region_std / region_mean * 100.0
    else:
        region_cv_percent = cv_percent

    return {
        "acceleration_power_mean_a_rpm": mean_power,
        "acceleration_power_cv_percent": cv_percent,
        "acceleration_region_power_cv_percent": region_cv_percent,
    }


def _estimate_acceleration_stop(speed_rpm: list[float]) -> int | None:
    if not speed_rpm:
        return None
    peak = max(speed_rpm)
    if peak <= 0:
        return None
    threshold = peak * 0.95
    for index in range(len(speed_rpm) - 1, -1, -1):
        if speed_rpm[index] < threshold:
            return index + 1
    return len(speed_rpm)


def encode_float(value: float) -> int:
    import struct
    return struct.unpack("<I", struct.pack("<f", value))[0]
