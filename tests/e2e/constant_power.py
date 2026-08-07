from __future__ import annotations

import json
from argparse import ArgumentParser, Namespace, SUPPRESS
from contextlib import contextmanager
from pathlib import Path
from typing import Iterator

from motor_debug_core.datalogger import Channel
from motor_debug_core.project import ProjectPaths

from .artifacts import plot_constant_power, write_csv, write_json
from .common import (
    CaptureResult,
    HardwareSession,
    firmware_metadata,
    resolve_output,
    snapshot_dict,
)
from .metrics import analyze_constant_power


NAME = "constant-power"
HELP = "Validate constant-power acceleration under a settable power limit."

IQ24 = 1 << 24


def register(parser: ArgumentParser) -> None:
    parser.add_argument(
        "--start-drive",
        action="store_true",
        help=SUPPRESS,
    )
    parser.add_argument("--duration", type=float, default=3.0)
    parser.add_argument("--sample-divider", type=int, default=8)
    parser.add_argument("--target-fraction", type=float, default=1.0)
    parser.add_argument("--power-fraction", type=float, default=0.35)
    parser.add_argument("--window-lo-fraction", type=float, default=0.45)
    parser.add_argument("--window-hi-fraction", type=float, default=0.85)
    parser.add_argument("--max-power-cv-percent", type=float, default=12.0)
    parser.add_argument("--power-match-percent", type=float, default=15.0)
    parser.add_argument("--min-speed-span-rpm", type=float, default=40.0)
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("artifacts/e2e/constant-power"),
    )


@contextmanager
def temporary_raw_values(
    session: HardwareSession, values: dict[str, int]
) -> Iterator[dict[str, int]]:
    original = {name: session.target.read_u32(name) for name in values}
    try:
        for name, value in values.items():
            session.target.write_u32(name, value)
            actual = session.target.read_u32(name)
            if actual != value:
                raise RuntimeError(
                    f"Write verification failed for {name}: "
                    f"0x{actual:08X} != 0x{value:08X}."
                )
        yield original
    finally:
        for name, value in original.items():
            session.target.write_u32(name, value)
            restored = session.target.read_u32(name)
            if restored != value:
                raise RuntimeError(
                    f"RAM restoration failed for {name}: "
                    f"0x{restored:08X} != 0x{value:08X}."
                )


def _capture(
    session: HardwareSession,
    *,
    speed_ref_rpm: float,
    ramp_time_s: float,
    power_enabled: bool,
    power_set_pu: float,
    divider: int,
    duration_s: float,
    channel_limit: float,
) -> CaptureResult:
    channels = [
        Channel(
            "speed_rpm",
            "cur_par.speed",
            session.dictionary.codec("cur_par.speed", unit="rpm"),
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
        Channel(
            "limit_a",
            "pid_spd.pid_out_max",
            session.dictionary.codec("pid_spd.pid_out_max", unit="A"),
        ),
    ]
    power_limit_raw = round(power_set_pu * IQ24)
    with temporary_raw_values(
        session,
        {
            "DebugW1": 1 if power_enabled else 0,
            "DebugW2": power_limit_raw if power_enabled else 0,
        },
    ) as original_debug:
        with session.temporary_engineering_value(
            "sm_ctrl.run_mode", 5
        ) as original_run_mode_raw:
            with session.temporary_engineering_value(
                "refs.speed_ref", speed_ref_rpm, unit="rpm"
            ) as original_speed_ref_raw:
                with session.temporary_engineering_value(
                    "rmp.T", ramp_time_s, unit="s"
                ) as original_ramp_t_raw:
                    before = session.verify_mode(5)
                    capture = session.capture_triggered(
                        channels,
                        divider=divider,
                        trigger_bit=4,
                        timeout_s=duration_s,
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
        return capture


def run(args: Namespace, paths: ProjectPaths) -> None:
    if not 1 <= args.sample_divider <= 255:
        raise ValueError("--sample-divider must be in range 1..255.")
    if args.duration <= 0:
        raise ValueError("--duration must be positive.")
    if not 0 < args.target_fraction <= 1.0:
        raise ValueError("--target-fraction must be in (0, 1].")
    if not 0 < args.power_fraction < 1.0:
        raise ValueError("--power-fraction must be in (0, 1).")
    if not 0 < args.window_lo_fraction < args.window_hi_fraction < 1.0:
        raise ValueError(
            "Window fractions must satisfy 0 < lo < hi < 1."
        )
    output = resolve_output(paths.root, args.output)
    output.mkdir(parents=True, exist_ok=True)

    with HardwareSession(paths) as session:
        initial = session.preflight()
        i_nom_a = session.read_engineering("drv_params.I_nom")
        speed_nom_rpm = session.read_engineering("drv_params.speed_nom")
        iq_current_limit_a = abs(session.read_engineering("refs.Iq_ref", unit="A"))
        if min(i_nom_a, speed_nom_rpm, iq_current_limit_a) <= 0:
            raise RuntimeError(
                "Positive I_nom, speed_nom and refs.Iq_ref are required."
            )
        hardware = session.hardware()

        target_speed_rpm = args.target_fraction * speed_nom_rpm
        power_set_pu = (
            args.power_fraction
            * (iq_current_limit_a / i_nom_a)
            * (target_speed_rpm / speed_nom_rpm)
        )
        speed_lo_rpm = args.window_lo_fraction * target_speed_rpm
        speed_hi_rpm = args.window_hi_fraction * target_speed_rpm
        ramp_time_s = 0.02

        capture_power = _capture(
            session,
            speed_ref_rpm=target_speed_rpm,
            ramp_time_s=ramp_time_s,
            power_enabled=True,
            power_set_pu=power_set_pu,
            divider=args.sample_divider,
            duration_s=args.duration,
            channel_limit=iq_current_limit_a,
        )
        metrics_power = analyze_constant_power(
            capture_power.samples,
            mode="power",
            speed_nom_rpm=speed_nom_rpm,
            i_nom_a=i_nom_a,
            iq_current_limit_a=iq_current_limit_a,
            power_set_pu=power_set_pu,
            speed_lo_rpm=speed_lo_rpm,
            speed_hi_rpm=speed_hi_rpm,
            max_power_cv_percent=args.max_power_cv_percent,
            power_match_percent=args.power_match_percent,
            min_speed_span_rpm=args.min_speed_span_rpm,
        )

        capture_torque = _capture(
            session,
            speed_ref_rpm=target_speed_rpm,
            ramp_time_s=ramp_time_s,
            power_enabled=False,
            power_set_pu=power_set_pu,
            divider=args.sample_divider,
            duration_s=args.duration,
            channel_limit=iq_current_limit_a,
        )
        metrics_torque = analyze_constant_power(
            capture_torque.samples,
            mode="torque",
            speed_nom_rpm=speed_nom_rpm,
            i_nom_a=i_nom_a,
            iq_current_limit_a=iq_current_limit_a,
            power_set_pu=power_set_pu,
            speed_lo_rpm=speed_lo_rpm,
            speed_hi_rpm=speed_hi_rpm,
            max_power_cv_percent=args.max_power_cv_percent,
            power_match_percent=args.power_match_percent,
            min_speed_span_rpm=args.min_speed_span_rpm,
        )

    criteria = dict(metrics_power["criteria"])
    criteria["a_b_torque_mode_verified"] = metrics_torque["passed"]
    passed = metrics_power["passed"] and metrics_torque["passed"]

    csv_path = output / "capture.csv"
    png_path = output / "power-transition.png"
    json_path = output / "report.json"
    write_csv(csv_path, capture_power.samples)
    plot_constant_power(
        png_path,
        capture_power.samples,
        speed_nom_rpm=speed_nom_rpm,
        i_nom_a=i_nom_a,
        power_set_pu=power_set_pu,
    )
    report = {
        "test": NAME,
        "firmware": firmware_metadata(paths),
        "hardware": hardware,
        "setup": {
            "required_run_mode": 5,
            "i_nom_a": i_nom_a,
            "speed_nom_rpm": speed_nom_rpm,
            "iq_current_limit_a": iq_current_limit_a,
            "target_speed_rpm": target_speed_rpm,
            "power_set_pu": power_set_pu,
            "power_limit_raw": round(power_set_pu * IQ24),
            "speed_window_rpm": [speed_lo_rpm, speed_hi_rpm],
            "ramp_time_s": ramp_time_s,
            "sample_divider": args.sample_divider,
            "duration_s": args.duration,
        },
        "snapshots": {
            "initial": snapshot_dict(initial),
            "power_before": snapshot_dict(capture_power.before),
            "power_runtime": snapshot_dict(capture_power.runtime),
            "power_after": snapshot_dict(capture_power.after),
            "torque_before": snapshot_dict(capture_torque.before),
            "torque_runtime": snapshot_dict(capture_torque.runtime),
            "torque_after": snapshot_dict(capture_torque.after),
        },
        "metrics": {"power_limited": metrics_power, "torque_limit": metrics_torque},
        "criteria": criteria,
        "passed": passed,
        "artifacts": {"csv": csv_path.name, "plot": png_path.name},
    }
    write_json(json_path, report)
    print(json.dumps(metrics_power, ensure_ascii=False, indent=2))
    print(json.dumps(metrics_torque, ensure_ascii=False, indent=2))
    print(f"JSON: {json_path}")
    print(f"CSV: {csv_path}")
    print(f"PNG: {png_path}")
    if not passed:
        raise RuntimeError("Constant-power acceptance criteria failed.")
