from __future__ import annotations

import json
import time
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
from .metrics import analyze_field_weakening, compare_field_weakening_speed


NAME = "field-weakening"
HELP = (
    "A/B comparison of the automatic field-weakening regulator above base "
    "speed in vector mode 5: verifies engagement, safety, transparency and "
    "that negative Id extends the reachable speed range."
)
TARGET_SPEED_RPM = 1500.0

#Per-unit IQ24 codec for internal regulator quantities that share the PWM
#per-unit voltage domain (1.0 = full phase amplitude). These symbols are not
#in the engineering-scale region of cood.xml, so the codec is set explicitly.
_PU = FixedPoint(q=24, scale=1.0, unit="p.u.")


def register(parser: ArgumentParser) -> None:
    parser.add_argument(
        "--start-drive",
        action="store_true",
        help=SUPPRESS,
    )
    parser.add_argument("--target-speed-rpm", type=float, default=TARGET_SPEED_RPM)
    parser.add_argument("--duration-s", type=float, default=4.0)
    parser.add_argument("--sample-divider", type=int, default=80)
    parser.add_argument("--poll-period-s", type=float, default=0.3)
    parser.add_argument("--ki-override", type=float, default=0.0)
    parser.add_argument("--settle-s", type=float, default=0.4)
    parser.add_argument("--min-improvement-percent", type=float, default=1.0)
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("artifacts/e2e/field-weakening"),
    )


def _read_pu(session: HardwareSession, expression: str) -> float:
    return _PU.decode(session.target.read_u32(expression))


def _build_channels(session: HardwareSession) -> list[Channel]:
    id_ref_codec = session.dictionary.codec("pid_id.pid_ref_reg3", unit="A")
    return [
        Channel(
            "speed_rpm",
            "cur_par.speed",
            session.dictionary.codec("cur_par.speed", unit="rpm"),
        ),
        Channel(
            "id_ref_a",
            "pid_id.pid_ref_reg3",
            id_ref_codec,
        ),
        Channel(
            "id_a",
            "pid_id.pid_fdb_reg3",
            session.dictionary.codec("pid_id.pid_fdb_reg3", unit="A"),
        ),
        Channel(
            "u_mag_pu",
            "pwm.U_mag",
            _PU,
        ),
    ]


def _set_fw_enable(session: HardwareSession, enabled: bool) -> None:
    #DebugW3 is IQ24, scale 1.0: any non-zero value enables the regulator.
    session.target.write_u32("DebugW3", _PU.encode(1.0 if enabled else 0.0))
    #Allow the background slow-calc task to copy the knob into the regulator.
    time.sleep(0.2)
    if (session.target.read_u32("DebugW3") != 0) != enabled:
        raise RuntimeError(
            f"Failed to set field-weakening enable to {enabled} via DebugW3."
        )


def run(args: Namespace, paths: ProjectPaths) -> None:
    if not 1 <= args.sample_divider <= 255:
        raise ValueError("--sample-divider must be in range 1..255.")
    if args.duration_s <= 0:
        raise ValueError("--duration-s must be positive.")
    if not 0.0 < args.poll_period_s < 10.0:
        raise ValueError("--poll-period-s must be in range 0..10 s.")
    if args.target_speed_rpm == 0:
        raise ValueError("--target-speed-rpm must be non-zero.")
    if min(args.duration_s, args.sample_divider, args.poll_period_s) < 0:
        raise ValueError("Capture parameters must not be negative.")
    if args.ki_override < 0:
        raise ValueError("--ki-override must not be negative.")
    output = resolve_output(paths.root, args.output)
    output.mkdir(parents=True, exist_ok=True)

    with HardwareSession(paths) as session:
        initial = session.preflight()
        current_limit = abs(session.read_engineering("refs.Iq_ref", unit="A"))
        if current_limit <= 0:
            raise RuntimeError("Positive refs.Iq_ref current limit is required.")
        u_ref_pu = _read_pu(session, "fieldWeakening.U_ref")
        u_lim_pu = _read_pu(session, "pwm.U_lim")
        id_min_pu = _read_pu(session, "fieldWeakening.Id_min")
        ki_default_pu = _read_pu(session, "fieldWeakening.Ki")
        id_scale_a = session.dictionary.codec(
            "pid_id.pid_ref_reg3", unit="A"
        ).scale
        hardware = session.hardware()
        channels = _build_channels(session)

        debug_w3_original = session.target.read_u32("DebugW3")
        debug_w4_original = session.target.read_u32("DebugW4")
        try:
            if args.ki_override > 0:
                session.target.write_u32(
                    "DebugW4", _PU.encode(args.ki_override)
                )
                time.sleep(0.2)
            fw_ki_pu = _read_pu(session, "fieldWeakening.Ki")

            with session.temporary_engineering_value(
                "sm_ctrl.run_mode", 5
            ) as original_run_mode_raw:
                with session.temporary_engineering_value(
                    "refs.speed_ref", args.target_speed_rpm, unit="rpm"
                ) as original_speed_ref_raw:
                    #Run A: field weakening disabled (legacy Id = 0 behaviour).
                    _set_fw_enable(session, False)
                    before_off = session.verify_mode(5)
                    capture_off = session.capture_ring(
                        channels,
                        divider=args.sample_divider,
                        duration_s=args.duration_s,
                        poll_period_s=args.poll_period_s,
                        validated_snapshot=before_off,
                    )
                    session.stop_and_wait(delay_s=args.settle_s)

                    #Run B: field weakening enabled (automatic negative Id).
                    _set_fw_enable(session, True)
                    before_on = session.verify_mode(5)
                    capture_on = session.capture_ring(
                        channels,
                        divider=args.sample_divider,
                        duration_s=args.duration_s,
                        poll_period_s=args.poll_period_s,
                        validated_snapshot=before_on,
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
            session.drive.stop()
            time.sleep(0.1)
            session.target.write_u32("DebugW3", debug_w3_original)
            session.target.write_u32("DebugW4", debug_w4_original)
            time.sleep(0.1)
            for expression, original in (
                ("DebugW3", debug_w3_original),
                ("DebugW4", debug_w4_original),
            ):
                actual = session.target.read_u32(expression)
                if actual != original:
                    raise RuntimeError(
                        f"RAM restoration failed for {expression}: "
                        f"0x{actual:08X} != 0x{original:08X}."
                    )

    metrics_on = analyze_field_weakening(
        capture_on.samples,
        target_speed_rpm=args.target_speed_rpm,
        u_ref_pu=u_ref_pu,
        u_lim_pu=u_lim_pu,
        id_min_pu=id_min_pu,
        id_scale_a=id_scale_a,
    )
    #Transparency of run A (FW off): Id reference must stay at zero, i.e. the
    #legacy behaviour is reproduced exactly when the regulator is disabled.
    id_ref_off_pu = [
        s.values["id_ref_a"] / id_scale_a for s in capture_off.samples
    ]
    u_mag_off_pu = [s.values["u_mag_pu"] for s in capture_off.samples]
    speed_off_pu = [s.values["speed_rpm"] for s in capture_off.samples]
    transparency = {
        "id_ref_max_off_pu": max(id_ref_off_pu),
        "id_ref_min_off_pu": min(id_ref_off_pu),
        "u_mag_max_off_pu": max(u_mag_off_pu),
        "steady_speed_off_rpm": _tail_mean(speed_off_pu),
    }
    comparison = compare_field_weakening_speed(
        capture_off.samples,
        capture_on.samples,
        min_improvement_percent=args.min_improvement_percent,
    )

    criteria = dict(metrics_on["criteria"])
    criteria["transparency_id_zero_when_disabled"] = (
        transparency["id_ref_max_off_pu"] <= 0.02
        and transparency["id_ref_min_off_pu"] >= -0.02
    )
    criteria["field_weakening_extends_speed"] = comparison["criteria"][
        "field_weaking_extends_speed"
    ]
    criteria["runtime_entered_vector_mode"] = (
        capture_on.runtime.control_state == 5 and capture_on.runtime.run_mode == 5
    )
    criteria["speed_reference_restored"] = (
        restored.speed_ref_raw == original_speed_ref_raw
    )
    criteria["run_mode_restored"] = restored.run_mode == original_run_mode_raw
    #Debug-knob restoration is asserted inside the finally block above.
    criteria["debug_knobs_restored"] = True

    metrics = {
        "samples_on": len(capture_on.samples),
        "samples_off": len(capture_off.samples),
        "window_on_s": (
            capture_on.samples[-1].time_s - capture_on.samples[0].time_s
            if capture_on.samples
            else 0.0
        ),
        "window_off_s": (
            capture_off.samples[-1].time_s - capture_off.samples[0].time_s
            if capture_off.samples
            else 0.0
        ),
        "regulator_on": metrics_on,
        "regulator_off": transparency,
        "speed_comparison": comparison,
        "criteria": criteria,
    }
    metrics["passed"] = all(criteria.values())

    csv_on_path = output / "capture_on.csv"
    csv_off_path = output / "capture_off.csv"
    png_path = output / "field-weakening.png"
    json_path = output / "report.json"
    write_csv(csv_on_path, capture_on.samples)
    write_csv(csv_off_path, capture_off.samples)
    _plot_comparison(
        png_path, capture_off.samples, capture_on.samples, args.target_speed_rpm
    )
    report = {
        "test": NAME,
        "firmware": firmware_metadata(paths),
        "hardware": hardware,
        "setup": {
            "required_run_mode": 5,
            "target_speed_rpm": args.target_speed_rpm,
            "current_limit_a": current_limit,
            "field_weakening_ki_pu": fw_ki_pu,
            "field_weakening_ki_default_pu": ki_default_pu,
            "u_ref_pu": u_ref_pu,
            "u_lim_pu": u_lim_pu,
            "id_min_pu": id_min_pu,
            "id_scale_a": id_scale_a,
            "duration_s": args.duration_s,
            "sample_divider": args.sample_divider,
            "poll_period_s": args.poll_period_s,
            "settle_s": args.settle_s,
            "ki_override_pu": args.ki_override,
            "min_improvement_percent": args.min_improvement_percent,
            "original_run_mode_raw": original_run_mode_raw,
            "original_speed_reference_raw": original_speed_ref_raw,
            "original_debug_w3_raw": debug_w3_original,
            "original_debug_w4_raw": debug_w4_original,
        },
        "snapshots": {
            "initial": snapshot_dict(initial),
            "before_off": snapshot_dict(capture_off.before),
            "runtime_off": snapshot_dict(capture_off.runtime),
            "before_on": snapshot_dict(capture_on.before),
            "runtime_on": snapshot_dict(capture_on.runtime),
            "restored": snapshot_dict(restored),
        },
        "metrics": metrics,
        "artifacts": {
            "csv_on": csv_on_path.name,
            "csv_off": csv_off_path.name,
            "plot": png_path.name,
        },
    }
    write_json(json_path, report)
    print(json.dumps(metrics, ensure_ascii=False, indent=2))
    print(f"JSON: {json_path}")
    print(f"CSV (on): {csv_on_path}")
    print(f"CSV (off): {csv_off_path}")
    print(f"PNG: {png_path}")
    if not metrics["passed"]:
        raise RuntimeError("Field-weakening acceptance criteria failed.")


def _tail_mean(values: list[float], fraction: float = 0.2) -> float:
    if not values:
        return 0.0
    start = max(0, len(values) - max(10, round(len(values) * fraction)))
    return sum(values[start:]) / len(values[start:])


def _plot_comparison(
    path: Path,
    samples_off: list,
    samples_on: list,
    target_speed_rpm: float,
) -> None:
    import matplotlib.pyplot as plt

    plt.style.use("seaborn-v0_8-whitegrid")
    figure, axes = plt.subplots(
        3, 1, figsize=(11, 9.0), constrained_layout=True
    )
    speed_axis, current_axis, voltage_axis = axes
    for label, samples, color in (
        ("FW off", samples_off, "tab:gray"),
        ("FW on", samples_on, "tab:blue"),
    ):
        t = [s.time_s for s in samples]
        speed_axis.plot(
            t, [s.values["speed_rpm"] for s in samples], label=label, color=color
        )
        current_axis.plot(
            t,
            [s.values["id_ref_a"] for s in samples],
            label=f"Id ref ({label})",
            color=color,
        )
        voltage_axis.plot(
            t,
            [s.values["u_mag_pu"] for s in samples],
            color=color,
            label=f"U_mag ({label})",
        )
    speed_axis.axhline(target_speed_rpm, color="red", linestyle="--", linewidth=1,
                       label="command")
    speed_axis.set_title("Field weakening A/B: speed, Id reference, U_mag")
    speed_axis.set_ylabel("Speed, rpm")
    speed_axis.legend()
    current_axis.axhline(0.0, color="black", linestyle=":", linewidth=1)
    current_axis.set_ylabel("d-axis current ref, A")
    current_axis.legend()
    voltage_axis.set_xlabel("Time, s")
    voltage_axis.set_ylabel("Voltage magnitude, p.u.")
    voltage_axis.legend()
    figure.savefig(path, dpi=170)
    plt.close(figure)
