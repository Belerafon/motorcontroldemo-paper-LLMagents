"""Minimal reusable JTAG capture.

Copy this file when starting a new measurement workflow. Change CHANNELS and
the trigger bit, then add task-specific analysis separately. Transport,
symbol resolution, engineering-unit conversion, safety checks, and the
firmware oscilloscope remain shared.
"""

from __future__ import annotations

import argparse
import csv
import time
from pathlib import Path

from motor_debug_core.datalogger import Channel, DataLogger
from motor_debug_core.drive import Drive
from motor_debug_core.object_dictionary import ObjectDictionary
from motor_debug_core.project import ProjectPaths
from motor_debug_core.target import SymbolTable, Target
from motor_debug_core.transport import OpenOcdTransport


# Four channels are mandatory. A codec is derived from cood.xml, so each
# expression below must be an object-dictionary "addr". For internal-only
# variables, construct Channel objects with a codec from values.py instead.
CHANNELS = [
    ("speed_rpm", "cur_par.speed", "rpm"),
    ("speed_ref_rpm", "rmp.output", "rpm"),
    ("iq_a", "pid_iq.pid_fdb_reg3", "A"),
    ("iq_ref_a", "pid_iq.pid_ref_reg3", "A"),
]

# Search for "dlog.set_mode" in SM_Ctrl.c to find the event bit for a mode.
DEFAULT_TRIGGER_BIT = 4


def arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Basic JTAG data-logger capture")
    parser.add_argument("--start-drive", action="store_true")
    parser.add_argument("--trigger-bit", type=int, default=DEFAULT_TRIGGER_BIT)
    parser.add_argument("--divider", type=int, default=1)
    parser.add_argument("--timeout", type=float, default=5.0)
    parser.add_argument(
        "--output", type=Path, default=Path("artifacts/basic-dlog-capture.csv")
    )
    parser.add_argument(
        "--vector-ide", type=Path, default=Path(r"C:\VectorIDEv1.3.3")
    )
    return parser.parse_args()


def main() -> None:
    args = arguments()
    if not args.start_drive:
        raise SystemExit("Explicit --start-drive authorization is required.")
    if not 0 <= args.trigger_bit <= 15:
        raise ValueError("Trigger bit must be in range 0..15.")

    paths = ProjectPaths.discover(Path(__file__), args.vector_ide)
    output = args.output if args.output.is_absolute() else paths.root / args.output
    output = output.resolve()
    samples = []

    with OpenOcdTransport(paths) as transport:
        target = Target(transport, SymbolTable(paths))
        drive = Drive(target)
        logger = DataLogger(target)
        dictionary = ObjectDictionary(paths.root / "cood.xml")
        channels = [
            Channel(name, expression, dictionary.codec(expression, unit=unit))
            for name, expression, unit in CHANNELS
        ]

        before = drive.snapshot(wait_ready=True)
        print(f"Before: {before.describe()}")
        before.require_startable()

        drive_started = False
        try:
            logger.arm_triggered(
                channels,
                args.divider,
                start_bits=1 << args.trigger_bit,
            )
            # Reuse the immediately preceding validated snapshot: another slow
            # series of JTAG reads here could let a ring buffer wrap.
            drive.start(authorized=True, validated_snapshot=before)
            drive_started = True

            def safety_check() -> None:
                state = drive.snapshot()
                if state.protection_state != 1 or state.fault1 or state.fault2:
                    raise RuntimeError(
                        f"Protection changed during capture: {state.describe()}"
                    )

            samples = logger.capture_triggered(
                args.timeout,
                safety_check=safety_check,
            )
        finally:
            if drive_started:
                drive.stop()
                time.sleep(0.25)
            logger.stop()
            print(f"After: {drive.snapshot(wait_ready=True).describe()}")

    output.parent.mkdir(parents=True, exist_ok=True)
    decoded_names = [channel.name for channel in channels]
    raw_names = [f"{name}_raw" for name in decoded_names]
    with output.open("w", encoding="utf-8-sig", newline="") as destination:
        writer = csv.DictWriter(
            destination,
            fieldnames=["sample", "time_s", *decoded_names, *raw_names],
            lineterminator="\n",
        )
        writer.writeheader()
        for sample in samples:
            writer.writerow(
                {
                    "sample": sample.number,
                    "time_s": sample.time_s,
                    **sample.values,
                    **{f"{name}_raw": sample.raw[name] for name in decoded_names},
                }
            )
    print(f"Captured {len(samples)} samples")
    print(f"CSV: {output}")


if __name__ == "__main__":
    main()
