"""Quick A/B probe of FW at high speed: FW off vs FW on at 1500 rpm."""
from __future__ import annotations

import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / ".opencode" / "skills" / "motorcontrol-demo" / "scripts"))

from motor_debug_core.drive import Drive
from motor_debug_core.project import ProjectPaths
from motor_debug_core.target import SymbolTable, Target
from motor_debug_core.transport import OpenOcdTransport
from motor_debug_core.values import FixedPoint

IQ24 = FixedPoint(q=24, scale=1.0)
SPEED_RPM = FixedPoint(q=24, scale=500.0, unit="rpm")


def read_iq(target: Target, expr: str) -> float:
    return IQ24.decode(target.read_u32(expr))


def read_rpm(target: Target, expr: str) -> float:
    return SPEED_RPM.decode(target.read_u32(expr))


def probe(target: Target, drive: Drive, label: str, fw_enable: bool) -> None:
    print(f"\n--- {label} ---")
    snapshot = drive.snapshot(wait_ready=True)
    snapshot.require_startable()

    target.write_u32("DebugW3", IQ24.encode(1.0 if fw_enable else 0.0))
    target.write_u32("DebugW4", IQ24.encode(0.5))
    original_speed = target.read_u32("refs.speed_ref")
    target.write_u32("refs.speed_ref", SPEED_RPM.encode(1500.0))

    try:
        drive.start(validated_snapshot=snapshot)
        for step in range(15):
            time.sleep(0.5)
            state = drive.snapshot()
            speed = read_rpm(target, "cur_par.speed")
            mod_index = read_iq(target, "Debug1")
            id_ref = read_iq(target, "Debug2")
            u_mag = read_iq(target, "pwm.U_mag")
            id_actual = read_iq(target, "pid_id.pid_fdb_reg3")
            iq_actual = read_iq(target, "pid_iq.pid_fdb_reg3")
            print(
                f"  t={(step+1)*0.5:4.1f}s | CTRL={state.control_state} "
                f"speed={speed:7.1f} rpm | mod_index={mod_index:.3f} "
                f"U_mag={u_mag:.3f} | Id_ref={id_ref:+.3f} "
                f"Id={id_actual:+.3f} Iq={iq_actual:+.3f} | "
                f"faults=0x{state.fault1:04X}/0x{state.fault2:04X}"
            )
            if state.protection_state != 1 or state.fault1 or state.fault2:
                print("  ! Protection tripped - aborting probe.")
                break
    finally:
        drive.stop()
        time.sleep(0.3)
        target.write_u32("refs.speed_ref", original_speed)
        target.write_u32("DebugW3", 0)
        target.write_u32("DebugW4", 0)


def main() -> int:
    paths = ProjectPaths.discover(ROOT, Path(r"C:\VectorIDEv1.3.3"))
    transport = OpenOcdTransport(paths)
    transport.open()
    try:
        target = Target(transport, SymbolTable(paths))
        drive = Drive(target)
        probe(target, drive, "Phase A: FW OFF, target 1500 rpm", fw_enable=False)
        time.sleep(0.5)
        probe(target, drive, "Phase B: FW ON, target 1500 rpm", fw_enable=True)
    finally:
        transport.close()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
