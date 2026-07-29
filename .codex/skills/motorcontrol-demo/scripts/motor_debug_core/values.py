from __future__ import annotations

import struct
from dataclasses import dataclass
from typing import Protocol


class ValueCodec(Protocol):
    unit: str

    def decode(self, raw: int) -> float: ...

    def encode(self, value: float) -> int: ...


@dataclass(frozen=True)
class FixedPoint:
    """Signed fixed-point quantity with an optional engineering-unit scale."""

    q: int
    scale: float = 1.0
    unit: str = ""

    def decode(self, raw: int) -> float:
        if raw >= 0x8000_0000:
            raw -= 0x1_0000_0000
        return raw / float(1 << self.q) * self.scale

    def encode(self, value: float) -> int:
        if self.scale == 0:
            raise ValueError("Fixed-point scale must not be zero.")
        raw = round(value / self.scale * (1 << self.q))
        if not -(1 << 31) <= raw < (1 << 31):
            raise ValueError(f"Fixed-point value is out of 32-bit range: {value}")
        return raw & 0xFFFF_FFFF


@dataclass(frozen=True)
class Float32:
    unit: str = ""

    def decode(self, raw: int) -> float:
        return struct.unpack("<f", struct.pack("<I", raw))[0]

    def encode(self, value: float) -> int:
        return struct.unpack("<I", struct.pack("<f", value))[0]


@dataclass(frozen=True)
class SignedInt32:
    scale: float = 1.0
    unit: str = ""

    def decode(self, raw: int) -> float:
        if raw >= 0x8000_0000:
            raw -= 0x1_0000_0000
        return raw * self.scale

    def encode(self, value: float) -> int:
        if self.scale == 0:
            raise ValueError("Integer scale must not be zero.")
        raw = round(value / self.scale)
        if not -(1 << 31) <= raw < (1 << 31):
            raise ValueError(f"Integer value is out of 32-bit range: {value}")
        return raw & 0xFFFF_FFFF
