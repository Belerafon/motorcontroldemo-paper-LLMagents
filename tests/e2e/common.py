from __future__ import annotations

import hashlib
import subprocess
import time
from contextlib import contextmanager
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Iterator

from motor_debug_core.datalogger import Channel, DataLogger, Sample
from motor_debug_core.drive import Drive, SafetySnapshot
from motor_debug_core.object_dictionary import ObjectDictionary
from motor_debug_core.project import ProjectPaths
from motor_debug_core.target import SymbolTable, Target
from motor_debug_core.transport import OpenOcdTransport
from motor_debug_core.values import FixedPoint, ValueCodec


HARDWARE_TYPES = {
    0: "vectorcard-drv8301",
    1: "vectorcard-drv8312",
    2: "vectorcard-simulator",
    3: "niiet-board-simulator",
    4: "motorcontrolboard-simulator",
}

# Plain IQ24 codec (scale=1.0) for variables that the dictionary exposes as
# raw per-unit IQ24 - e.g. the Debug* scratch variables.
IQ24_PU = FixedPoint(q=24, scale=1.0)


@dataclass(frozen=True)
class CaptureResult:
    samples: list[Sample]
    before: SafetySnapshot
    runtime: SafetySnapshot
    after: SafetySnapshot


def snapshot_dict(snapshot: SafetySnapshot) -> dict[str, int]:
    return asdict(snapshot)


def resolve_output(root: Path, output: Path) -> Path:
    return output.resolve() if output.is_absolute() else (root / output).resolve()


def firmware_metadata(paths: ProjectPaths) -> dict[str, str | int]:
    digest = hashlib.sha256(paths.elf.read_bytes()).hexdigest()
    commit = subprocess.run(
        ["git", "rev-parse", "HEAD"],
        cwd=paths.root,
        check=True,
        capture_output=True,
        text=True,
    ).stdout.strip()
    dirty = bool(
        subprocess.run(
            ["git", "status", "--porcelain"],
            cwd=paths.root,
            check=True,
            capture_output=True,
            text=True,
        ).stdout
    )
    return {
        "git_commit": commit,
        "git_dirty": dirty,
        "elf": str(paths.elf),
        "elf_sha256": digest,
        "elf_size_bytes": paths.elf.stat().st_size,
    }


class HardwareSession:
    """One OpenOCD session with fail-safe drive and logger cleanup."""

    def __init__(self, paths: ProjectPaths):
        self.paths = paths
        self.transport = OpenOcdTransport(paths)
        self.target: Target
        self.drive: Drive
        self.logger: DataLogger
        self.dictionary: ObjectDictionary

    def __enter__(self) -> "HardwareSession":
        self.transport.open()
        self.target = Target(self.transport, SymbolTable(self.paths))
        self.drive = Drive(self.target)
        self.logger = DataLogger(self.target)
        self.dictionary = ObjectDictionary(self.paths.root / "cood.xml")
        return self

    def __exit__(self, *_exc_info) -> None:
        try:
            try:
                self.drive.stop()
            finally:
                self.logger.stop()
        finally:
            self.transport.close()

    def hardware(self) -> dict[str, str | int]:
        number = self.target.read_u16("sw.HardwareType")
        return {
            "hardware_type": number,
            "hardware_name": HARDWARE_TYPES.get(number, "unknown"),
        }

    def preflight(self) -> SafetySnapshot:
        snapshot = self.drive.snapshot(wait_ready=True)
        print(f"Safety: {snapshot.describe()}")
        snapshot.require_startable()
        return snapshot

    def verify_mode(self, expected: int) -> SafetySnapshot:
        snapshot = self.preflight()
        if snapshot.run_mode != expected:
            raise RuntimeError(
                f"Run mode {expected} required, got {snapshot.run_mode}."
            )
        return snapshot

    def read_engineering(self, expression: str, *, unit: str = "") -> float:
        codec = self.dictionary.codec(expression, unit=unit)
        return codec.decode(self.target.read_u32(expression))

    def read_iq24(self, expression: str) -> float:
        """Read a raw IQ24 per-unit symbol via direct fixed-point codec.

        Use for symbols that the dictionary exposes as plain IQ24 with no
        engineering scale (e.g. fw.U_setpoint, fw.mod_index, Debug1/2/...).
        """
        return IQ24_PU.decode(self.target.read_u32(expression))

    # Internal alias so scenarios can call the same helper as a private hook
    # without coupling to the module-level constant.
    _read_iq24 = read_iq24

    @contextmanager
    def temporary_iq24_value(
        self, expression: str, value_pu: float
    ) -> Iterator[int]:
        """Temporarily write an IQ24 per-unit value and restore it on exit.

        Safer than temporary_engineering_value for symbols whose dictionary
        entry's StandartNum variant wins over IQ (multiple dictionary entries
        mapping to the same address). Always decodes/encodes via plain IQ24.
        """
        original = self.target.read_u32(expression)
        encoded = IQ24_PU.encode(value_pu)
        try:
            self.target.write_u32(expression, encoded)
            actual = self.target.read_u32(expression)
            if actual != encoded:
                raise RuntimeError(
                    f"Write verification failed for {expression}: "
                    f"0x{actual:08X} != 0x{encoded:08X}."
                )
            yield original
        finally:
            self.drive.stop()
            time.sleep(0.1)
            self.target.write_u32(expression, original)
            restored = self.target.read_u32(expression)
            if restored != original:
                raise RuntimeError(
                    f"RAM restoration failed for {expression}: "
                    f"0x{restored:08X} != 0x{original:08X}."
                )

    def read_controller(self, name: str) -> dict[str, float]:
        fields = {
            "kp": f"{name}.Kp_reg3",
            "ki": f"{name}.Ki_reg3",
            "kd": f"{name}.Kd_reg3",
            "kc": f"{name}.Kc_reg3",
            "output_max_v": f"{name}.pid_out_max",
            "output_min_v": f"{name}.pid_out_min",
        }
        return {
            key: self.read_engineering(expression)
            for key, expression in fields.items()
        }

    @contextmanager
    def temporary_engineering_value(
        self, expression: str, value: float, *, unit: str = ""
    ) -> Iterator[int]:
        codec = self.dictionary.codec(expression, unit=unit)
        original = self.target.read_u32(expression)
        encoded = codec.encode(value)
        try:
            self.target.write_u32(expression, encoded)
            actual = self.target.read_u32(expression)
            if actual != encoded:
                raise RuntimeError(
                    f"Write verification failed for {expression}: "
                    f"0x{actual:08X} != 0x{encoded:08X}."
                )
            yield original
        finally:
            self.drive.stop()
            time.sleep(0.1)
            self.target.write_u32(expression, original)
            restored = self.target.read_u32(expression)
            if restored != original:
                raise RuntimeError(
                    f"RAM restoration failed for {expression}: "
                    f"0x{restored:08X} != 0x{original:08X}."
                )

    def capture_ring(
        self,
        channels: list[Channel],
        *,
        divider: int,
        duration_s: float,
        poll_period_s: float,
        validated_snapshot: SafetySnapshot,
    ) -> CaptureResult:
        if duration_s <= 0:
            raise ValueError("Capture duration must be positive.")
        wrap_period = DataLogger.buffer_length * divider / DataLogger.update_rate_hz
        if poll_period_s >= wrap_period / 2:
            raise ValueError(
                f"Poll period {poll_period_s:g}s is too slow for "
                f"{wrap_period:g}s ring-buffer wrap period."
            )
        self.logger.arm_ring(channels, divider)
        samples: list[Sample] = []
        runtime = validated_snapshot
        try:
            self.drive.start(validated_snapshot=validated_snapshot)

            def safety_check() -> None:
                state = self.drive.snapshot()
                if (
                    state.protection_state != 1
                    or state.fault1
                    or state.fault2
                ):
                    raise RuntimeError(
                        f"Protection changed during capture: {state.describe()}"
                    )

            samples = self.logger.capture(
                duration_s,
                safety_check=safety_check,
                poll_period_s=poll_period_s,
            )
            runtime = self.drive.snapshot()
            if (
                runtime.protection_state != 1
                or runtime.fault1
                or runtime.fault2
            ):
                raise RuntimeError(
                    f"Protection changed after capture: {runtime.describe()}"
                )
        finally:
            self.drive.stop()
            time.sleep(0.2)
            self.logger.stop()

        after = self._wait_stopped()
        print(f"Stopped: {after.describe()}")
        if after.protection_state != 1 or after.fault1 or after.fault2:
            raise RuntimeError(f"Fault after stop: {after.describe()}")
        return CaptureResult(samples, validated_snapshot, runtime, after)

    def capture_triggered(
        self,
        channels: list[Channel],
        *,
        divider: int,
        trigger_bit: int,
        timeout_s: float,
        validated_snapshot: SafetySnapshot,
    ) -> CaptureResult:
        if not 0 <= trigger_bit <= 15:
            raise ValueError("Trigger bit must be in range 0..15.")
        if timeout_s <= 0:
            raise ValueError("Trigger timeout must be positive.")
        self.logger.arm_triggered(
            channels,
            divider,
            start_bits=1 << trigger_bit,
        )
        samples: list[Sample] = []
        runtime = validated_snapshot
        try:
            self.drive.start(validated_snapshot=validated_snapshot)

            def safety_check() -> None:
                state = self.drive.snapshot()
                if (
                    state.protection_state != 1
                    or state.fault1
                    or state.fault2
                ):
                    raise RuntimeError(
                        f"Protection changed during capture: {state.describe()}"
                    )

            samples = self.logger.capture_triggered(
                timeout_s,
                safety_check=safety_check,
            )
            runtime = self.drive.snapshot()
            if (
                runtime.protection_state != 1
                or runtime.fault1
                or runtime.fault2
            ):
                raise RuntimeError(
                    f"Protection changed after capture: {runtime.describe()}"
                )
        finally:
            self.drive.stop()
            time.sleep(0.2)
            self.logger.stop()

        after = self._wait_stopped()
        print(f"Stopped: {after.describe()}")
        if after.protection_state != 1 or after.fault1 or after.fault2:
            raise RuntimeError(f"Fault after stop: {after.describe()}")
        return CaptureResult(samples, validated_snapshot, runtime, after)

    def stop_and_wait(self, *, delay_s: float = 0.2) -> SafetySnapshot:
        self.drive.stop()
        time.sleep(delay_s)
        snapshot = self._wait_stopped()
        if (
            snapshot.protection_state != 1
            or snapshot.fault1
            or snapshot.fault2
        ):
            raise RuntimeError(f"Fault after stop: {snapshot.describe()}")
        return snapshot

    def _wait_stopped(self) -> SafetySnapshot:
        snapshot = self.drive.snapshot()
        for _ in range(20):
            if snapshot.control_state == 0:
                return snapshot
            time.sleep(0.05)
            snapshot = self.drive.snapshot()
        raise RuntimeError(f"Drive did not return to CTRL_STOP: {snapshot.describe()}")
