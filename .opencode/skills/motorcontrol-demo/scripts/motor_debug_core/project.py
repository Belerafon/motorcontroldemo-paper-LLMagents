from __future__ import annotations

import os
import socket
import subprocess
from dataclasses import dataclass
from pathlib import Path


@dataclass(frozen=True)
class ProjectPaths:
    root: Path
    vector_ide: Path

    @classmethod
    def discover(cls, anchor: Path, vector_ide: Path) -> "ProjectPaths":
        for candidate in (anchor.resolve(), *anchor.resolve().parents):
            if (candidate / "Makefile").is_file() and (
                candidate / ".opencode/skills/motorcontrol-demo/SKILL.md"
            ).is_file():
                return cls(candidate, vector_ide.resolve())
        raise RuntimeError("MotorControlDemo repository root was not found.")

    @property
    def elf(self) -> Path:
        return self.root / "EclipseIDE/Debug/MotorControlDemo.elf"

    @property
    def binary(self) -> Path:
        return self.root / "EclipseIDE/Debug/MotorControlDemo.bin"

    @property
    def make(self) -> Path:
        return self.vector_ide / "CodeBench/bin/cs-make.exe"

    @property
    def gdb(self) -> Path:
        return self.vector_ide / "CodeBench/bin/arm-none-eabi-gdb.exe"

    @property
    def openocd(self) -> Path:
        return self.vector_ide / "openocd/bin/openocd.exe"

    @property
    def openocd_scripts(self) -> Path:
        return self.vector_ide / "openocd/scripts"

    @property
    def openocd_config(self) -> Path:
        return self.openocd_scripts / "k1921/k1921vk01t/connect_stlink.cfg"


class Project:
    """Project lifecycle: build, flash, and creation of live target sessions."""

    def __init__(self, paths: ProjectPaths):
        self.paths = paths

    def _require(self, *paths: Path) -> None:
        for path in paths:
            if not path.is_file():
                raise FileNotFoundError(path)

    def build(self, *, incremental: bool = False, jobs: int = 8) -> None:
        codebench = self.paths.vector_ide / "CodeBench/bin"
        gnu_bin = (
            self.paths.vector_ide
            / "GNUGCC/ARM Embedded GCC/8.2.1-1.7-20190524-0603/bin"
        )
        self._require(
            self.paths.make,
            codebench / "arm-none-eabi-gcc.exe",
            codebench / "arm-none-eabi-objcopy.exe",
            codebench / "arm-none-eabi-size.exe",
            self.paths.root / "Makefile",
        )
        environment = os.environ.copy()
        environment["PATH"] = os.pathsep.join(
            [str(codebench), str(gnu_bin), environment.get("PATH", "")]
        )
        if not incremental:
            subprocess.run(
                [str(self.paths.make), "-C", str(self.paths.root), "clean"],
                check=True,
                env=environment,
            )
        subprocess.run(
            [
                str(self.paths.make),
                "-C",
                str(self.paths.root),
                f"-j{jobs}",
                "all",
            ],
            check=True,
            env=environment,
        )
        self._require(self.paths.elf, self.paths.binary)
        print(f"ELF: {self.paths.elf}")
        print(f"BIN: {self.paths.binary}")

    @staticmethod
    def _debug_port_busy() -> bool:
        try:
            with socket.create_connection(("127.0.0.1", 4444), timeout=0.2):
                return True
        except OSError:
            return False

    def flash(self) -> None:
        self._require(
            self.paths.openocd, self.paths.openocd_config, self.paths.elf
        )
        if self._debug_port_busy():
            raise RuntimeError(
                "OpenOCD is already running. Reuse or stop that session before flashing."
            )
        subprocess.run(
            [
                str(self.paths.openocd),
                "-f",
                str(self.paths.openocd_config),
                "-s",
                str(self.paths.openocd_scripts),
                "-c",
                "reset halt",
                "-c",
                f"program {self.paths.elf.as_posix()} verify reset exit",
            ],
            cwd=self.paths.root,
            check=True,
        )
        print("Firmware programmed, verified, and reset successfully.")
