from __future__ import annotations

import csv
import json
from pathlib import Path
from typing import Any

from motor_debug_core.datalogger import Sample


def write_csv(path: Path, samples: list[Sample]) -> None:
    if not samples:
        raise RuntimeError("Oscilloscope returned no samples.")
    path.parent.mkdir(parents=True, exist_ok=True)
    names = list(samples[0].values)
    with path.open("w", encoding="utf-8-sig", newline="") as destination:
        writer = csv.DictWriter(
            destination,
            fieldnames=[
                "sample",
                "time_s",
                *names,
                *(f"{name}_raw" for name in names),
            ],
            lineterminator="\n",
        )
        writer.writeheader()
        for sample in samples:
            writer.writerow(
                {
                    "sample": sample.number,
                    "time_s": sample.time_s,
                    **sample.values,
                    **{f"{name}_raw": sample.raw[name] for name in names},
                }
            )


def write_json(path: Path, report: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(
        json.dumps(report, ensure_ascii=False, indent=2, allow_nan=False) + "\n",
        encoding="utf-8",
    )


def plot_current_mode4(path: Path, samples: list[Sample]) -> None:
    import matplotlib.pyplot as plt

    time_s = [sample.time_s for sample in samples]
    plt.style.use("seaborn-v0_8-whitegrid")
    figure, (d_axis, q_axis) = plt.subplots(
        2, 1, figsize=(11, 7.2), sharex=True, constrained_layout=True
    )
    d_axis.plot(time_s, [s.values["id_ref_a"] for s in samples], "--", label="Id ref")
    d_axis.plot(time_s, [s.values["id_a"] for s in samples], label="Id")
    d_axis.set_title("Mode 4: current-loop tracking at zero speed")
    d_axis.set_ylabel("d-axis current, A")
    d_axis.legend()
    if "iq_ref_a" in samples[0].values:
        q_axis.plot(
            time_s,
            [s.values["iq_ref_a"] for s in samples],
            "--",
            label="Iq ref",
        )
    else:
        q_axis.axhline(0.0, linestyle="--", label="Iq ref = 0")
    q_axis.plot(time_s, [s.values["iq_a"] for s in samples], label="Iq")
    q_axis.set_xlabel("Time, s")
    q_axis.set_ylabel("q-axis current, A")
    q_lines, q_labels = q_axis.get_legend_handles_labels()
    if "rotor_speed_rpm" in samples[0].values:
        speed_axis = q_axis.twinx()
        speed_axis.plot(
            time_s,
            [s.values["rotor_speed_rpm"] for s in samples],
            color="tab:red",
            alpha=0.8,
            label="Rotor speed",
        )
        speed_axis.set_ylabel("Rotor speed, rpm", color="tab:red")
        speed_axis.tick_params(axis="y", labelcolor="tab:red")
        speed_lines, speed_labels = speed_axis.get_legend_handles_labels()
        q_lines += speed_lines
        q_labels += speed_labels
    q_axis.legend(q_lines, q_labels)
    figure.savefig(path, dpi=170)
    plt.close(figure)


def plot_vector_500(path: Path, samples: list[Sample]) -> None:
    import matplotlib.pyplot as plt

    time_s = [sample.time_s for sample in samples]
    plt.style.use("seaborn-v0_8-whitegrid")
    figure, (speed_axis, current_axis) = plt.subplots(
        2, 1, figsize=(11, 7.2), sharex=True, constrained_layout=True
    )
    speed_axis.plot(
        time_s,
        [s.values["speed_ref_rpm"] for s in samples],
        "--",
        label="Speed reference",
    )
    speed_axis.plot(
        time_s,
        [s.values["speed_rpm"] for s in samples],
        label="Measured speed",
    )
    speed_axis.axhline(475.0, color="gray", linestyle=":", label="95% of 500 rpm")
    speed_axis.set_title("Vector control: acceleration to 500 rpm")
    speed_axis.set_ylabel("Speed, rpm")
    speed_axis.legend()
    current_axis.plot(
        time_s,
        [s.values["iq_ref_a"] for s in samples],
        "--",
        label="Iq reference",
    )
    current_axis.plot(
        time_s,
        [s.values["iq_a"] for s in samples],
        label="Measured Iq",
    )
    current_axis.set_xlabel("Time, s")
    current_axis.set_ylabel("q-axis current, A")
    current_axis.legend()
    figure.savefig(path, dpi=170)
    plt.close(figure)


def plot_constant_power(
    path: Path, samples: list[Sample], *, power_limit_pu: float
) -> None:
    import matplotlib.pyplot as plt

    time_s = [sample.time_s for sample in samples]
    speed_rpm = [s.values["speed_rpm"] for s in samples]
    iq_a = [s.values["iq_a"] for s in samples]
    speed_pu = [value / 500.0 for value in speed_rpm]
    iq_pu = [value / 5.0 for value in iq_a]
    power_pu = [sp * iq for sp, iq in zip(speed_pu, iq_pu)]
    plt.style.use("seaborn-v0_8-whitegrid")
    figure, (speed_axis, power_axis) = plt.subplots(
        2, 1, figsize=(11, 7.2), sharex=True, constrained_layout=True
    )
    speed_axis.plot(
        time_s,
        [s.values["speed_ref_rpm"] for s in samples],
        "--",
        label="Speed reference",
    )
    speed_axis.plot(time_s, speed_rpm, label="Measured speed")
    speed_axis.set_title("Constant-power acceleration (speed*Iq held at limit)")
    speed_axis.set_ylabel("Speed, rpm")
    speed_axis.legend()
    power_axis.plot(time_s, power_pu, label="Power speed_pu * iq_pu")
    power_axis.axhline(
        power_limit_pu,
        color="tab:red",
        linestyle=":",
        label=f"Power limit {power_limit_pu:g} pu",
    )
    if "power_pu" in samples[0].values:
        power_axis.plot(
            time_s,
            [s.values["power_pu"] for s in samples],
            color="tab:green",
            alpha=0.7,
            label="Firmware power (DebugW3)",
        )
    power_axis.set_xlabel("Time, s")
    power_axis.set_ylabel("Power, per-unit")
    power_axis.legend()
    figure.savefig(path, dpi=170)
    plt.close(figure)
