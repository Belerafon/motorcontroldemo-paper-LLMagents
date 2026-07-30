from __future__ import annotations

import re
import subprocess

from .project import ProjectPaths
from .transport import OpenOcdTransport


class SymbolTable:
    """Resolves C expressions against the exact ELF used for the target."""

    def __init__(self, paths: ProjectPaths):
        self.paths = paths
        self._cache: dict[str, int] = {}

    def address(self, expression: str) -> int:
        if expression not in self._cache:
            result = subprocess.run(
                [
                    str(self.paths.gdb),
                    "-q",
                    "-batch",
                    str(self.paths.elf),
                    "-ex",
                    f"p/x &{expression}",
                ],
                check=True,
                capture_output=True,
                text=True,
                errors="replace",
            )
            match = re.search(r"0x[0-9a-fA-F]+", result.stdout)
            if match is None:
                raise RuntimeError(
                    f"GDB did not resolve {expression!r}: {result.stdout}"
                )
            self._cache[expression] = int(match.group(0), 16)
        return self._cache[expression]


class Target:
    """Symbolic target memory access over an OpenOCD transport."""

    def __init__(self, transport: OpenOcdTransport, symbols: SymbolTable):
        self.transport = transport
        self.symbols = symbols

    def address(self, expression: str) -> int:
        return self.symbols.address(expression)

    def read_u16(self, expression: str) -> int:
        return self.transport.read_u16(self.address(expression))

    def read_u32(self, expression: str) -> int:
        return self.transport.read_u32(self.address(expression))

    def write_u16(self, expression: str, value: int) -> None:
        self.transport.write_u16(self.address(expression), value)

    def write_u32(self, expression: str, value: int) -> None:
        self.transport.write_u32(self.address(expression), value)
