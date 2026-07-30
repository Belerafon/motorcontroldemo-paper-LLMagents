from __future__ import annotations

import time
from dataclasses import dataclass

from .target import Target
from .values import ValueCodec


@dataclass(frozen=True)
class Channel:
    name: str
    expression: str
    codec: ValueCodec


@dataclass(frozen=True)
class Sample:
    number: int
    time_s: float
    values: dict[str, float]
    raw: dict[str, int]


class DataLogger:
    """Generic four-channel firmware oscilloscope with ring-buffer draining."""

    buffer_length = 256
    channel_count = 4
    update_rate_hz = 10_000

    def __init__(self, target: Target):
        self.target = target
        self.channels: list[Channel] = []
        self.divider = 1
        self._last_counter = 0

    def arm_ring(self, channels: list[Channel], divider: int) -> None:
        self._arm(channels, divider, start_bits=0, prehistory_points=0)

    def arm_triggered(
        self,
        channels: list[Channel],
        divider: int,
        *,
        start_bits: int,
        prehistory_points: int = 0,
    ) -> None:
        """Arm ring history and let firmware switch to one-shot on an event."""
        if not 0 <= prehistory_points <= 255:
            raise ValueError("Prehistory must be in range 0..255 samples.")
        self._arm(
            channels,
            divider,
            start_bits=start_bits,
            prehistory_points=prehistory_points,
        )

    def _arm(
        self,
        channels: list[Channel],
        divider: int,
        *,
        start_bits: int,
        prehistory_points: int,
    ) -> None:
        if len(channels) != self.channel_count:
            raise ValueError("The firmware data logger requires exactly four channels.")
        if not 1 <= divider <= 255:
            raise ValueError("Data logger divider must be in range 1..255.")
        self.stop()
        time.sleep(0.15)
        self.target.write_u16("dlog.StartBits", start_bits)
        self.target.write_u16("dlog.OneShotOperation", 1)
        self.target.write_u32("dlog.trig_shift", prehistory_points << 16)
        control = self.target.read_u32("dlog.control")
        self.target.write_u32("dlog.control", control & ~(1 << 6))
        for index, channel in enumerate(channels, start=1):
            address = self.target.address(channel.expression)
            self.target.write_u32(f"dlog.object{index}Info.varAddr", address)
            self.target.write_u16(f"dlog.object{index}Info.varSize", 32)
        self.target.write_u32("dlog.WriteDelimiter", divider)
        self.target.write_u16("dlog.mode_reset", 2)
        time.sleep(0.3)
        if self.target.read_u16("dlog.mode_reset") != 2:
            raise RuntimeError("Data logger ring mode was not armed.")
        if self.target.read_u32("dlog.WriteDelimiter") != divider:
            raise RuntimeError("Data logger divider verification failed.")
        for index, channel in enumerate(channels, start=1):
            actual = self.target.read_u32(f"dlog.object{index}Info.varAddr")
            expected = self.target.address(channel.expression)
            if actual != expected:
                raise RuntimeError(
                    f"Channel {index} pointer mismatch: 0x{actual:08X} != "
                    f"0x{expected:08X}."
                )
        self.channels = channels
        self.divider = divider
        self._last_counter = self.target.read_u16("dlog.Wcounter")

    def capture_triggered(
        self,
        timeout_s: float,
        *,
        safety_check=None,
        poll_period_s: float = 0.02,
    ) -> list[Sample]:
        """Wait for a firmware event-triggered one-shot and return ordered samples."""
        deadline = time.monotonic() + timeout_s
        next_safety = time.monotonic() + 0.5
        while time.monotonic() < deadline:
            control = self.target.read_u32("dlog.control")
            if control & (1 << 6):
                break
            if safety_check is not None and time.monotonic() >= next_safety:
                safety_check()
                next_safety += 0.5
            time.sleep(poll_period_s)
        else:
            raise TimeoutError("The firmware oscilloscope did not complete its one-shot.")

        first = self.target.read_u16("dlog.first_point_written") & 0xFF
        prehistory = self.target.read_u16("dlog.prehistory_length")
        first_count = self.buffer_length - first
        rows = self._read_segment(first, first_count)
        if first:
            rows.extend(self._read_segment(0, first))
        period = self.divider / self.update_rate_hz
        result = []
        for number, row in enumerate(rows):
            raw = {channel.name: value for channel, value in zip(self.channels, row)}
            decoded = {
                channel.name: channel.codec.decode(value)
                for channel, value in zip(self.channels, row)
            }
            result.append(
                Sample(number, (number - prehistory) * period, decoded, raw)
            )
        return result

    def stop(self) -> None:
        self.target.write_u16("dlog.mode_reset", 0)

    def _read_segment(self, start: int, count: int) -> list[list[int]]:
        if not count:
            return []
        buffer_addresses = [
            self.target.address(f"dlog.buff[{index * self.buffer_length}]")
            for index in range(self.channel_count)
        ]
        command = "; ".join(
            f"mdw 0x{address + start * 4:08x} {count}"
            for address in buffer_addresses
        )
        response = self.target.transport.command(command, timeout=5.0)
        values = self.target.transport.memory_values(response, 8)
        expected = self.channel_count * count
        if len(values) != expected:
            raise RuntimeError(
                f"Expected {expected} oscilloscope words, got {len(values)}."
            )
        return [
            [values[channel * count + point] for channel in range(self.channel_count)]
            for point in range(count)
        ]

    def drain(self, next_number: int) -> list[Sample]:
        current = self.target.read_u16("dlog.Wcounter")
        count = (current - self._last_counter) % self.buffer_length
        if not count:
            return []
        first_count = min(count, self.buffer_length - self._last_counter)
        rows = self._read_segment(self._last_counter, first_count)
        if count > first_count:
            rows.extend(self._read_segment(0, count - first_count))
        period = self.divider / self.update_rate_hz
        result = []
        for offset, row in enumerate(rows):
            raw = {channel.name: value for channel, value in zip(self.channels, row)}
            decoded = {
                channel.name: channel.codec.decode(value)
                for channel, value in zip(self.channels, row)
            }
            number = next_number + offset
            result.append(Sample(number, number * period, decoded, raw))
        self._last_counter = current
        return result

    def capture(
        self,
        duration_s: float,
        *,
        safety_check=None,
        poll_period_s: float = 0.2,
    ) -> list[Sample]:
        samples: list[Sample] = []
        started = time.monotonic()
        next_safety = started + 1.0
        while time.monotonic() - started < duration_s:
            time.sleep(poll_period_s)
            samples.extend(self.drain(len(samples)))
            if safety_check is not None and time.monotonic() >= next_safety:
                safety_check()
                next_safety += 1.0
        return samples
