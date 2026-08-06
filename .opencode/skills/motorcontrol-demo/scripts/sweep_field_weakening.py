"""Find target speed where FW regulates mod_index near U_setpoint."""
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
CURRENT_A = FixedPoint(q=24, scale=5.0, unit="A")


def read(target, expr, codec):
    return codec.decode(target.read_u32(expr))


def probe_one(target, drive, target_speed):
    snapshot = drive.snapshot(wait_ready=True)
    snapshot.require_startable()
    target.write_u32("DebugW3", IQ24.encode(1.0))
    target.write_u32("DebugW4", IQ24.encode(0.5))
    original = target.read_u32("refs.speed_ref")
    target.write_u32("refs.speed_ref", SPEED_RPM.encode(target_speed))
    drive.start(validated_snapshot=snapshot)
    time.sleep(2.5)
    state = drive.snapshot()
    speed = read(target, "cur_par.speed", SPEED_RPM)
    mod_index = read(target, "Debug1", IQ24)
    id_ref_a = read(target, "pid_id.pid_ref_reg3", CURRENT_A)
    iq_a = read(target, "pid_iq.pid_fdb_reg3", CURRENT_A)
    print(
        f"  target={target_speed:5.0f} rpm | actual={speed:6.1f} | "
        f"mod_index={mod_index:.3f} | Id_ref={id_ref_a:+.3f} A | "
        f"Iq={iq_a:+.3f} A | faults=0x{state.fault1:04X}"
    )
    drive.stop()
    time.sleep(0.3)
    target.write_u32("refs.speed_ref", original)
    target.write_u32("DebugW3", 0)
    target.write_u32("DebugW4", 0)
    return speed, mod_index, id_ref_a


def main():
    paths = ProjectPaths.discover(ROOT, Path(r"C:\VectorIDEv1.3.3"))
    transport = OpenOcdTransport(paths)
    transport.open()
    try:
        target = Target(transport, SymbolTable(paths))
        drive = Drive(target)
        print("Speed sweep with FW enabled (Id_mag_max = 0.5 p.u.):")
        for target_speed in [500, 600, 650, 700, 750, 800, 900, 1000]:
            probe_one(target, drive, target_speed)
        print("\nNow with larger FW authority (Id_mag_max = 1.0 p.u.):")
        snapshot = drive.snapshot(wait_ready=True)
        snapshot.require_startable()
        target.write_u32("DebugW3", IQ24.encode(1.0))
        target.write_u32("DebugW4", IQ24.encode(1.0))
        for target_speed in [800, 1000, 1200, 1500]:
            original = target.read_u32("refs.speed_ref")
            target.write_u32("refs.speed_ref", SPEED_RPM.encode(target_speed))
            drive.start(validated_snapshot=snapshot)
            time.sleep(2.5)
            state = drive.snapshot()
            speed = read(target, "cur_par.speed", SPEED_RPM)
            mod_index = read(target, "Debug1", IQ24)
            id_ref_a = read(target, "pid_id.pid_ref_reg3", CURRENT_A)
            iq_a = read(target, "pid_iq.pid_fdb_reg3", CURRENT_A)
            print(
                f"  target={target_speed:5.0f} rpm | actual={speed:6.1f} | "
                f"mod_index={mod_index:.3f} | Id_ref={id_ref_a:+.3f} A | "
                f"Iq={iq_a:+.3f} A | faults=0x{state.fault1:04X}"
            )
            drive.stop()
            time.sleep(0.3)
            target.write_u32("refs.speed_ref", original)
            snapshot = drive.snapshot(wait_ready=True)
            snapshot.require_startable()
        target.write_u32("DebugW3", 0)
        target.write_u32("DebugW4", 0)
    finally:
        transport.close()


if __name__ == "__main__":
    main()
