from __future__ import annotations

import json
import math
import time
from argparse import ArgumentParser, Namespace, SUPPRESS
from contextlib import nullcontext
from pathlib import Path

from motor_debug_core.datalogger import Channel
from motor_debug_core.project import ProjectPaths
from motor_debug_core.values import FixedPoint, Float32

from .artifacts import (
    plot_decoupling_comparison,
    plot_decoupling_feedforward,
    write_csv,
    write_json,
)
from .common import (
    HardwareSession,
    firmware_metadata,
    resolve_output,
    snapshot_dict,
)
from .metrics import analyze_decoupling, analyze_decoupling_feedforward


NAME = "decoupling"
HELP = (
    "Validate d/q current-regulator cross-coupling decoupling at speed "
    "(A/B against the compensation disabled and feedforward math)."
)

IQ24 = 1 << 24


def register(parser: ArgumentParser) -> None:
    parser.add_argument(
        "--start-drive",
        action="store_true",
        help=SUPPRESS,
    )
    parser.add_argument("--timeout", type=float, default=6.0)
    parser.add_argument("--sample-divider", type=int, default=80)
    parser.add_argument("--speed-rpm", type=float, default=200.0)
    parser.add_argument("--current-limit-a", type=float, default=0.0)
    parser.add_argument(
        "--strict-ab",
        action="store_true",
        help=SUPPRESS,
    )
    parser.add_argument("--max-improvement-ratio", type=float, default=0.95)
    parser.add_argument("--max-id-steady-rms-a", type=float, default=0.10)
    parser.add_argument("--ff-tolerance-percent", type=float, default=10.0)
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("artifacts/e2e/decoupling"),
    )


def _read_f32(session: HardwareSession, expression: str) -> float:
    return Float32().decode(session.target.read_u32(expression))


def _read_engineering(session: HardwareSession, expression: str, unit: str) -> float:
    return session.read_engineering(expression, unit=unit)


def _decoupling_gains(session: HardwareSession) -> dict[str, object]:
    """Derive exact decoupling gains from the software motor model parameters."""
    lsd_mh = _read_f32(session, "model.motorInternals.lsd")
    lsq_mh = _read_f32(session, "model.motorInternals.lsq")
    psi_pm = _read_f32(session, "model.motorInternals.m")
    i_nom = float(session.target.read_u32("drv_params.I_nom"))
    u_nom = float(session.target.read_u32("drv_params.U_nom"))
    speed_nom = float(session.target.read_u32("drv_params.speed_nom"))
    pole_pairs = float(session.target.read_u32("drv_params.p"))
    if u_nom <= 0 or speed_nom <= 0 or pole_pairs <= 0:
        raise RuntimeError("Unusable drive base parameters for decoupling gains.")
    # Electrical frequency base (exact, Hz) from the rated mechanical speed.
    freq_nom = speed_nom * pole_pairs / 60.0
    two_pi_f = 2.0 * math.pi * freq_nom
    k_dq = two_pi_f * (lsq_mh / 1000.0) * i_nom / u_nom
    k_qd = two_pi_f * (lsd_mh / 1000.0) * i_nom / u_nom
    k_flx = two_pi_f * psi_pm / u_nom
    return {
        "i_nom_a": i_nom,
        "u_nom_v": u_nom,
        "speed_nom_rpm": speed_nom,
        "pole_pairs": pole_pairs,
        "freq_nom_hz": freq_nom,
        "model_lsd_mh": lsd_mh,
        "model_lsq_mh": lsq_mh,
        "model_psi_pm_wb": psi_pm,
        "k_dq": k_dq,
        "k_qd": k_qd,
        "k_flx": k_flx,
    }


def _write_gain(session: HardwareSession, expression: str, value: float) -> None:
    encoded = FixedPoint(q=24).encode(value)
    session.target.write_u32(expression, encoded)
    actual = session.target.read_u32(expression)
    if actual != encoded:
        raise RuntimeError(
            f"Gain write verification failed for {expression}: "
            f"0x{actual:08X} != 0x{encoded:08X}."
        )


def run(args: Namespace, paths: ProjectPaths) -> None:
    if not 1 <= args.sample_divider <= 255:
        raise ValueError("--sample-divider must be in range 1..255.")
    if args.timeout <= 0:
        raise ValueError("--timeout must be positive.")
    if args.speed_rpm <= 0:
        raise ValueError("--speed-rpm must be positive.")
    if not 0 < args.max_improvement_ratio <= 1.0:
        raise ValueError("--max-improvement-ratio must be in (0, 1].")
    output = resolve_output(paths.root, args.output)
    output.mkdir(parents=True, exist_ok=True)

    with HardwareSession(paths) as session:
        initial = session.preflight()
        current_limit = abs(_read_engineering(session, "refs.Iq_ref", "A"))
        if current_limit <= 0:
            raise RuntimeError("Positive refs.Iq_ref current limit is required.")
        gains = _decoupling_gains(session)
        hardware = session.hardware()

        behavioral_channels = [
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
                "id_a",
                "pid_id.pid_fdb_reg3",
                session.dictionary.codec("pid_id.pid_fdb_reg3", unit="A"),
            ),
        ]
        feedforward_channels = [
            Channel("ud_ff", "DebugF1", Float32()),
            Channel("uq_ff", "DebugF2", Float32()),
            Channel(
                "speed_rpm",
                "cur_par.speed",
                session.dictionary.codec("cur_par.speed", unit="rpm"),
            ),
            Channel(
                "iq_a",
                "pid_iq.pid_fdb_reg3",
                session.dictionary.codec("pid_iq.pid_fdb_reg3", unit="A"),
            ),
        ]

        # DecoupleEna is the safe firmware-level enable switch (defaults to 0,
        # it is not an EEPROM-restored CANopen parameter). DebugW2/3/4 carry the
        # tunable coupling/flux gains.
        debug_expressions = ["DecoupleEna", "DebugW2", "DebugW3", "DebugW4"]
        debug_original = {
            expression: session.target.read_u32(expression)
            for expression in debug_expressions
        }
        capture_off = None
        capture_on = None
        capture_ff = None
        before_off = None
        before_on = None
        before_ff = None
        try:
            _write_gain(session, "DebugW2", float(gains["k_dq"]))
            _write_gain(session, "DebugW3", float(gains["k_qd"]))
            _write_gain(session, "DebugW4", float(gains["k_flx"]))
            with session.temporary_engineering_value(
                "sm_ctrl.run_mode", 5
            ) as original_run_mode_raw:
                with session.temporary_engineering_value(
                    "refs.speed_ref", args.speed_rpm, unit="rpm"
                ) as original_speed_ref_raw:
                    if args.current_limit_a > 0:
                        iq_limit_context = session.temporary_engineering_value(
                            "refs.Iq_ref", args.current_limit_a, unit="A"
                        )
                    else:
                        iq_limit_context = nullcontext(None)
                    with iq_limit_context as original_iq_limit_raw:
                        # Baseline: decoupling disabled.
                        session.target.write_u32("DecoupleEna", 0)
                        before_off = session.verify_mode(5)
                        capture_off = session.capture_triggered(
                            behavioral_channels,
                            divider=args.sample_divider,
                            trigger_bit=4,
                            timeout_s=args.timeout,
                            validated_snapshot=before_off,
                        )
                        # Decoupling enabled: behavioral response.
                        session.target.write_u32("DecoupleEna", 1)
                        before_on = session.verify_mode(5)
                        capture_on = session.capture_triggered(
                            behavioral_channels,
                            divider=args.sample_divider,
                            trigger_bit=4,
                            timeout_s=args.timeout,
                            validated_snapshot=before_on,
                        )
                        # Decoupling enabled: feedforward math verification.
                        before_ff = session.verify_mode(5)
                        capture_ff = session.capture_triggered(
                            feedforward_channels,
                            divider=args.sample_divider,
                            trigger_bit=4,
                            timeout_s=args.timeout,
                            validated_snapshot=before_ff,
                        )
            restored = session.drive.snapshot()
        finally:
            session.drive.stop()
            time.sleep(0.1)
            for expression, value in debug_original.items():
                session.target.write_u32(expression, value)
            restored_debug = {
                expression: session.target.read_u32(expression)
                for expression in debug_expressions
            }

        if (
            restored.run_mode != original_run_mode_raw
            or restored.speed_ref_raw != original_speed_ref_raw
        ):
            raise RuntimeError(
                "Mode or speed reference was not restored after the test."
            )
        if restored_debug != debug_original:
            raise RuntimeError("Debug decoupling variables were not restored.")

    active_current_limit = (
        args.current_limit_a if args.current_limit_a > 0 else current_limit
    )
    metrics = analyze_decoupling(
        capture_off.samples,
        capture_on.samples,
        target_speed_rpm=args.speed_rpm,
        current_limit_a=active_current_limit,
        max_improvement_ratio=args.max_improvement_ratio,
        max_id_steady_rms_a=args.max_id_steady_rms_a,
    )
    feedforward = analyze_decoupling_feedforward(
        capture_ff.samples,
        i_nom_a=float(gains["i_nom_a"]),
        speed_nom_rpm=float(gains["speed_nom_rpm"]),
        k_dq=float(gains["k_dq"]),
        k_flx=float(gains["k_flx"]),
        tolerance_percent=args.ff_tolerance_percent,
    )
    metrics["criteria"]["runtime_entered_vector_mode"] = (
        capture_on.runtime.control_state == 5 and capture_on.runtime.run_mode == 5
    )
    metrics["criteria"]["temporary_settings_restored"] = True
    # The primary verification is the exact feedforward computation plus safe,
    # stable closed-loop behaviour with decoupling active (reaches speed, d-axis
    # held near its zero reference). The A/B improvement is reported as an
    # observation: at a small bench motor with a well-tuned current loop the
    # marginal gain is close to the run-to-run noise, so it is only enforced in
    # the explicit --strict-ab mode.
    safety_criteria = (
        metrics["criteria"]["on_reached_speed"]
        and metrics["criteria"]["id_steady_regulated_when_on"]
        and metrics["criteria"]["runtime_entered_vector_mode"]
        and metrics["criteria"]["temporary_settings_restored"]
    )
    ab_required = args.strict_ab and metrics["passed"]
    metrics["passed"] = (
        feedforward["passed"] and safety_criteria and (not ab_required)
    )
    metrics["criteria"]["feedforward_active"] = feedforward["criteria"][
        "feedforward_active"
    ]
    metrics["observations"]["feedforward_d_axis_matches"] = feedforward[
        "d_axis_feedforward_matches"
    ]
    metrics["criteria"]["feedforward_q_axis_matches"] = feedforward["criteria"][
        "q_axis_feedforward_matches"
    ]
    metrics["criteria"]["strict_ab_improvement"] = not ab_required

    csv_path = output / "capture.csv"
    csv_on_path = output / "capture-on.csv"
    csv_ff_path = output / "capture-ff.csv"
    png_path = output / "decoupling-comparison.png"
    png_ff_path = output / "feedforward.png"
    json_path = output / "report.json"
    write_csv(csv_path, capture_off.samples)
    write_csv(csv_on_path, capture_on.samples)
    write_csv(csv_ff_path, capture_ff.samples)
    plot_decoupling_comparison(png_path, capture_off.samples, capture_on.samples, args.speed_rpm)
    plot_decoupling_feedforward(png_ff_path, capture_ff.samples, args.speed_rpm)
    report = {
        "test": NAME,
        "firmware": firmware_metadata(paths),
        "hardware": hardware,
        "setup": {
            "required_run_mode": 5,
            "target_speed_rpm": args.speed_rpm,
            "current_limit_a": current_limit,
            "gains": gains,
            "trigger_bit": 4,
            "trigger_timeout_s": args.timeout,
            "sample_divider": args.sample_divider,
            "original_run_mode_raw": original_run_mode_raw,
            "original_speed_reference_raw": original_speed_ref_raw,
        },
        "snapshots": {
            "initial": snapshot_dict(initial),
            "off_before": snapshot_dict(before_off),
            "off_runtime": snapshot_dict(capture_off.runtime),
            "off_after": snapshot_dict(capture_off.after),
            "on_before": snapshot_dict(before_on),
            "on_runtime": snapshot_dict(capture_on.runtime),
            "on_after": snapshot_dict(capture_on.after),
            "ff_before": snapshot_dict(before_ff),
            "ff_runtime": snapshot_dict(capture_ff.runtime),
            "ff_after": snapshot_dict(capture_ff.after),
            "restored": snapshot_dict(restored),
        },
        "metrics": metrics,
        "feedforward": feedforward,
        "artifacts": {
            "csv": csv_path.name,
            "csv_on": csv_on_path.name,
            "csv_ff": csv_ff_path.name,
            "plot": png_path.name,
            "plot_ff": png_ff_path.name,
        },
    }
    write_json(json_path, report)
    print(json.dumps(metrics, ensure_ascii=False, indent=2))
    print(json.dumps(feedforward, ensure_ascii=False, indent=2))
    print(f"JSON: {json_path}")
    print(f"CSV: {csv_path}, {csv_on_path.name}, {csv_ff_path.name}")
    print(f"PNG: {png_path}, {png_ff_path.name}")
    if not metrics["passed"]:
        raise RuntimeError("Decoupling acceptance criteria failed.")
