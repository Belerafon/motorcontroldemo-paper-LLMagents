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
    "Engage field weakening above the base-speed region and verify that "
    "negative Id is auto-commanded while modulation index stays in check."
)
# Default target is set just above the no-FW voltage-saturation point of the
# bundled PMSM model (about 680 rpm with the default Udc/U_lim). With
# Id_mag_max = 0.5 p.u. the FW regulator can keep the modulation index below
# the hard limit at this speed while commanding a clearly negative Id.
TARGET_SPEED_RPM = 700.0
FW_ENABLE_PU = 1.0
FW_ID_MAG_MAX_PU = 0.5

# DebugW3 / DebugW4 / Debug1 / Debug2 are exposed in cood.xml with both an IQ24
# and a StandartNum variant on the same address. The object-dictionary layer
# resolves the codec deterministically but not always to IQ24 - so for the FW
# per-unit channels we override with an explicit IQ24 codec.
DEBUG_IQ24 = FixedPoint(q=24, scale=1.0)


def register(parser: ArgumentParser) -> None:
    parser.add_argument(
        "--start-drive",
        action="store_true",
        help=SUPPRESS,
    )
    parser.add_argument("--timeout", type=float, default=8.0)
    parser.add_argument("--sample-divider", type=int, default=200)
    parser.add_argument("--target-speed-rpm", type=float, default=TARGET_SPEED_RPM)
    parser.add_argument("--fw-id-mag-max-pu", type=float, default=FW_ID_MAG_MAX_PU)
    parser.add_argument("--max-steady-mod-index-pu", type=float, default=1.05)
    parser.add_argument("--min-fw-id-ref-a", type=float, default=0.30)
    parser.add_argument("--min-steady-speed-rpm", type=float, default=650.0)
    parser.add_argument("--settling-dwell-s", type=float, default=0.50)
    parser.add_argument("--settling-tolerance-rpm", type=float, default=30.0)
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
    if args.target_speed_rpm <= 0:
        raise ValueError("--target-speed-rpm must be positive.")
    if not 0 < args.fw_id_mag_max_pu <= 1.0:
        raise ValueError("--fw-id-mag-max-pu must be in range (0; 1].")
    if min(
        args.max_steady_mod_index_pu,
        args.min_fw_id_ref_a,
        args.min_steady_speed_rpm,
        args.settling_dwell_s,
        args.settling_tolerance_rpm,
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
        hardware = session.hardware()
        u_setpoint_pu = session._read_iq24("fw.U_setpoint")
        channels = [
            Channel(
                "speed_rpm",
                "cur_par.speed",
                session.dictionary.codec("cur_par.speed", unit="rpm"),
            ),
            Channel(
                "mod_index_pu",
                "Debug1",
                DEBUG_IQ24,
            ),
            Channel(
                "id_ref_a",
                "pid_id.pid_ref_reg3",
                session.dictionary.codec("pid_id.pid_ref_reg3", unit="A"),
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
                "refs.speed_ref", args.target_speed_rpm, unit="rpm"
            ) as original_speed_ref_raw:
                with session.temporary_iq24_value(
                    "DebugW3", FW_ENABLE_PU
                ) as original_debugw3_raw:
                    with session.temporary_iq24_value(
                        "DebugW4", args.fw_id_mag_max_pu
                    ) as original_debugw4_raw:
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

    metrics = analyze_field_weakening(
        capture.samples,
        target_speed_rpm=args.target_speed_rpm,
        u_setpoint_pu=u_setpoint_pu,
        max_steady_mod_index_pu=args.max_steady_mod_index_pu,
        min_fw_id_ref_a=args.min_fw_id_ref_a,
        min_steady_speed_rpm=args.min_steady_speed_rpm,
        settling_dwell_s=args.settling_dwell_s,
        settling_tolerance_rpm=args.settling_tolerance_rpm,
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
    metrics["criteria"]["capture_window_covers_run"] = (
        metrics["window_s"] >= 3.0
    )
    metrics["passed"] = all(metrics["criteria"].values())

    csv_path = output / "capture.csv"
    png_path = output / "field-weakening.png"
    json_path = output / "report.json"
    write_csv(csv_path, capture.samples)
    plot_field_weakening(png_path, capture.samples, u_setpoint_pu=u_setpoint_pu)
    report = {
        "test": NAME,
        "firmware": firmware_metadata(paths),
        "hardware": hardware,
        "setup": {
            "required_run_mode": 5,
            "target_speed_rpm": args.target_speed_rpm,
            "current_limit_a": current_limit,
            "ramp_time_s": ramp_time,
            "fw_enable_pu": FW_ENABLE_PU,
            "fw_id_mag_max_pu": args.fw_id_mag_max_pu,
            "fw_u_setpoint_pu": u_setpoint_pu,
            "trigger_bit": 4,
            "trigger_timeout_s": args.timeout,
            "sample_divider": args.sample_divider,
            "original_run_mode_raw": original_run_mode_raw,
            "original_speed_reference_raw": original_speed_ref_raw,
            "original_debugw3_raw": original_debugw3_raw,
            "original_debugw4_raw": original_debugw4_raw,
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
