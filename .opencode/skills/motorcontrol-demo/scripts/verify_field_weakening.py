"""One-off JTAG/DAP verification of the field weakening module.

This is NOT a registered e2e scenario. It performs a quick read-only sanity
check of the fw.* struct fields and the Debug* mirrors, both while stopped
and while running the software motor model. After the check it stops the
model and restores temporary RAM settings.
"""
from __future__ import annotations

import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / ".opencode" / "skills" / "motorcontrol-demo" / "scripts"))

from motor_debug_core.drive import Drive
from motor_debug_core.project import ProjectPaths
from motor_debug_core.target import SymbolTable, Target
from motor_debug_core.transport import OpenOcdTransport
from motor_debug_core.values import FixedPoint

IQ24 = FixedPoint(q=24, scale=1.0)


def read_iq(target: Target, expr: str) -> float:
    return IQ24.decode(target.read_u32(expr))


def read_int(target: Target, expr: str) -> int:
    raw = target.read_u32(expr)
    if raw >= 0x8000_0000:
        raw -= 0x1_0000_0000
    return raw


def show(target: Target, label: str) -> None:
    iq_fields = [
        ("fw.U_setpoint", "fw.U_setpoint"),
        ("fw.Id_mag_max", "fw.Id_mag_max"),
        ("fw.mod_index", "fw.mod_index"),
        ("fw.Id_ref", "fw.Id_ref"),
        ("fw.pid.Kp_reg3", "fw.pid.Kp_reg3"),
        ("fw.pid.Ki_reg3", "fw.pid.Ki_reg3"),
        ("fw.pid.Kc_reg3", "fw.pid.Kc_reg3"),
        ("fw.pid.pid_out_max", "fw.pid.pid_out_max"),
        ("fw.pid.pid_out_min", "fw.pid.pid_out_min"),
        ("fw.pid.ui_reg3", "fw.pid.ui_reg3"),
        ("DebugW3 (enable input, IQ24)", "DebugW3"),
        ("DebugW4 (Id_mag_max input, IQ24)", "DebugW4"),
        ("Debug1 (mod_index mirror, IQ24)", "Debug1"),
        ("Debug2 (Id_ref mirror, IQ24)", "Debug2"),
        ("pwm.U_mag", "pwm.U_mag"),
        ("pwm.U_lim", "pwm.U_lim"),
    ]
    int_fields = [
        ("fw.enable (0/1)", "fw.enable"),
        ("pwm.ULimitation (0/1)", "pwm.ULimitation"),
    ]
    print(f"\n=== {label} ===")
    for name, expr in iq_fields:
        try:
            value = read_iq(target, expr)
            print(f"  {name:38s} = {value:+.5f}")
        except Exception as error:  # noqa: BLE001
            print(f"  {name:38s} -> ERROR: {error}")
    for name, expr in int_fields:
        try:
            value = read_int(target, expr)
            print(f"  {name:38s} = {value}")
        except Exception as error:  # noqa: BLE001
            print(f"  {name:38s} -> ERROR: {error}")


def main() -> int:
    paths = ProjectPaths.discover(ROOT, Path(r"C:\VectorIDEv1.3.3"))
    transport = OpenOcdTransport(paths)
    transport.open()
    try:
        target = Target(transport, SymbolTable(paths))
        drive = Drive(target)

        snapshot = drive.snapshot(wait_ready=True)
        print(snapshot.describe())
        snapshot.require_startable()
        show(target, "At CTRL_STOP, FW disabled (DebugW3=0)")

        print("\nEnabling FW via DebugW3=1.0, DebugW4=0.5 ...")
        target.write_u32("DebugW3", IQ24.encode(1.0))
        target.write_u32("DebugW4", IQ24.encode(0.5))
        show(target, "After DebugW3=1.0, DebugW4=0.5 (still CTRL_STOP)")

        print("\nStarting software model to see fw.mod_index reacting ...")
        drive.start(validated_snapshot=snapshot)
        import time
        for _ in range(8):
            time.sleep(0.5)
            state = drive.snapshot()
            mod_index = read_iq(target, "Debug1")
            id_ref = read_iq(target, "Debug2")
            print(
                f"  runtime: {state.describe()} | "
                f"mod_index={mod_index:+.4f} Id_ref={id_ref:+.4f}"
            )
            if state.protection_state != 1 or state.fault1 or state.fault2:
                print("  ! Protection tripped during runtime check.")
                break
        drive.stop()
        import time as _t
        _t.sleep(0.2)
        show(target, "After stop")

        print("\nDisabling FW and restoring DebugW3=0, DebugW4=0 ...")
        target.write_u32("DebugW3", 0)
        target.write_u32("DebugW4", 0)
        show(target, "Final state, FW disabled")
    finally:
        transport.close()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
