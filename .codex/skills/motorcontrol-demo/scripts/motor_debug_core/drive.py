from __future__ import annotations

import time
from dataclasses import dataclass

from .target import Target
from .values import FixedPoint


@dataclass(frozen=True)
class SafetySnapshot:
    sys_state: int
    protection_state: int
    fault1: int
    fault2: int
    control_state: int
    run_mode: int
    speed_ref_raw: int
    iq_ref_raw: int
    flux_ref_raw: int

    def describe(self) -> str:
        speed = FixedPoint(24, 500, "rpm").decode(self.speed_ref_raw)
        iq = FixedPoint(24, 5, "A").decode(self.iq_ref_raw)
        flux = FixedPoint(24, 5, "A").decode(self.flux_ref_raw)
        return (
            f"SYS={self.sys_state}, PROT={self.protection_state}, "
            f"CTRL={self.control_state}, mode={self.run_mode}, "
            f"faults=0x{self.fault1:04X}/0x{self.fault2:04X}, "
            f"speed={speed:.1f} rpm, Iq={iq:.3f} A, flux={flux:.3f} A"
        )

    def require_startable(self) -> None:
        if self.sys_state != 1:
            raise RuntimeError(f"SYS_READY required, got {self.sys_state}.")
        if self.protection_state != 1:
            raise RuntimeError(
                f"PROT_ON_OK required, got {self.protection_state}."
            )
        if self.fault1 or self.fault2:
            raise RuntimeError(
                f"Faults are active: 0x{self.fault1:04X}/0x{self.fault2:04X}."
            )
        if self.control_state != 0:
            raise RuntimeError(f"CTRL_STOP required, got {self.control_state}.")


class Drive:
    """Model state inspection and momentary command handling."""

    def __init__(self, target: Target):
        self.target = target

    def snapshot(self, wait_ready: bool = False) -> SafetySnapshot:
        snapshot = None
        attempts = 30 if wait_ready else 1
        for _ in range(attempts):
            snapshot = SafetySnapshot(
                sys_state=self.target.read_u16("sm_sys.state"),
                protection_state=self.target.read_u16("sm_prot.state"),
                fault1=self.target.read_u16("sm_prot.bit_fault1"),
                fault2=self.target.read_u16("sm_prot.bit_fault2"),
                control_state=self.target.read_u16("sm_ctrl.state"),
                run_mode=self.target.read_u32("sm_ctrl.run_mode"),
                speed_ref_raw=self.target.read_u32("refs.speed_ref"),
                iq_ref_raw=self.target.read_u32("refs.Iq_ref"),
                flux_ref_raw=self.target.read_u32("refs.i_flux_ref"),
            )
            if snapshot.sys_state == 1 and snapshot.protection_state == 1:
                break
            if snapshot.protection_state == 2 or snapshot.fault1 or snapshot.fault2:
                break
            time.sleep(0.2)
        assert snapshot is not None
        return snapshot

    def start(
        self, *, validated_snapshot: SafetySnapshot | None = None
    ) -> None:
        snapshot = validated_snapshot or self.snapshot(wait_ready=True)
        snapshot.require_startable()
        self.target.write_u32("cmd.all", 0x1)

    def stop(self) -> None:
        self.target.write_u32("cmd.all", 0x2)
