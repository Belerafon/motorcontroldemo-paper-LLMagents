from __future__ import annotations

import json
import math
import time
from argparse import ArgumentParser, Namespace, SUPPRESS
from pathlib import Path

from motor_debug_core.datalogger import Channel
from motor_debug_core.drive import SafetySnapshot
from motor_debug_core.project import ProjectPaths

from .artifacts import plot_current_mode4, write_csv, write_json
from .common import (
    HardwareSession,
    firmware_metadata,
    resolve_output,
    snapshot_dict,
)
from .metrics import analyze_current_mode4


NAME = "current-mode4"
HELP = "Pre-align the rotor, then validate mode 4 current loops at zero speed."


def register(parser: ArgumentParser) -> None:
    parser.add_argument(
        "--start-drive",
        action="store_true",
        help=SUPPRESS,
    )
    parser.add_argument("--timeout", type=float, default=2.0)
    parser.add_argument("--sample-divider", type=int, default=4)
    parser.add_argument("--max-id-error-percent", type=float, default=5.0)
    parser.add_argument("--max-id-error-a", type=float, default=0.05)
    parser.add_argument("--max-iq-rms-a", type=float, default=0.10)
    parser.add_argument("--max-iq-bias-a", type=float, default=0.05)
    parser.add_argument("--max-overshoot-percent", type=float, default=20.0)
    parser.add_argument("--max-settling-s", type=float, default=0.05)
    parser.add_argument("--max-rotor-speed-rpm", type=float, default=5.0)
    parser.add_argument("--min-test-current-a", type=float, default=0.10)
    parser.add_argument("--alignment-current-a", type=float, default=0.20)
    parser.add_argument("--alignment-min-s", type=float, default=0.30)
    parser.add_argument("--alignment-timeout-s", type=float, default=10.0)
    parser.add_argument("--alignment-settle-s", type=float, default=0.15)
    parser.add_argument("--alignment-max-speed-rpm", type=float, default=1.0)
    parser.add_argument("--post-alignment-stop-s", type=float, default=0.05)
    parser.add_argument(
        "--pwm-type",
        type=int,
        choices=(0, 1, 6),
        default=None,
        help="Temporarily select motor PWM type in RAM (6: centered SVPWM).",
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("artifacts/e2e/current-mode4"),
    )


def _align_rotor(
    session: HardwareSession,
    args: Namespace,
    validated_snapshot: SafetySnapshot,
    speed_expression: str,
) -> tuple[dict[str, object], SafetySnapshot, SafetySnapshot]:
    speed_codec = session.dictionary.codec(speed_expression, unit="rpm")
    observations: list[dict[str, float]] = []
    started = time.monotonic()
    stable_since: float | None = None
    stable_duration = 0.0
    next_progress_s = 0.25
    runtime = validated_snapshot
    stopped: SafetySnapshot | None = None
    try:
        session.drive.start(validated_snapshot=validated_snapshot)
        while True:
            now = time.monotonic()
            elapsed = now - started
            rotor_speed = speed_codec.decode(
                session.target.read_u32(speed_expression)
            )
            runtime = session.drive.snapshot()
            if (
                runtime.protection_state != 1
                or runtime.fault1
                or runtime.fault2
            ):
                raise RuntimeError(
                    "Protection changed during rotor alignment: "
                    + runtime.describe()
                )
            observations.append(
                {
                    "time_s": elapsed,
                    "rotor_speed_rpm": rotor_speed,
                }
            )
            if elapsed >= next_progress_s:
                print(
                    f"Alignment: t={elapsed:.2f} s, "
                    f"rotor={rotor_speed:+.2f} rpm"
                )
                next_progress_s += 0.25
            if (
                elapsed >= args.alignment_min_s
                and abs(rotor_speed) <= args.alignment_max_speed_rpm
            ):
                if stable_since is None:
                    stable_since = now
                if now - stable_since >= args.alignment_settle_s:
                    stable_duration = now - stable_since
                    break
            else:
                stable_since = None
            if elapsed >= args.alignment_timeout_s:
                raise RuntimeError(
                    "Rotor did not settle during mode 2 alignment: "
                    f"|speed| remained above "
                    f"{args.alignment_max_speed_rpm:g} rpm for "
                    f"{args.alignment_timeout_s:g} s."
                )
            time.sleep(0.02)
    finally:
        stopped = session.stop_and_wait()

    assert stopped is not None
    stopped_speed = speed_codec.decode(session.target.read_u32(speed_expression))
    peak_speed = max(abs(item["rotor_speed_rpm"]) for item in observations)
    return (
        {
            "mode": 2,
            "duration_s": observations[-1]["time_s"],
            "sample_count": len(observations),
            "peak_abs_rotor_speed_rpm": peak_speed,
            "final_running_speed_rpm": observations[-1]["rotor_speed_rpm"],
            "speed_after_stop_rpm": stopped_speed,
            "stable_duration_s": stable_duration,
            "observations": observations,
        },
        runtime,
        stopped,
    )


def _speed_feedback(session: HardwareSession) -> dict[str, object]:
    sensor_type = session.target.read_u32("drv_params.sens_type") & 0x7
    feedback_by_sensor = {
        1: ("encoder", "posspeedEqep.speed_elec"),
        2: ("hall", "DPReCAP.speed"),
        3: ("encoder+hall", "posspeedEqep.speed_elec"),
        4: ("ssi", "SSI_Encoder.speed_elec"),
    }
    try:
        sensor_name, expression = feedback_by_sensor[sensor_type]
    except KeyError as error:
        raise RuntimeError(
            "Mode 4 rotor alignment requires configured position feedback "
            "(encoder, Hall, encoder+Hall or SSI); "
            f"drv_params.sens_type={sensor_type}."
        ) from error
    session.dictionary.codec(expression, unit="rpm")
    return {
        "sensor_type": sensor_type,
        "sensor_name": sensor_name,
        "expression": expression,
    }


def run(args: Namespace, paths: ProjectPaths) -> None:
    if not 1 <= args.sample_divider <= 255:
        raise ValueError("--sample-divider must be in range 1..255.")
    if args.timeout <= 0:
        raise ValueError("--timeout must be positive.")
    if min(
        args.max_id_error_percent,
        args.max_id_error_a,
        args.max_iq_rms_a,
        args.max_iq_bias_a,
        args.max_overshoot_percent,
        args.max_settling_s,
        args.max_rotor_speed_rpm,
        args.min_test_current_a,
        args.alignment_current_a,
        args.alignment_min_s,
        args.alignment_timeout_s,
        args.alignment_settle_s,
        args.alignment_max_speed_rpm,
        args.post_alignment_stop_s,
    ) < 0:
        raise ValueError("Acceptance limits must not be negative.")
    if args.alignment_current_a == 0:
        raise ValueError("--alignment-current-a must be positive.")
    if args.alignment_timeout_s <= 0:
        raise ValueError("--alignment-timeout-s must be positive.")
    if (
        args.alignment_min_s + args.alignment_settle_s
        > args.alignment_timeout_s
    ):
        raise ValueError(
            "Alignment timeout must cover minimum and stable-settle times."
        )
    output = resolve_output(paths.root, args.output)
    output.mkdir(parents=True, exist_ok=True)

    with HardwareSession(paths) as session:
        initial = session.preflight()
        flux_ref = session.read_engineering("refs.i_flux_ref", unit="A")
        if abs(flux_ref) < args.min_test_current_a:
            raise RuntimeError(
                f"Mode 4 test requires at least {args.min_test_current_a:g} A "
                f"d-axis excitation, got {flux_ref:.3f} A."
            )
        alignment_current = math.copysign(
            min(abs(flux_ref), args.alignment_current_a),
            flux_ref,
        )

        controllers = {
            "id": session.read_controller("pid_id"),
            "iq": session.read_controller("pid_iq"),
        }
        configuration_criteria = {
            f"{axis}_kp_positive": values["kp"] > 0
            for axis, values in controllers.items()
        }
        configuration_criteria.update(
            {
                f"{axis}_ki_positive": values["ki"] > 0
                for axis, values in controllers.items()
            }
        )
        configuration_criteria.update(
            {
                f"{axis}_output_range_valid": (
                    values["output_min_v"] < 0 < values["output_max_v"]
                )
                for axis, values in controllers.items()
            }
        )
        if not all(configuration_criteria.values()):
            failed = [
                name for name, passed in configuration_criteria.items() if not passed
            ]
            raise RuntimeError(
                "Unsafe or incomplete current-controller configuration: "
                + ", ".join(failed)
            )
        hardware = session.hardware()
        speed_feedback = _speed_feedback(session)
        speed_expression = str(speed_feedback["expression"])
        with session.temporary_u16_value(
            "pwm.PWM_type", args.pwm_type
        ) as original_pwm_type:
            active_pwm_type = session.target.read_u16("pwm.PWM_type")
            with session.temporary_engineering_value(
                "refs.theta_elec", 0.0
            ) as original_theta_raw:
                with session.temporary_engineering_value(
                    "refs.speed_ref", 0.0, unit="rpm"
                ) as original_speed_ref_raw:
                    with session.temporary_engineering_value(
                        "sm_ctrl.run_mode", 2
                    ) as original_run_mode_raw:
                        with session.temporary_engineering_value(
                            "refs.i_flux_ref", alignment_current, unit="A"
                        ) as original_flux_ref_raw:
                            alignment_before = session.verify_mode(2)
                            (
                                alignment,
                                alignment_runtime,
                                alignment_after,
                            ) = _align_rotor(
                                session,
                                args,
                                alignment_before,
                                speed_expression,
                            )
                    time.sleep(args.post_alignment_stop_s)
                    with session.temporary_engineering_value(
                        "sm_ctrl.run_mode", 4
                    ) as mode4_original_run_mode_raw:
                        if mode4_original_run_mode_raw != original_run_mode_raw:
                            raise RuntimeError(
                                "Run mode changed unexpectedly between alignment "
                                "and the mode 4 capture."
                            )
                        before = session.verify_mode(4)
                        speed_ref = session.read_engineering(
                            "refs.speed_ref", unit="rpm"
                        )
                        channels = [
                            Channel(
                                "id_ref_a",
                                "pid_id.pid_ref_reg3",
                                session.dictionary.codec(
                                    "pid_id.pid_ref_reg3", unit="A"
                                ),
                            ),
                            Channel(
                                "id_a",
                                "pid_id.pid_fdb_reg3",
                                session.dictionary.codec(
                                    "pid_id.pid_fdb_reg3", unit="A"
                                ),
                            ),
                            Channel(
                                "rotor_speed_rpm",
                                speed_expression,
                                session.dictionary.codec(
                                    speed_expression, unit="rpm"
                                ),
                            ),
                            Channel(
                                "iq_a",
                                "pid_iq.pid_fdb_reg3",
                                session.dictionary.codec(
                                    "pid_iq.pid_fdb_reg3", unit="A"
                                ),
                            ),
                        ]
                        capture = session.capture_triggered(
                            channels,
                            divider=args.sample_divider,
                            trigger_bit=3,
                            timeout_s=args.timeout,
                            validated_snapshot=before,
                        )
        restored = session.drive.snapshot()
        restored_theta_raw = session.target.read_u32("refs.theta_elec")
        restored_pwm_type = session.target.read_u16("pwm.PWM_type")
        if (
            restored.run_mode != original_run_mode_raw
            or restored.speed_ref_raw != original_speed_ref_raw
            or restored.flux_ref_raw != original_flux_ref_raw
            or restored_theta_raw != original_theta_raw
            or restored_pwm_type != original_pwm_type
        ):
            raise RuntimeError("Temporary mode 4 test settings were not restored.")

    metrics = analyze_current_mode4(
        capture.samples,
        max_id_error_percent=args.max_id_error_percent,
        max_id_error_a=args.max_id_error_a,
        max_iq_rms_a=args.max_iq_rms_a,
        max_iq_bias_a=args.max_iq_bias_a,
        max_overshoot_percent=args.max_overshoot_percent,
        max_settling_s=args.max_settling_s,
        max_rotor_speed_rpm=args.max_rotor_speed_rpm,
        min_test_current_a=args.min_test_current_a,
    )
    configuration_criteria["runtime_entered_mode4"] = (
        capture.runtime.control_state == 4 and capture.runtime.run_mode == 4
    )
    configuration_criteria["temporary_settings_restored"] = (
        restored.run_mode == original_run_mode_raw
        and restored.speed_ref_raw == original_speed_ref_raw
        and restored.flux_ref_raw == original_flux_ref_raw
        and restored_theta_raw == original_theta_raw
        and restored_pwm_type == original_pwm_type
    )
    configuration_criteria["pwm_type_selected"] = (
        args.pwm_type is None or active_pwm_type == args.pwm_type
    )
    metrics["criteria"].update(configuration_criteria)
    metrics["passed"] = all(metrics["criteria"].values())

    csv_path = output / "capture.csv"
    png_path = output / "current-loops.png"
    json_path = output / "report.json"
    write_csv(csv_path, capture.samples)
    plot_current_mode4(png_path, capture.samples)
    report = {
        "test": NAME,
        "firmware": firmware_metadata(paths),
        "hardware": hardware,
        "setup": {
            "required_run_mode": 4,
            "speed_reference_rpm": speed_ref,
            "flux_reference_a": flux_ref,
            "alignment_current_a": alignment_current,
            "alignment_min_s": args.alignment_min_s,
            "alignment_timeout_s": args.alignment_timeout_s,
            "alignment_settle_s": args.alignment_settle_s,
            "alignment_max_speed_rpm": args.alignment_max_speed_rpm,
            "post_alignment_stop_s": args.post_alignment_stop_s,
            "alignment": alignment,
            "speed_feedback": speed_feedback,
            "requested_pwm_type": args.pwm_type,
            "active_pwm_type": active_pwm_type,
            "original_pwm_type": original_pwm_type,
            "original_run_mode_raw": original_run_mode_raw,
            "original_speed_reference_raw": original_speed_ref_raw,
            "original_flux_reference_raw": original_flux_ref_raw,
            "original_theta_elec_raw": original_theta_raw,
            "trigger_bit": 3,
            "trigger_timeout_s": args.timeout,
            "sample_divider": args.sample_divider,
            "controllers": controllers,
        },
        "snapshots": {
            "initial": snapshot_dict(initial),
            "alignment_before": snapshot_dict(alignment_before),
            "alignment_runtime": snapshot_dict(alignment_runtime),
            "alignment_after": snapshot_dict(alignment_after),
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
        raise RuntimeError("Mode 4 current-loop acceptance criteria failed.")
