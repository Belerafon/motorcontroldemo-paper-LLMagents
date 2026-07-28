from __future__ import annotations

import re
import socket
import subprocess
import tempfile
import time
from pathlib import Path

from .project import ProjectPaths


class OpenOcdTransport:
    """Owns or reuses OpenOCD and exposes non-halting DAP memory access."""

    def __init__(self, paths: ProjectPaths):
        self.paths = paths
        self.process: subprocess.Popen[bytes] | None = None
        self.started_here = False
        self._out = None
        self._err = None

    @staticmethod
    def port_ready() -> bool:
        try:
            with socket.create_connection(("127.0.0.1", 4444), timeout=0.25):
                return True
        except OSError:
            return False

    def open(self) -> "OpenOcdTransport":
        if self.port_ready():
            return self
        temp_dir = Path(tempfile.gettempdir())
        self._out = (temp_dir / "motor-debug-openocd.out.log").open("wb")
        self._err = (temp_dir / "motor-debug-openocd.err.log").open("wb")
        self.process = subprocess.Popen(
            [
                str(self.paths.openocd),
                "-f",
                str(self.paths.openocd_config),
                "-s",
                str(self.paths.openocd_scripts),
                "-c",
                "reset",
                "-c",
                "halt",
                "-c",
                "resume",
            ],
            cwd=self.paths.root,
            stdout=self._out,
            stderr=self._err,
            creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0),
        )
        self.started_here = True
        for _ in range(40):
            if self.process.poll() is not None:
                self._err.flush()
                details = Path(self._err.name).read_text(
                    encoding="utf-8", errors="replace"
                )
                raise RuntimeError(f"OpenOCD exited early:\n{details}")
            if self.port_ready():
                return self
            time.sleep(0.1)
        raise RuntimeError("OpenOCD did not open telnet port 4444.")

    def close(self) -> None:
        if self.started_here and self.port_ready():
            try:
                self.command("shutdown")
            except OSError:
                pass
        if self.process is not None:
            try:
                self.process.wait(timeout=3)
            except subprocess.TimeoutExpired:
                self.process.terminate()
        for stream in (self._out, self._err):
            if stream is not None:
                stream.close()

    def __enter__(self) -> "OpenOcdTransport":
        return self.open()

    def __exit__(self, *_exc_info) -> None:
        self.close()

    def command(self, command: str, timeout: float = 3.0) -> str:
        with socket.create_connection(("127.0.0.1", 4444), timeout=timeout) as client:
            client.settimeout(0.06)
            try:
                while client.recv(65_536):
                    pass
            except socket.timeout:
                pass
            client.sendall((command + "\n").encode("ascii"))
            chunks: list[bytes] = []
            deadline = time.monotonic() + timeout
            last_data = None
            while time.monotonic() < deadline:
                try:
                    chunk = client.recv(65_536)
                    if not chunk:
                        break
                    chunks.append(chunk)
                    last_data = time.monotonic()
                except socket.timeout:
                    if last_data is not None and time.monotonic() - last_data >= 0.1:
                        break
            return b"".join(chunks).decode("ascii", errors="replace")

    @staticmethod
    def memory_values(response: str, digits: int) -> list[int]:
        pattern = re.compile(
            rf"0x[0-9a-fA-F]+:\s+((?:[0-9a-fA-F]{{{digits}}}\s+)+)"
        )
        return [
            int(token, 16)
            for match in pattern.finditer(response)
            for token in match.group(1).split()
        ]

    def read_words(self, address: int, count: int) -> list[int]:
        response = self.command(f"mdw 0x{address:08x} {count}", timeout=5.0)
        values = self.memory_values(response, 8)
        if len(values) != count:
            raise RuntimeError(
                f"Expected {count} words at 0x{address:08X}, got {len(values)}."
            )
        return values

    def read_u16(self, address: int) -> int:
        response = self.command(f"mdh 0x{address:08x} 1")
        values = self.memory_values(response, 4)
        if len(values) != 1:
            raise RuntimeError(f"Cannot read u16 at 0x{address:08X}: {response}")
        return values[0]

    def read_u32(self, address: int) -> int:
        return self.read_words(address, 1)[0]

    def write_u16(self, address: int, value: int) -> None:
        self.command(f"mwh 0x{address:08x} 0x{value & 0xFFFF:04x}")

    def write_u32(self, address: int, value: int) -> None:
        self.command(f"mww 0x{address:08x} 0x{value & 0xFFFFFFFF:08x}")
