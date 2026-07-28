---
name: motorcontrol-demo
description: Build, flash, and debug this MotorControlDemo firmware for the NIIET K1921VK01T through VectorIDE, ARM GCC, OpenOCD, and an ST-Link probe. Use for compilation, ELF/BIN generation, firmware programming and verification, OpenOCD/GDB sessions, live interrupt-counter checks, fault/state inspection, breakpoints, or carefully controlled runtime commands in this repository.
---

# MotorControlDemo

Resolve all paths relative to the repository root. Assume the supported local VectorIDE installation is `C:\VectorIDEv1.3.3` unless the user gives another path.

## Build

Run:

```powershell
python .codex\skills\motorcontrol-demo\scripts\motor_debug.py build
```

The build script calls the repository `Makefile` directly with the ARM GCC tools bundled in VectorIDE. It does not launch Eclipse. Use the `Debug` configuration; the repository's historical `Release` configuration is incomplete and targets Cortex-M3 defaults.

Expect:

- `EclipseIDE\Debug\MotorControlDemo.elf`
- `EclipseIDE\Debug\MotorControlDemo.bin`
- `0 errors, 0 warnings`

The root `Makefile` can also be called directly:

```powershell
& 'C:\VectorIDEv1.3.3\CodeBench\bin\cs-make.exe' -j8
```

## Flash

Run:

```powershell
python .codex\skills\motorcontrol-demo\scripts\motor_debug.py flash
```

Never perform a separate probe-only OpenOCD invocation immediately before flashing. Connect, halt, program, verify, and reset in one OpenOCD process. Treat `** Verified OK **` and exit code 0 as success.

If OpenOCD reports `init mode failed`, reconnect the ST-Link USB device and retry the single flash command. ST-Link V2 firmware JTAG v45 was reliable; an older JTAG v29 adapter repeatedly required physical USB reconnection. PnP restart/disable-enable did not reproduce a physical reconnect.

## Inspect a running target

Run the read-only diagnostic:

```powershell
python .codex\skills\motorcontrol-demo\scripts\motor_debug.py status
```

This resets and resumes the target, then reads state, faults, mode, and
references through the DAP without halting the CPU.

Prefer non-halting DAP reads for timing counters. Halting and resuming with GDB can itself set `F_PROGRAM_1K` or `F_PROGRAM_10K`, invalidating fault observations.

Read [references/jtag-control.md](references/jtag-control.md) before setting breakpoints, changing variables, clearing faults, or issuing control commands.

Read [references/python-debug-toolkit.md](references/python-debug-toolkit.md) before adding a new automated measurement or tuning workflow.

## General workflow for measurement and tuning tasks

Use the same sequence for acceleration tests, regulator tuning, load steps,
braking, current-loop checks, and similar drive work:

1. Define the result before touching the target: signals, units, event,
   required time resolution, duration, and acceptance metrics.
2. Confirm whether the build controls a physical drive or a simulator. Obtain
   explicit authorization for every start, reference change, fault reset, or
   other control action needed by the experiment.
3. Build and flash when the running image cannot be proven to match the ELF.
   Resolve every RAM address from that ELF; never reuse numeric addresses from
   an earlier build.
4. Inspect the control code around the event. Identify the active
   `sm_ctrl.run_mode`, the entry/exit state, the corresponding
   `dlog.set_mode` trigger bit, and the exact meaning of each signal.
5. Select four logger channels. Prefer existing object-dictionary variables so
   `object_dictionary.py` can obtain Q format and engineering scale from
   `cood.xml`. For internal-only variables, assign an explicit codec from
   `values.py`.
6. Choose the acquisition mode:
   - use event-triggered one-shot for short transitions;
   - use ring mode only for long processes and drain it faster than
     `256 * WriteDelimiter / 10000` seconds;
   - choose `WriteDelimiter` so the 256-point window covers the event while
     retaining enough time resolution.
7. Read and report the complete safety snapshot. Require `SYS_READY`,
   `PROT_ON_OK`, clear faults, `CTRL_STOP`, reviewed mode, and reviewed
   references. Arm the logger only after this review.
8. Reuse the validated snapshot when issuing `start`; do not insert a slow
   second JTAG inspection between arming and the event. Always issue `stop` in
   `finally`.
9. Read the logger through non-halting DAP accesses. For one-shot data, reorder
   from `dlog.first_point_written`. Preserve both raw words and converted
   engineering values.
10. Validate the result before reporting it: check sample count and spacing,
    trigger position, buffer-wrap continuity, units, plausible signal ranges,
    protection state, and final `CTRL_STOP`. Save raw CSV plus task-specific
    PNG and JSON metrics.

Do not use CANopen or UniCON as the runtime transport for these automated
workflows. Do not halt the CPU with GDB while PWM is active.

Start a new simple measurement from:

```powershell
python .codex\skills\motorcontrol-demo\scripts\basic_dlog_capture.py --help
```

`basic_dlog_capture.py` is deliberately limited to safe capture and raw CSV.
Copy or adapt its channel list and trigger selection, then implement
task-specific analysis as a scenario under `motor_debug_core/scenarios/`.

## Capture the digital data logger

The firmware contains a four-channel, 32-bit digital oscilloscope (`TDataLog dlog`) with 256 samples per channel. Its update rate is 10 kHz and `WriteDelimiter` selects every Nth sample. For transitions longer than the 256-sample window, use ring mode and drain completed samples through non-halting DAP reads.

After obtaining explicit authorization to start the drive, run:

```powershell
python .codex\skills\motorcontrol-demo\scripts\motor_debug.py acceleration --start-drive
```

The script resolves every RAM address from the current ELF, verifies `SYS_READY`, `PROT_ON_OK`, clear fault words, `CTRL_STOP`, mode, and references, then records:

- actual speed (`cur_par.speed`), scaled at 500 rpm/pu;
- ramped speed reference (`rmp.output`), scaled at 500 rpm/pu;
- actual q-axis current (`pid_iq.pid_fdb_reg3`), scaled at 5 A/pu;
- q-axis current reference (`pid_iq.pid_ref_reg3`), scaled at 5 A/pu.

It always issues `stop` after the capture, writes CSV and PNG artifacts, and reports 10–90% rise time, time to 95%, overshoot, and acceleration-current metrics.

The Python toolkit is intentionally layered:

- `project.py` owns build and flash lifecycle;
- `transport.py` owns OpenOCD startup and non-halting DAP access;
- `target.py` resolves symbolic ELF expressions and exposes typed memory access;
- `values.py` contains reusable fixed-point, integer, and float conversions;
- `object_dictionary.py` derives Q formats and engineering scales from `cood.xml`;
- `datalogger.py` owns the generic four-channel firmware oscilloscope;
- `drive.py` owns safety snapshots and momentary start/stop commands;
- `scenarios/` contains task-specific workflows such as acceleration measurement.

Add future workflows, including speed-regulator tuning, as new scenario modules registered in `scenarios/__init__.py`; do not duplicate transport, logger, conversion, or safety code.

## Safety

Before a start command, read and report:

- `sm_sys.state` — require `SYS_READY`;
- `sm_prot.state` — require `PROT_ON_OK`;
- `sm_prot.bit_fault1` and `bit_fault2`;
- `sm_ctrl.state`, `sm_ctrl.run_mode`;
- all applicable `refs` values.

Do not clear protections, enable PWM, change current/speed references, or issue `start` without explicit user authorization for the exact action. A `stop` command is the safe default.
