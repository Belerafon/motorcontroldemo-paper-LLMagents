from __future__ import annotations

import json
import statistics
import time
from argparse import ArgumentParser, Namespace, SUPPRESS
from contextlib import ExitStack
from pathlib import Path

from motor_debug_core.datalogger import Channel
from motor_debug_core.project import ProjectPaths

from .artifacts import plot_decoupling, write_csv, write_json
from .common import (
    HardwareSession,
    firmware_metadata,
    resolve_output,
    snapshot_dict,
)
from .metrics import analyze_decoupling_correctness, analyze_decoupling_stability


NAME = "current-decoupling"
HELP = (
    "Verify d-q cross-coupling decoupling of the current loops at speed: "
    "compensation voltage matches the analytic cross term and the loops "
    "stay stable."
)

_IQ = 1 << 24


def _iq24(value: float) -> int:
    raw = int(round(value * _IQ))
    return (raw & 0xFFFFFFFF)


def _signed(raw: int) -> int:
    return raw - 0x100000000 if raw >= 0x80000000 else raw


# Firmware symbols (all IQ24, per-unit) sampled at the steady operating point.
_OPERATING_SYMBOLS = (
    "cur_par.speed",         # we, pu (1.0 = nominal electrical speed)
    "pid_iq.pid_fdb_reg3",   # Iq, pu (1.0 = I_nom)
    "pid_id.pid_fdb_reg3",   # Id, pu
    "Debug1",                # dVd, pu (decoupling diagnostic)
    "Debug2",                # dVq, pu (decoupling diagnostic)
    "DebugW3",               # K read-back, pu
)


def register(parser: ArgumentParser) -> None:
    parser.add_argument(
        "--start-drive",
        action="store_true",
        help=SUPPRESS,
    )
    parser.add_argument("--gain", type=float, default=0.26,
                        help="Decoupling gain DebugW3, per-unit inductance.")
    parser.add_argument("--target-speed", type=float, default=500.0,
                        help="Target speed, rpm.")
    parser.add_argument("--iq-limit", type=float, default=2.0,
                        help="Current limit refs.Iq_ref during the test, A.")
    parser.add_argument("--load-nm", type=float, default=1.0,
                        help="Load torque applied to create steady q-current, Nm.")
    parser.add_argument("--capture-duration", type=float, default=3.0,
                        help="Ring-capture duration covering acceleration, s.")
    parser.add_argument("--sample-divider", type=int, default=120)
    parser.add_argument("--load-settle", type=float, default=0.7,
                        help="Settle time after the load step before sampling, s.")
    parser.add_argument("--poll-count", type=int, default=25)
    parser.add_argument("--max-dv-error-pu", type=float, default=0.010,
                        help="Max |measured - predicted| decoupling voltage, pu.")
    parser.add_argument("--max-dv-rel-error-percent", type=float, default=15.0)
    parser.add_argument("--max-id-pu", type=float, default=0.10,
                        help="Max tolerated |Id| during acceleration, pu.")
    parser.add_argument("--min-we-pu", type=float, default=0.30)
    parser.add_argument("--min-iq-pu", type=float, default=0.05)
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("artifacts/e2e/current-decoupling"),
    )


def _channels(session: HardwareSession) -> list[Channel]:
    return [
        Channel("speed_rpm", "cur_par.speed",
                session.dictionary.codec("cur_par.speed", unit="rpm")),
        Channel("iq_a", "pid_iq.pid_fdb_reg3",
                session.dictionary.codec("pid_iq.pid_fdb_reg3", unit="A")),
        Channel("id_a", "pid_id.pid_fdb_reg3",
                session.dictionary.codec("pid_id.pid_fdb_reg3", unit="A")),
        Channel("speed_ref_rpm", "rmp.output",
                session.dictionary.codec("rmp.output", unit="rpm")),
    ]


def _run_case(
    session: HardwareSession,
    gain_pu: float,
    args: Namespace,
    channels: list[Channel],
) -> tuple[list, dict[str, float], dict[str, int], dict[str, int]]:
    """Run one decoupling case: accelerate, then sample the steady point."""
    load_codec = session.dictionary.codec("model.load")
    session.target.write_u32("DebugW3", _iq24(gain_pu))
    before = session.verify_mode(5)

    operating_rows: list[tuple[int, ...]] = []
    samples: list = []
    runtime: dict[str, int] = snapshot_dict(session.drive.snapshot())
    try:
        session.logger.arm_ring(channels, divider=args.sample_divider)
        session.drive.start(validated_snapshot=before)
        samples = session.logger.capture(
            args.capture_duration, poll_period_s=0.05
        )

        # Steady-state operating point under a load that sustains q-current.
        session.target.write_u32("model.load", load_codec.encode(args.load_nm))
        time.sleep(args.load_settle)
        runtime = snapshot_dict(session.drive.snapshot())
        if (
            runtime["protection_state"] != 1
            or runtime["fault1"]
            or runtime["fault2"]
        ):
            raise RuntimeError(
                "Protection changed under load: " + session.drive.snapshot().describe()
            )
        for _ in range(max(1, args.poll_count)):
            operating_rows.append(
                tuple(session.target.read_u32(name) for name in _OPERATING_SYMBOLS)
            )
            time.sleep(0.01)
    finally:
        try:
            session.target.write_u32("model.load", load_codec.encode(0.0))
        except Exception:
            pass
        session.drive.stop()
        time.sleep(0.2)
        session.logger.stop()

    after = session._wait_stopped()
    if after.protection_state != 1 or after.fault1 or after.fault2:
        raise RuntimeError(f"Fault after stop: {after.describe()}")

    operating = _average_operating(operating_rows)
    operating["k_set_pu"] = gain_pu
    return samples, operating, snapshot_dict(before), runtime


def _average_operating(rows: list[tuple[int, ...]]) -> dict[str, float]:
    if not rows:
        return {
            "we_pu": 0.0, "iq_pu": 0.0, "id_pu": 0.0,
            "dvd_pu": 0.0, "dvq_pu": 0.0, "k_readback_pu": 0.0,
        }
    cols = list(zip(*rows))
    # Convert each raw word to signed *before* averaging: signals near zero
    # (Id, dVq) otherwise straddle 0/0xFFFFFFFF and corrupt the mean.
    means = tuple(
        statistics.mean(_signed(raw) for raw in column) / _IQ for column in cols
    )
    return {
        "we_pu": means[0],
        "iq_pu": means[1],
        "id_pu": means[2],
        "dvd_pu": means[3],
        "dvq_pu": means[4],
        "k_readback_pu": means[5],
    }


def run(args: Namespace, paths: ProjectPaths) -> None:
    if not 1 <= args.sample_divider <= 255:
        raise ValueError("--sample-divider must be in range 1..255.")
    if min(
        args.gain,
        args.target_speed,
        args.iq_limit,
        args.capture_duration,
        args.load_settle,
        args.max_dv_error_pu,
        args.max_dv_rel_error_percent,
        args.max_id_pu,
        args.min_we_pu,
        args.min_iq_pu,
    ) < 0:
        raise ValueError("Numeric arguments must not be negative.")
    if args.load_nm < 0:
        raise ValueError("--load-nm must not be negative.")
    output = resolve_output(paths.root, args.output)
    output.mkdir(parents=True, exist_ok=True)

    with HardwareSession(paths) as session:
        initial = session.preflight()
        current_limit = abs(session.read_engineering("refs.Iq_ref", unit="A"))
        hardware = session.hardware()
        channels = _channels(session)

        with session.temporary_engineering_value("sm_ctrl.run_mode", 5):
            with session.temporary_engineering_value(
                "refs.speed_ref", args.target_speed, unit="rpm"
            ):
                with session.temporary_engineering_value(
                    "refs.Iq_ref", args.iq_limit, unit="A"
                ):
                    # Reference case: decoupling off (legacy behaviour).
                    (
                        base_samples,
                        base_operating,
                        base_before,
                        base_runtime,
                    ) = _run_case(session, 0.0, args, channels)
                    # Active case: decoupling on.
                    (
                        on_samples,
                        on_operating,
                        on_before,
                        on_runtime,
                    ) = _run_case(session, args.gain, args, channels)
        restored = session.drive.snapshot()
        session.target.write_u32("DebugW3", _iq24(0.0))

    base_stability = analyze_decoupling_stability(
        base_samples,
        target_speed_rpm=args.target_speed,
        max_id_pu=args.max_id_pu,
    )
    on_stability = analyze_decoupling_stability(
        on_samples,
        target_speed_rpm=args.target_speed,
        max_id_pu=args.max_id_pu,
    )
    on_correctness = analyze_decoupling_correctness(
        on_operating,
        k_pu=on_operating["k_readback_pu"],
        max_abs_error_pu=args.max_dv_error_pu,
        max_rel_error_percent=args.max_dv_rel_error_percent,
        min_we_pu=args.min_we_pu,
        min_iq_pu=args.min_iq_pu,
    )

    criteria = {
        "baseline_loops_stable": base_stability["passed"],
        "decoupled_loops_stable": on_stability["passed"],
        "decoupling_voltage_matches_cross_term": on_correctness["passed"],
        "baseline_entered_vector_mode": (
            base_runtime["control_state"] == 5
            and base_runtime["run_mode"] == 5
        ),
        "decoupled_entered_vector_mode": (
            on_runtime["control_state"] == 5
            and on_runtime["run_mode"] == 5
        ),
        "temporary_settings_restored": (
            restored.run_mode == initial.run_mode
            and restored.speed_ref_raw == initial.speed_ref_raw
            and restored.iq_ref_raw == initial.iq_ref_raw
        ),
    }

    csv_base = output / "baseline_capture.csv"
    csv_on = output / "decoupled_capture.csv"
    png_path = output / "decoupling.png"
    json_path = output / "report.json"
    write_csv(csv_base, base_samples)
    write_csv(csv_on, on_samples)
    plot_decoupling(png_path, base_samples, on_samples)

    report = {
        "test": NAME,
        "firmware": firmware_metadata(paths),
        "hardware": hardware,
        "setup": {
            "required_run_mode": 5,
            "gain_pu": args.gain,
            "target_speed_rpm": args.target_speed,
            "iq_limit_a": args.iq_limit,
            "load_nm": args.load_nm,
            "original_current_limit_a": current_limit,
            "sample_divider": args.sample_divider,
            "capture_duration_s": args.capture_duration,
        },
        "snapshots": {
            "initial": snapshot_dict(initial),
            "baseline_before": base_before,
            "baseline_runtime": base_runtime,
            "decoupled_before": on_before,
            "decoupled_runtime": on_runtime,
            "restored": snapshot_dict(restored),
        },
        "baseline_stability": base_stability,
        "decoupled_stability": on_stability,
        "decoupling_correctness": on_correctness,
        "criteria": criteria,
        "passed": all(criteria.values()),
        "artifacts": {
            "baseline_csv": csv_base.name,
            "decoupled_csv": csv_on.name,
            "plot": png_path.name,
        },
    }
    write_json(json_path, report)
    print(json.dumps({
        "baseline_stability": base_stability["criteria"],
        "decoupled_stability": on_stability["criteria"],
        "decoupling_correctness": on_correctness["criteria"],
        "operating_point": on_operating,
        "criteria": criteria,
        "passed": report["passed"],
    }, ensure_ascii=False, indent=2))
    print(f"JSON: {json_path}")
    print(f"CSV: {csv_base.name}, {csv_on.name}")
    print(f"PNG: {png_path}")
    if not report["passed"]:
        raise RuntimeError("Current-decoupling acceptance criteria failed.")
