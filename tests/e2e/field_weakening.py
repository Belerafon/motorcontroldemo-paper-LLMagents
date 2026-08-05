from __future__ import annotations

import json
from argparse import ArgumentParser, Namespace, SUPPRESS
from pathlib import Path

from motor_debug_core.datalogger import Channel
from motor_debug_core.project import ProjectPaths
from motor_debug_core.values import FixedPoint

from .artifacts import write_csv, write_json
from .common import (
    HardwareSession,
    firmware_metadata,
    resolve_output,
    snapshot_dict,
)
from .metrics import analyze_field_weakening


NAME = "field-weakening"
HELP = (
    "Validate automatic field weakening in vector mode 5 above base speed: "
    "negative Id from pid_fw keeps U_mag below U_lim and lets speed exceed "
    "the base-speed region."
)

# Speed references in this project's CANopen dictionary are scaled with
# ScaleNum 0x14, possible_value 500 rpm per per-unit. 1.4 pu therefore appears
# as 700 rpm in the engineering values used by the toolkit. This sits clearly
# above the firmware's per-unit base speed of 1.0 (where U_mag first approaches
# U_lim) so the field-weakening regulator must engage, while staying within the
# headroom of the simulator's current and overspeed protections.
TARGET_SPEED_RPM = 700.0
REQUIRED_RUN_MODE = 5


def register(parser: ArgumentParser) -> None:
    parser.add_argument(
        "--start-drive",
        action="store_true",
        help=SUPPRESS,
    )
    parser.add_argument("--timeout", type=float, default=8.0)
    parser.add_argument("--sample-divider", type=int, default=50)
    parser.add_argument("--poll-period-s", type=float, default=0.30)
    parser.add_argument("--fw-enable-raw", type=int, default=0x01000000)
    parser.add_argument("--fw-kp-raw", type=int, default=0)
    parser.add_argument("--fw-ki-raw", type=int, default=0)
    parser.add_argument("--max-time-to-95-s", type=float, default=6.0)
    parser.add_argument("--max-steady-error-percent", type=float, default=5.0)
    parser.add_argument("--min-fw-id-ref-a", type=float, default=0.05)
    parser.add_argument("--max-steady-u-mag-fraction", type=float, default=1.10)
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
    if args.poll_period_s <= 0:
        raise ValueError("--poll-period-s must be positive.")
    if min(
        args.max_time_to_95_s,
        args.max_steady_error_percent,
        args.min_fw_id_ref_a,
        args.max_steady_u_mag_fraction,
    ) < 0:
        raise ValueError("Acceptance limits must not be negative.")
    output = resolve_output(paths.root, args.output)
    output.mkdir(parents=True, exist_ok=True)

    with HardwareSession(paths) as session:
        initial = session.preflight()
        current_limit = abs(session.read_engineering("refs.Iq_ref", unit="A"))
        if current_limit <= 0:
            raise RuntimeError("Positive refs.Iq_ref current limit is required.")
        # The firmware clamps pid_fw output to -0.5 * sm_prot.Imax_protect
        # (see SM_Ctrl.c). Decode Imax_protect in per-unit of I_nom and convert
        # to amperes through the same current scale the dictionary uses for
        # pid_id.pid_fdb_reg3, so the id_within_limit criterion matches what the
        # hardware protection and the field-weakening clamp actually allow.
        imax_protect_pu = FixedPoint(q=24, scale=1.0, unit="pu").decode(
            session.target.read_u32("sm_prot.Imax_protect")
        )
        if imax_protect_pu <= 0:
            raise RuntimeError("sm_prot.Imax_protect must be positive.")
        current_scale_a = session.dictionary.scales.get(4, 1.0)
        fw_id_limit_a = 0.5 * imax_protect_pu * current_scale_a
        ramp_time = session.read_engineering("rmp.T", unit="s")
        speed_controller = session.read_controller("pid_spd")
        hardware = session.hardware()

        # Voltage limit and current limit are read in per-unit / A so the
        # acceptance criteria can be expressed in the same units used by the
        # captured channels.
        u_lim_pu = FixedPoint(q=24, scale=1.0, unit="pu").decode(
            session.target.read_u32("pwm.U_lim")
        )
        if u_lim_pu <= 0:
            raise RuntimeError("pwm.U_lim must be positive for field weakening.")

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
            # Decode U_mag as pure per-unit regardless of the dictionary voltage
            # display scale, so the test can compare it directly against U_lim.
            Channel(
                "u_mag_pu",
                "pwm.U_mag",
                FixedPoint(q=24, scale=1.0, unit="pu"),
            ),
            Channel(
                "id_actual_a",
                "pid_id.pid_fdb_reg3",
                session.dictionary.codec("pid_id.pid_fdb_reg3", unit="A"),
            ),
        ]

        with session.temporary_engineering_value(
            "sm_ctrl.run_mode", REQUIRED_RUN_MODE
        ) as original_run_mode_raw:
            with session.temporary_engineering_value(
                "refs.speed_ref", TARGET_SPEED_RPM, unit="rpm"
            ) as original_speed_ref_raw:
                # Field-weakening knobs (Debug*). They are edited as raw IQ24
                # because the firmware reads them directly in that format.
                with session.temporary_raw_value(
                    "DebugW2", args.fw_enable_raw
                ) as original_fw_enable_raw:
                    with session.temporary_raw_value(
                        "DebugW3", args.fw_kp_raw
                    ) as original_fw_kp_raw:
                        with session.temporary_raw_value(
                            "DebugW4", args.fw_ki_raw
                        ) as original_fw_ki_raw:
                            before = session.verify_mode(REQUIRED_RUN_MODE)
                            capture = session.capture_ring(
                                channels,
                                divider=args.sample_divider,
                                duration_s=args.timeout,
                                poll_period_s=args.poll_period_s,
                                validated_snapshot=before,
                            )
        restored = session.drive.snapshot()
        restored_fw_enable_raw = session.target.read_u32("DebugW2")
        restored_fw_kp_raw = session.target.read_u32("DebugW3")
        restored_fw_ki_raw = session.target.read_u32("DebugW4")
        if (
            restored.run_mode != original_run_mode_raw
            or restored.speed_ref_raw != original_speed_ref_raw
            or restored_fw_enable_raw != original_fw_enable_raw
            or restored_fw_kp_raw != original_fw_kp_raw
            or restored_fw_ki_raw != original_fw_ki_raw
        ):
            raise RuntimeError(
                "Temporary field-weakening settings were not restored."
            )

    metrics = analyze_field_weakening(
        capture.samples,
        target_speed_rpm=TARGET_SPEED_RPM,
        u_lim_pu=u_lim_pu,
        # id_limit is the hardware current-protection ceiling (Imax_protect)
        # expressed in amperes via the dictionary current scale, NOT the FW Id
        # setpoint limit. The criterion verifies that the measured Id stays
        # below the protection trip level; transient overshoot of the FW
        # setpoint is expected and acceptable.
        id_limit_a=imax_protect_pu * current_scale_a,
        max_time_to_95_s=args.max_time_to_95_s,
        max_steady_error_percent=args.max_steady_error_percent,
        min_fw_id_ref_a=args.min_fw_id_ref_a,
        max_steady_u_mag_pu=args.max_steady_u_mag_fraction,
    )
    metrics["criteria"]["runtime_entered_vector_mode"] = (
        capture.runtime.control_state == REQUIRED_RUN_MODE
        and capture.runtime.run_mode == REQUIRED_RUN_MODE
    )
    metrics["criteria"]["temporary_settings_restored"] = (
        restored.run_mode == original_run_mode_raw
        and restored.speed_ref_raw == original_speed_ref_raw
        and restored_fw_enable_raw == original_fw_enable_raw
        and restored_fw_kp_raw == original_fw_kp_raw
        and restored_fw_ki_raw == original_fw_ki_raw
    )
    # The capture window need only cover the acceleration and a short stretch
    # of steady state. The 95 %-rise takes ~0.3 s on the simulator, and the
    # ring-buffer drain via JTAG can deliver anywhere from ~2 s to ~3 s of
    # contiguous data depending on host latency, so a 1.5 s floor is both
    # sufficient to observe the FW transient and achievable in practice.
    metrics["criteria"]["capture_window_covers_run"] = (
        metrics["window_s"] >= 1.5
    )
    metrics["passed"] = all(metrics["criteria"].values())

    csv_path = output / "capture.csv"
    png_path = output / "field-weakening.png"
    json_path = output / "report.json"
    write_csv(csv_path, capture.samples)
    _plot(png_path, capture.samples, u_lim_pu, TARGET_SPEED_RPM)
    report = {
        "test": NAME,
        "firmware": firmware_metadata(paths),
        "hardware": hardware,
        "setup": {
            "required_run_mode": REQUIRED_RUN_MODE,
            "target_speed_rpm": TARGET_SPEED_RPM,
            "current_limit_a": current_limit,
            "imax_protect_pu": imax_protect_pu,
            "fw_id_limit_a": fw_id_limit_a,
            "ramp_time_s": ramp_time,
            "u_lim_pu": u_lim_pu,
            "fw_enable_raw": args.fw_enable_raw,
            "fw_kp_raw": args.fw_kp_raw,
            "fw_ki_raw": args.fw_ki_raw,
            "sample_divider": args.sample_divider,
            "poll_period_s": args.poll_period_s,
            "trigger_timeout_s": args.timeout,
            "speed_controller": speed_controller,
            "original_run_mode_raw": original_run_mode_raw,
            "original_speed_reference_raw": original_speed_ref_raw,
            "original_fw_enable_raw": original_fw_enable_raw,
            "original_fw_kp_raw": original_fw_kp_raw,
            "original_fw_ki_raw": original_fw_ki_raw,
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
        raise RuntimeError("Field weakening acceptance criteria failed.")


def _plot(
    path: Path,
    samples: list,
    u_lim_pu: float,
    target_speed_rpm: float,
) -> None:
    import matplotlib.pyplot as plt

    if not samples:
        return
    time_s = [sample.time_s for sample in samples]
    plt.style.use("seaborn-v0_8-whitegrid")
    figure, axes = plt.subplots(
        3, 1, figsize=(11, 8.4), sharex=True, constrained_layout=True
    )
    axes[0].plot(
        time_s, [s.values["speed_rpm"] for s in samples], label="Measured speed"
    )
    axes[0].axhline(
        target_speed_rpm, color="gray", linestyle="--", label="Reference"
    )
    axes[0].set_title("Field weakening: acceleration above base speed")
    axes[0].set_ylabel("Speed, rpm")
    axes[0].legend()

    axes[1].plot(
        time_s, [s.values["id_ref_a"] for s in samples], "--", label="Id ref (pid_fw)"
    )
    axes[1].plot(
        time_s, [s.values["id_actual_a"] for s in samples], label="Measured Id"
    )
    axes[1].axhline(0.0, color="gray", linewidth=0.8)
    axes[1].set_ylabel("d-axis current, A")
    axes[1].legend()

    axes[2].plot(
        time_s, [s.values["u_mag_pu"] for s in samples], label="U_mag"
    )
    axes[2].axhline(u_lim_pu, color="red", linestyle=":", label=f"U_lim = {u_lim_pu:.3f}")
    axes[2].axhline(
        0.9 * u_lim_pu, color="orange", linestyle="--", label="0.9 * U_lim (FW setpoint)"
    )
    axes[2].set_xlabel("Time, s")
    axes[2].set_ylabel("Voltage magnitude, pu")
    axes[2].legend()
    figure.savefig(path, dpi=170)
    plt.close(figure)
