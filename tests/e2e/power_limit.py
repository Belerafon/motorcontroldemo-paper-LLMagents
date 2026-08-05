from __future__ import annotations

import json
import socket
import time
from argparse import ArgumentParser, Namespace, SUPPRESS
from contextlib import ExitStack
from dataclasses import dataclass
from pathlib import Path

from motor_debug_core.project import ProjectPaths
from motor_debug_core.values import FixedPoint

from .artifacts import plot_power_limit, write_csv, write_json
from .common import (
    HardwareSession,
    firmware_metadata,
    resolve_output,
    snapshot_dict,
)
from .metrics import analyze_power_limit


NAME = "power-limit"
HELP = "Validate the closed-loop constant-power limiter in vector mode 5."

# Mapping of the firmware Debug* knobs (see SM_Ctrl.c, all IQ24):
#   DebugW1 (5174h.09h) - enable: value > 0.5 pu switches the limiter on.
#   DebugW2 (5174h.0Bh) - power setpoint, pu of nominal power (drv_params.power).
#   DebugW3 (5174h.0Dh) - Kp of the power loop, pu Iq / pu P.
#   DebugW4 (5174h.0Fh) - Ki of the power loop, pu Iq / (pu P * s).
# Diagnostics: Debug1 = effective Iq limit (pu of I_nom), Debug2 = power (pu).
ENABLE_THRESHOLD_PU = 0.5
LOAD_TORQUE = 0.5          # model.load (float) -> ~10 W steady at 500 rpm
POWER_SCALE_W = 300.0      # drv_params.power: 1.0 pu = 300 W (cood.xml scale 8)
CURRENT_SCALE_A = 5.0      # drv_params.I_nom: 1.0 pu = 5 A
TARGET_SPEED_RPM = 500.0
DEFAULT_KP = 15.0
DEFAULT_KI = 60.0


def _iq24_raw(pu: float) -> int:
    value = int(round(pu * (1 << 24)))
    if not -(1 << 31) <= value < (1 << 31):
        raise ValueError(f"IQ24 value out of range: {pu}")
    return value & 0xFFFF_FFFF


def register(parser: ArgumentParser) -> None:
    parser.add_argument(
        "--start-drive",
        action="store_true",
        help=SUPPRESS,
    )
    parser.add_argument("--load-torque", type=float, default=LOAD_TORQUE)
    parser.add_argument("--kp", type=float, default=DEFAULT_KP)
    parser.add_argument("--ki", type=float, default=DEFAULT_KI)
    parser.add_argument("--setpoint-1-fraction", type=float, default=0.60)
    parser.add_argument("--setpoint-2-fraction", type=float, default=0.35)
    parser.add_argument("--poll-interval-s", type=float, default=0.05)
    parser.add_argument("--baseline-s", type=float, default=2.0)
    parser.add_argument("--dwell-1-s", type=float, default=2.5)
    parser.add_argument("--dwell-2-s", type=float, default=2.5)
    parser.add_argument("--return-s", type=float, default=2.0)
    parser.add_argument("--steady-settle-s", type=float, default=1.2)
    parser.add_argument("--max-setpoint-error-percent", type=float, default=15.0)
    parser.add_argument("--max-setpoint-error-w", type=float, default=0.5)
    parser.add_argument("--max-natural-return-error-w", type=float, default=1.5)
    parser.add_argument("--min-limit-reduction-percent", type=float, default=10.0)
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("artifacts/e2e/power-limit"),
    )


def _read_power_w(session: HardwareSession) -> float:
    codec = FixedPoint(24, POWER_SCALE_W, "W")
    return codec.decode(session.target.read_u32("cur_par.power"))


def _wait_no_fault(session: HardwareSession, timeout_s: float) -> None:
    deadline = time.monotonic() + timeout_s
    while time.monotonic() < deadline:
        snapshot = session.drive.snapshot()
        if (
            snapshot.protection_state != 1
            or snapshot.fault1
            or snapshot.fault2
        ):
            raise RuntimeError(
                f"Protection changed while running: {snapshot.describe()}"
            )
        time.sleep(0.05)


@dataclass(frozen=True)
class _PowerSample:
    number: int
    time_s: float
    values: dict[str, float]
    raw: dict[str, int]


class _LiveTelnet:
    """One persistent OpenOCD telnet session for low-overhead polling.

    The shared transport opens a fresh socket per command, which is too slow for
    multi-hertz polling. This helper keeps a single connection alive for the
    duration of the capture and reads on silence (same heuristic as the
    transport), amortising the connection setup.
    """

    def __init__(self, host: str = "127.0.0.1", port: int = 4444) -> None:
        self.sock = socket.create_connection((host, port), timeout=1.0)
        self.sock.settimeout(0.06)
        self._drain()

    def _drain(self) -> None:
        try:
            while self.sock.recv(65_536):
                pass
        except socket.timeout:
            pass

    def command(self, command: str, timeout: float = 2.0) -> str:
        self._drain()
        self.sock.sendall((command + "\n").encode("ascii"))
        chunks: list[bytes] = []
        deadline = time.monotonic() + timeout
        last_data: float | None = None
        while time.monotonic() < deadline:
            try:
                chunk = self.sock.recv(65_536)
                if not chunk:
                    break
                chunks.append(chunk)
                last_data = time.monotonic()
            except socket.timeout:
                if last_data is not None and time.monotonic() - last_data >= 0.05:
                    break
        return b"".join(chunks).decode("ascii", errors="replace")

    def close(self) -> None:
        try:
            self.sock.close()
        except OSError:
            pass

    def write(self, address: int, value: int) -> None:
        self.command(f"mww 0x{address:08x} 0x{value & 0xFFFFFFFF:08x}", timeout=2.0)


def _capture_stepped(
    session: HardwareSession,
    *,
    poll_interval_s: float,
    natural_power_w: float,
    setpoint_1_w: float,
    setpoint_2_w: float,
    baseline_s: float,
    dwell_1_s: float,
    dwell_2_s: float,
    return_s: float,
) -> tuple[list[_PowerSample], list[dict[str, object]], float]:
    """Run the drive, step the power setpoint and poll the observables directly
    over a single persistent DAP telnet session (no core stop).
    """
    from motor_debug_core.transport import OpenOcdTransport

    target = session.target
    power_codec = FixedPoint(24, POWER_SCALE_W, "W")
    limit_codec = FixedPoint(24, CURRENT_SCALE_A, "A")
    speed_codec = session.dictionary.codec("cur_par.speed", unit="rpm")
    iq_codec = session.dictionary.codec("pid_iq.pid_ref_reg3", unit="A")
    observables = [
        ("power_w", "cur_par.power", power_codec),
        ("speed_rpm", "cur_par.speed", speed_codec),
        ("iq_ref_a", "pid_iq.pid_ref_reg3", iq_codec),
        ("limit_a", "Debug1", limit_codec),
    ]
    obs_addresses = [target.address(expr) for _, expr, _ in observables]
    obs_cmd = "; ".join(f"mdw 0x{addr:08x} 1" for addr in obs_addresses)
    # Protection fields, read as 16-bit words in the same round-trip.
    prot_fields = ["sm_prot.state", "sm_prot.bit_fault1", "sm_prot.bit_fault2"]
    prot_addresses = [target.address(name) for name in prot_fields]
    prot_cmd = "; ".join(f"mdh 0x{addr:08x} 1" for addr in prot_addresses)

    total_s = baseline_s + dwell_1_s + dwell_2_s + return_s
    phases: list[dict[str, object]] = [
        {"name": "baseline", "start_s": 0.0, "end_s": baseline_s,
         "setpoint_w": None, "active": False},
        {"name": "setpoint_1", "start_s": baseline_s,
         "end_s": baseline_s + dwell_1_s, "setpoint_w": setpoint_1_w, "active": True},
        {"name": "setpoint_2", "start_s": baseline_s + dwell_1_s,
         "end_s": baseline_s + dwell_1_s + dwell_2_s,
         "setpoint_w": setpoint_2_w, "active": True},
        {"name": "disabled", "start_s": baseline_s + dwell_1_s + dwell_2_s,
         "end_s": total_s, "setpoint_w": None, "active": False},
    ]
    boundaries = [
        (baseline_s, [("DebugW2", _iq24_raw(setpoint_1_w / POWER_SCALE_W)),
                      ("DebugW1", _iq24_raw(1.0))]),
        (baseline_s + dwell_1_s,
         [("DebugW2", _iq24_raw(setpoint_2_w / POWER_SCALE_W))]),
        (baseline_s + dwell_1_s + dwell_2_s, [("DebugW1", 0)]),
    ]
    boundary_addrs = {sym: target.address(sym)
                      for writes in (b[1] for b in boundaries)
                      for sym, _ in writes}
    fired: set[int] = set()
    target.write_u32("DebugW1", 0)  # baseline: limiter off (before opening live)

    samples: list[_PowerSample] = []
    live = _LiveTelnet()
    started = time.monotonic()
    next_safety = 0.0
    number = 0
    try:
        while True:
            now = time.monotonic()
            elapsed = now - started
            if elapsed >= total_s:
                break
            for idx, (btime, writes) in enumerate(boundaries):
                if idx not in fired and elapsed >= btime:
                    for sym, val in writes:
                        live.write(boundary_addrs[sym], val)
                    fired.add(idx)
            response = live.command(obs_cmd, timeout=2.0)
            words = OpenOcdTransport.memory_values(response, 8)
            if len(words) != len(observables):
                raise RuntimeError(
                    f"Batched observable read returned {len(words)} words, "
                    f"expected {len(observables)}: {response!r}"
                )
            raw = {name: words[i] for i, (name, _, _) in enumerate(observables)}
            decoded = {
                name: codec.decode(raw[name]) for name, _, codec in observables
            }
            samples.append(_PowerSample(number, elapsed, decoded, raw))
            number += 1
            if elapsed >= next_safety:
                prot_response = live.command(prot_cmd, timeout=2.0)
                prot_vals = OpenOcdTransport.memory_values(prot_response, 4)
                if len(prot_vals) != len(prot_fields):
                    raise RuntimeError(
                        f"Protection read returned {len(prot_vals)} words: "
                        f"{prot_response!r}"
                    )
                prot_state, fault1, fault2 = prot_vals
                if prot_state != 1 or fault1 or fault2:
                    raise RuntimeError(
                        f"Protection changed during capture: "
                        f"state={prot_state} faults=0x{fault1:04X}/0x{fault2:04X}"
                    )
                next_safety = elapsed + 1.0
            time.sleep(poll_interval_s)
    finally:
        live.close()
        target.write_u32("DebugW1", 0)
        time.sleep(0.05)
    return samples, phases, natural_power_w


def run(args: Namespace, paths: ProjectPaths) -> None:
    if not 0.005 <= args.poll_interval_s <= 1.0:
        raise ValueError("--poll-interval-s must be in range 0.005..1.0.")
    if min(
        args.load_torque,
        args.kp,
        args.ki,
        args.setpoint_1_fraction,
        args.setpoint_2_fraction,
        args.baseline_s,
        args.dwell_1_s,
        args.dwell_2_s,
        args.return_s,
        args.steady_settle_s,
        args.max_setpoint_error_percent,
        args.max_setpoint_error_w,
        args.max_natural_return_error_w,
        args.min_limit_reduction_percent,
    ) < 0:
        raise ValueError("Numeric arguments must not be negative.")
    if not 0.0 < args.setpoint_2_fraction < args.setpoint_1_fraction < 1.0:
        raise ValueError(
            "Require 0 < setpoint-2-fraction < setpoint-1-fraction < 1."
        )
    if args.steady_settle_s > min(args.dwell_1_s, args.dwell_2_s):
        raise ValueError("Steady-settle window must fit inside each dwell.")
    output = resolve_output(paths.root, args.output)
    output.mkdir(parents=True, exist_ok=True)

    power_codec = FixedPoint(24, POWER_SCALE_W, "W")
    limit_codec = FixedPoint(24, CURRENT_SCALE_A, "A")

    with HardwareSession(paths) as session:
        initial = session.preflight()
        iq_ref_a = abs(session.read_engineering("refs.Iq_ref", unit="A"))
        if iq_ref_a <= 0:
            raise RuntimeError("Positive refs.Iq_ref current limit is required.")
        hardware = session.hardware()
        original_w1 = session.target.read_u32("DebugW1")
        original_w2 = session.target.read_u32("DebugW2")
        with ExitStack() as stack:
            stack.enter_context(
                session.temporary_engineering_value("model.load", args.load_torque)
            )
            stack.enter_context(
                session.temporary_engineering_value("DebugW3", _iq24_raw(args.kp))
            )
            stack.enter_context(
                session.temporary_engineering_value("DebugW4", _iq24_raw(args.ki))
            )
            stack.enter_context(
                session.temporary_engineering_value("sm_ctrl.run_mode", 5)
            )
            stack.enter_context(
                session.temporary_engineering_value(
                    "refs.speed_ref", TARGET_SPEED_RPM, unit="rpm"
                )
            )
            before = session.verify_mode(5)
            # Warm up: reach steady state with the load applied and limiter off.
            session.drive.start(validated_snapshot=before)
            _wait_no_fault(session, 0.5)
            time.sleep(args.steady_settle_s)
            natural_power_w = _read_power_w(session)
            if natural_power_w <= 0.5:
                raise RuntimeError(
                    f"Load torque did not produce measurable steady power "
                    f"(got {natural_power_w:.3f} W). Check model.load and Iq_ref."
                )
            setpoint_1_w = args.setpoint_1_fraction * natural_power_w
            setpoint_2_w = args.setpoint_2_fraction * natural_power_w
            capture_samples, phases, baseline_power_w = _capture_stepped(
                session,
                poll_interval_s=args.poll_interval_s,
                natural_power_w=natural_power_w,
                setpoint_1_w=setpoint_1_w,
                setpoint_2_w=setpoint_2_w,
                baseline_s=args.baseline_s,
                dwell_1_s=args.dwell_1_s,
                dwell_2_s=args.dwell_2_s,
                return_s=args.return_s,
            )
            runtime = session.drive.snapshot()
            session.drive.stop()
            time.sleep(0.2)
            session.logger.stop()
            # Manually restore the stepped knobs (not handled by the stack).
            session.target.write_u32("DebugW1", original_w1)
            session.target.write_u32("DebugW2", original_w2)
            restored_run_mode = session.drive.snapshot().run_mode
            restored_speed_ref = session.drive.snapshot().speed_ref_raw
            restored_w1 = session.target.read_u32("DebugW1")
            restored_w2 = session.target.read_u32("DebugW2")
        restored = session.drive.snapshot()
        if restored.run_mode != 5 or restored.speed_ref_raw != before.speed_ref_raw:
            raise RuntimeError("Temporary mode or speed reference not restored.")
        if restored_w1 != original_w1 or restored_w2 != original_w2:
            raise RuntimeError("Temporary DebugW1/DebugW2 not restored.")

    metrics = analyze_power_limit(
        capture_samples,
        phases=phases,
        natural_power_w=natural_power_w,
        iq_ref_a=iq_ref_a,
        max_setpoint_error_percent=args.max_setpoint_error_percent,
        max_setpoint_error_w=args.max_setpoint_error_w,
        max_natural_return_error_w=args.max_natural_return_error_w,
        min_limit_reduction_percent=args.min_limit_reduction_percent,
    )
    metrics["baseline_power_w"] = baseline_power_w
    metrics["setpoint_1_w"] = setpoint_1_w
    metrics["setpoint_2_w"] = setpoint_2_w
    metrics["criteria"]["runtime_stayed_in_mode_5"] = (
        runtime.control_state == 5 and runtime.run_mode == 5
    )
    metrics["criteria"]["no_faults_at_runtime"] = (
        runtime.protection_state == 1 and not runtime.fault1 and not runtime.fault2
    )
    metrics["criteria"]["debug_knobs_restored"] = (
        restored_w1 == original_w1 and restored_w2 == original_w2
    )
    metrics["criteria"]["mode_and_speed_restored"] = (
        restored_run_mode == 5 and restored_speed_ref == before.speed_ref_raw
    )
    metrics["passed"] = all(metrics["criteria"].values())

    csv_path = output / "capture.csv"
    png_path = output / "power-limit.png"
    json_path = output / "report.json"
    write_csv(csv_path, capture_samples)
    plot_power_limit(png_path, capture_samples, phases, natural_power_w)
    report = {
        "test": NAME,
        "firmware": firmware_metadata(paths),
        "hardware": hardware,
        "setup": {
            "required_run_mode": 5,
            "target_speed_rpm": TARGET_SPEED_RPM,
            "load_torque": args.load_torque,
            "iq_ref_a": iq_ref_a,
            "kp": args.kp,
            "ki": args.ki,
            "power_scale_w_per_pu": POWER_SCALE_W,
            "current_scale_a_per_pu": CURRENT_SCALE_A,
            "enable_threshold_pu": ENABLE_THRESHOLD_PU,
            "poll_interval_s": args.poll_interval_s,
            "natural_power_w": natural_power_w,
            "setpoint_1_w": setpoint_1_w,
            "setpoint_2_w": setpoint_2_w,
            "setpoint_1_fraction": args.setpoint_1_fraction,
            "setpoint_2_fraction": args.setpoint_2_fraction,
            "phase_timing_s": {
                "baseline": args.baseline_s,
                "dwell_1": args.dwell_1_s,
                "dwell_2": args.dwell_2_s,
                "return": args.return_s,
            },
            "original_debug_w1_raw": original_w1,
            "original_debug_w2_raw": original_w2,
        },
        "snapshots": {
            "initial": snapshot_dict(initial),
            "before": snapshot_dict(before),
            "runtime": snapshot_dict(runtime),
            "after": snapshot_dict(restored),
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
        raise RuntimeError("Constant-power limiter acceptance criteria failed.")
