from __future__ import annotations

import csv
import json
from argparse import ArgumentParser, Namespace
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np

from ..datalogger import Channel, DataLogger, Sample
from ..drive import Drive
from ..object_dictionary import ObjectDictionary
from ..project import Project
from ..target import SymbolTable, Target
from ..transport import OpenOcdTransport
from .base import Scenario


class AccelerationScenario(Scenario):
    """One scenario: speed-step acceleration with q-axis current observation."""

    name = "acceleration"
    help = "Measure acceleration time and q-axis current using the firmware oscilloscope."

    @classmethod
    def register(cls, parser: ArgumentParser) -> None:
        super().register(parser)
        parser.add_argument(
            "--timeout", type=float, default=5.0, help="Trigger completion timeout."
        )
        parser.add_argument(
            "--sample-divider",
            type=int,
            default=1,
            help="Oscilloscope decimation, 1 gives 0.1 ms/sample.",
        )
        parser.add_argument(
            "--output",
            type=Path,
            default=Path("artifacts/motor-transition.csv"),
        )

    def run(self, project: Project, args: Namespace) -> None:
        if not args.start_drive:
            raise RuntimeError("Acceleration scenario requires --start-drive.")
        paths = project.paths
        symbols = SymbolTable(paths)
        samples: list[Sample] = []
        with OpenOcdTransport(paths) as transport:
            target = Target(transport, symbols)
            drive = Drive(target)
            logger = DataLogger(target)
            before = drive.snapshot(wait_ready=True)
            print(f"Safety: {before.describe()}")
            before.require_startable()
            dictionary = ObjectDictionary(paths.root / "cood.xml")
            channels = [
                Channel(
                    "speed_rpm",
                    "cur_par.speed",
                    dictionary.codec("cur_par.speed", unit="rpm"),
                ),
                Channel(
                    "speed_ref_rpm",
                    "rmp.output",
                    dictionary.codec("rmp.output", unit="rpm"),
                ),
                Channel(
                    "iq_a",
                    "pid_iq.pid_fdb_reg3",
                    dictionary.codec("pid_iq.pid_fdb_reg3", unit="A"),
                ),
                Channel(
                    "iq_ref_a",
                    "pid_iq.pid_ref_reg3",
                    dictionary.codec("pid_iq.pid_ref_reg3", unit="A"),
                ),
            ]
            # CTRL_RUN_VECTOR_SM uses StartBits bit 4 as its oscilloscope trigger.
            logger.arm_triggered(
                channels, args.sample_divider, start_bits=(1 << 4)
            )
            drive_started = False
            try:
                drive.start(authorized=True, validated_snapshot=before)
                drive_started = True

                def safety_check() -> None:
                    state = drive.snapshot()
                    if state.protection_state != 1 or state.fault1 or state.fault2:
                        raise RuntimeError(
                            f"Protection changed during capture: {state.describe()}"
                        )

                samples = logger.capture_triggered(
                    args.timeout, safety_check=safety_check
                )
            finally:
                if drive_started:
                    drive.stop()
                logger.stop()

        output = args.output
        if not output.is_absolute():
            output = paths.root / output
        output = output.resolve()
        output.parent.mkdir(parents=True, exist_ok=True)
        self._write_csv(output, samples)
        summary = self._analyze_and_plot(output.with_suffix(".png"), samples)
        with output.with_suffix(".summary.json").open(
            "w", encoding="utf-8", newline="\n"
        ) as destination:
            destination.write(json.dumps(summary, ensure_ascii=False, indent=2))
            destination.write("\n")
        print(json.dumps(summary, ensure_ascii=False))
        print(f"CSV: {output}")
        print(f"PNG: {output.with_suffix('.png')}")

    @staticmethod
    def _write_csv(path: Path, samples: list[Sample]) -> None:
        if not samples:
            raise RuntimeError("Oscilloscope returned no samples.")
        names = list(samples[0].values)
        with path.open("w", encoding="utf-8-sig", newline="") as destination:
            writer = csv.DictWriter(
                destination,
                fieldnames=["sample", "time_s", *names],
                lineterminator="\n",
            )
            writer.writeheader()
            for sample in samples:
                writer.writerow(
                    {
                        "sample": sample.number,
                        "time_s": sample.time_s,
                        **sample.values,
                    }
                )

    @staticmethod
    def _analyze_and_plot(path: Path, samples: list[Sample]) -> dict:
        time_s = np.array([sample.time_s for sample in samples])
        speed = np.array([sample.values["speed_rpm"] for sample in samples])
        speed_ref = np.array([sample.values["speed_ref_rpm"] for sample in samples])
        iq = np.array([sample.values["iq_a"] for sample in samples])
        iq_ref = np.array([sample.values["iq_ref_a"] for sample in samples])
        tail = max(10, len(samples) // 10)
        target = float(np.median(speed_ref[-tail:]))
        direction = 1.0 if target >= 0 else -1.0
        starts = np.flatnonzero(
            direction * speed_ref >= max(1.0, abs(target) * 0.01)
        )
        start = int(starts[0]) if starts.size else 0
        t0 = time_s[start]

        def crossing(level: float) -> int | None:
            hits = np.flatnonzero(
                direction * speed[start:] >= direction * target * level
            )
            return None if not hits.size else int(start + hits[0])

        i10, i90, i95 = crossing(0.10), crossing(0.90), crossing(0.95)
        end = i95 if i95 is not None else len(samples) - 1
        ramp = slice(start, end + 1)
        peak_speed = float(direction * np.max(direction * speed[start:]))
        overshoot = (
            (peak_speed - abs(target)) / abs(target) * 100.0 if target else 0.0
        )
        tolerance = abs(target) * 0.02
        settling_index = None
        for index in range(start, len(speed)):
            if np.all(np.abs(speed[index:] - target) <= tolerance):
                settling_index = index
                break
        summary = {
            "target_speed_rpm": target,
            "steady_speed_rpm": float(np.mean(speed[-tail:])),
            "rise_time_10_90_s": (
                None if i10 is None or i90 is None else float(time_s[i90] - time_s[i10])
            ),
            "time_to_95_percent_s": (
                None if i95 is None else float(time_s[i95] - t0)
            ),
            "overshoot_percent": overshoot,
            "settling_time_2_percent_s": (
                None
                if settling_index is None
                else float(time_s[settling_index] - t0)
            ),
            "peak_abs_iq_a_during_acceleration": float(np.max(np.abs(iq[ramp]))),
            "rms_iq_a_during_acceleration": float(
                np.sqrt(np.mean(np.square(iq[ramp])))
            ),
            "steady_iq_a": float(np.mean(iq[-tail:])),
            "samples": len(samples),
            "sample_period_s": float(np.median(np.diff(time_s))),
        }
        relative_time = time_s - t0
        plt.style.use("seaborn-v0_8-whitegrid")
        figure, (speed_axis, current_axis) = plt.subplots(
            2, 1, figsize=(11, 7.2), sharex=True, constrained_layout=True
        )
        speed_axis.plot(relative_time, speed_ref, "--", label="Задание скорости")
        speed_axis.plot(relative_time, speed, linewidth=2, label="Скорость")
        speed_axis.axhline(target * 0.95, color="gray", linestyle=":", label="95%")
        if i95 is not None:
            speed_axis.axvline(relative_time[i95], color="tab:green", linestyle=":")
        speed_axis.set_ylabel("Скорость, об/мин")
        speed_axis.set_title("Переходный процесс разгона двигателя")
        speed_axis.legend()
        current_axis.plot(relative_time, iq_ref, "--", label="Задание тока Iq")
        current_axis.plot(relative_time, iq, label="Фактический ток Iq")
        current_axis.set_xlabel("Время от начала разгона, с")
        current_axis.set_ylabel("Ток, А")
        current_axis.legend()
        figure.savefig(path, dpi=170)
        plt.close(figure)
        return summary
