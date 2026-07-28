# Python debug toolkit

`scripts/motor_debug.py` is the single CLI entry point. It uses JTAG/OpenOCD
only; CANopen and UniCON are not runtime dependencies.

`scripts/basic_dlog_capture.py` is the smallest working capture example. It is
the preferred starting point when exploring a new set of signals before the
workflow deserves a registered scenario and task-specific analysis.

## Stable layers

- `project.py`: clean/incremental builds and verified flashing.
- `transport.py`: OpenOCD lifecycle and non-halting DAP memory operations.
- `target.py`: ELF-based symbolic address resolution and typed memory access.
- `object_dictionary.py`: Q format and engineering scale lookup from `cood.xml`.
- `values.py`: reusable fixed-point, floating-point, and integer codecs.
- `drive.py`: safety snapshots plus momentary `start` and `stop` commands.
- `datalogger.py`: generic four-channel ring and event-triggered one-shot modes.

## Scenarios

Task-specific behavior belongs under `scripts/motor_debug_core/scenarios/`.
Each scenario:

1. registers its CLI arguments;
2. reads and reports a safety snapshot;
3. selects four symbolic expressions and codecs;
4. arms the generic data logger;
5. starts only with explicit authorization;
6. guarantees `stop` in `finally`;
7. performs task-specific analysis and artifact generation.

Before writing a scenario, use the basic script to prove the chosen symbols,
codecs, trigger bit, divider, and window length. Keep the first capture raw;
add automatic parameter changes only after observation is trustworthy.

Register a new scenario in `scenarios/__init__.py`. Speed-controller tuning,
current-loop tuning, load steps, and braking tests should reuse the stable
layers rather than implement their own OpenOCD, scaling, or logger code.

## Firmware oscilloscope notes

The current source uses four 32-bit arrays of 256 points and updates at 10 kHz.
The manuals describe an older 16-bit build; do not truncate the current data.
For short events, prefer firmware-triggered one-shot capture and order the
result from `dlog.first_point_written`. For long processes, use ring mode and
drain the buffer faster than its wrap period:

`wrap_period = 256 * WriteDelimiter / 10000`.

The poll-and-read interval must be comfortably shorter than this period. A
single modulo-256 write counter cannot reveal that a complete wrap was missed.
