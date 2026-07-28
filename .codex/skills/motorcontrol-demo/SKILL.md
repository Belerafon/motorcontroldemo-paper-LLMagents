---
name: motorcontrol-demo
description: Build, flash, and debug this MotorControlDemo firmware for the NIIET K1921VK01T through VectorIDE, ARM GCC, OpenOCD, and an ST-Link probe. Use for compilation, ELF/BIN generation, firmware programming and verification, OpenOCD/GDB sessions, live interrupt-counter checks, fault/state inspection, breakpoints, or carefully controlled runtime commands in this repository.
---

# MotorControlDemo

Resolve all paths relative to the repository root. Assume the supported local VectorIDE installation is `C:\VectorIDEv1.3.3` unless the user gives another path.

## Build

Run:

```powershell
powershell -ExecutionPolicy Bypass -File .codex\skills\motorcontrol-demo\scripts\build.ps1
```

Use the `Debug` configuration. It is the complete Cortex-M4F configuration in `.cproject`; the repository's `Release` configuration is incomplete and targets Cortex-M3 defaults.

Expect:

- `EclipseIDE\Debug\MotorControlDemo.elf`
- `EclipseIDE\Debug\MotorControlDemo.bin`
- `0 errors, 0 warnings`

The build script uses VectorIDE's Eclipse headless builder and restores `.settings\language.settings.xml` byte-for-byte so an environment hash refresh does not dirty the repository.

## Flash

Run:

```powershell
powershell -ExecutionPolicy Bypass -File .codex\skills\motorcontrol-demo\scripts\flash.ps1
```

Never perform a separate probe-only OpenOCD invocation immediately before flashing. Connect, halt, program, verify, and reset in one OpenOCD process. Treat `** Verified OK **` and exit code 0 as success.

If OpenOCD reports `init mode failed`, reconnect the ST-Link USB device and retry the single flash command. ST-Link V2 firmware JTAG v45 was reliable; an older JTAG v29 adapter repeatedly required physical USB reconnection. PnP restart/disable-enable did not reproduce a physical reconnect.

## Inspect a running target

Run the read-only diagnostic:

```powershell
powershell -ExecutionPolicy Bypass -File .codex\skills\motorcontrol-demo\scripts\jtag-status.ps1
```

This resets and resumes the target, reads RAM through the DAP without halting the CPU, compares `msCounter` and `FastCounter`, and reports state, faults, references, and ISR execution times.

Prefer non-halting DAP reads for timing counters. Halting and resuming with GDB can itself set `F_PROGRAM_1K` or `F_PROGRAM_10K`, invalidating fault observations.

Read [references/jtag-control.md](references/jtag-control.md) before setting breakpoints, changing variables, clearing faults, or issuing control commands.

## Safety

Before a start command, read and report:

- `sm_sys.state` — require `SYS_READY`;
- `sm_prot.state` — require `PROT_ON_OK`;
- `sm_prot.bit_fault1` and `bit_fault2`;
- `sm_ctrl.state`, `sm_ctrl.run_mode`;
- all applicable `refs` values.

Do not clear protections, enable PWM, change current/speed references, or issue `start` without explicit user authorization for the exact action. A `stop` command is the safe default.
