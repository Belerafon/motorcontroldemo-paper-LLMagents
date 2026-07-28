from __future__ import annotations

import struct
from dataclasses import dataclass
from typing import Protocol


class ValueCodec(Protocol):
    unit: str

    def decode(self, raw: int) -> float: ...


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


@dataclass(frozen=True)
class Float32:
    unit: str = ""

    def decode(self, raw: int) -> float:
        return struct.unpack("<f", struct.pack("<I", raw))[0]


@dataclass(frozen=True)
class SignedInt32:
    scale: float = 1.0
    unit: str = ""

    def decode(self, raw: int) -> float:
        if raw >= 0x8000_0000:
            raw -= 0x1_0000_0000
        return raw * self.scale
