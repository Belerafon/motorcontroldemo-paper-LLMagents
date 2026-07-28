# JTAG and control reference

## Target and tools

- MCU: NIIET K1921VK01T, Cortex-M4F, hard-float ABI.
- OpenOCD config: `C:\VectorIDEv1.3.3\openocd\scripts\k1921\k1921vk01t\connect_stlink.cfg`.
- GDB: `C:\VectorIDEv1.3.3\CodeBench\bin\arm-none-eabi-gdb.exe`.
- ELF: `EclipseIDE\Debug\MotorControlDemo.elf`.
- OpenOCD ports: GDB 3333, telnet 4444, Tcl 6666.

## Start a persistent server

Start OpenOCD with `reset`, `halt`, and optionally `resume` in the same process. Do not probe and exit before a real operation: some target/probe combinations allow only the first connection after USB reconnection.

```powershell
& 'C:\VectorIDEv1.3.3\openocd\bin\openocd.exe' `
  -f 'C:\VectorIDEv1.3.3\openocd\scripts\k1921\k1921vk01t\connect_stlink.cfg' `
  -s 'C:\VectorIDEv1.3.3\openocd\scripts' `
  -c 'reset halt'
```

Attach GDB:

```powershell
& 'C:\VectorIDEv1.3.3\CodeBench\bin\arm-none-eabi-gdb.exe' `
  'EclipseIDE\Debug\MotorControlDemo.elf' `
  -ex 'target extended-remote localhost:3333'
```

Useful GDB commands:

```text
monitor reset halt
break main
continue
print msCounter
print/x sm_prot.bit_fault1
set mem inaccessible-by-default off
detach
```

Precompiled CANopen library debug paths may reference an unavailable `W:\...` source tree. This does not prevent symbol and RAM inspection.

## Non-halting diagnostics

Use OpenOCD `mdh`/`mdw` over telnet while the target runs. Resolve addresses from the current ELF rather than assuming fixed addresses:

```text
(gdb) p/x &msCounter
(gdb) p/x &sm_prot.bit_fault1
```

Important runtime symbols:

| Expression | Width | Meaning |
|---|---:|---|
| `msCounter` | 16 | 1 kHz ISR counter |
| `FastCounter` | 16 | 10 kHz ISR counter |
| `TIsr1` | 32 | Latest 1 kHz ISR execution time |
| `TIsr10` | 32 | Latest 10 kHz ISR execution time |
| `sm_sys.state` | 16 | `0=SYS_INIT`, `1=SYS_READY` |
| `sm_prot.state` | 16 | `0=OFF`, `1=ON_OK`, `2=FAIL` |
| `sm_ctrl.state` | 16 | Control state; `0=CTRL_STOP` |
| `sm_ctrl.run_mode` | 16 | Selected control algorithm |
| `sm_prot.bit_fault1/2` | 16 | Raw fault words |
| `refs.*` | 32 | IQ-format control references |

Expected behavior is approximately 1,000 increments/s for `msCounter` and 10,000 increments/s for `FastCounter`.

Do not use repeated GDB halt/resume cycles to validate timing faults. A halted core can resume with a large timer delta and set `F_PROGRAM_1K` (`0x0100`) or `F_PROGRAM_10K` (`0x0200`) as an observation artifact.

## Fault words

Relevant `bit_fault1` flags:

- `0x0040`: `F_CAN_RESTORATION_ERROR`
- `0x0100`: `F_PROGRAM_1K`
- `0x0200`: `F_PROGRAM_10K`
- `0x1000`: `F_PDPINT`
- `0x2000`: `F_RELAY_START`

Relevant `bit_fault2` flags:

- `0x0010`: `F_CTRL_MAX_I_PH_A`
- `0x0020`: `F_CTRL_MAX_I_PH_B`
- `0x0040`: `F_CTRL_MAX_I_PH_C`
- `0x0200`: `F_CTRL_SPEED_MAX`

Read masks and `sm_prot.state` before deciding whether a raw fault prevents operation.

## Control commands

`cmd` is a 32-bit union:

- bit 0 (`0x1`): `start`
- bit 1 (`0x2`): `stop`
- bit 2 (`0x4`): `trip_reset`

The 1 kHz command logic consumes and clears these momentary bits. Use a word write to `&cmd.all` while the target is running.

Before `start`, require:

1. explicit user authorization to start the physical drive;
2. `sm_sys.state == SYS_READY`;
3. `sm_prot.state == PROT_ON_OK`;
4. reviewed fault words and masks;
5. reviewed `sm_ctrl.run_mode`;
6. reviewed relevant `refs` values and their IQ scaling;
7. confirmation that the power stage and motor can move safely.

Never bypass `PROT_FAIL` by directly overwriting `sm_prot.state`. Use `trip_reset` only when the user authorizes clearing faults and the underlying fault condition is gone.

`stop` is the safe command. If uncertain, set `cmd.all` to `0x2` and verify `sm_ctrl.state == CTRL_STOP`.
