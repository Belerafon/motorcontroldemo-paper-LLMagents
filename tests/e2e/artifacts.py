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


def plot_decoupling_high_speed(
    path: Path, samples: list[Sample], *, target_speed_rpm: float = 3000.0
) -> None:
    import matplotlib.pyplot as plt

    time_s = [sample.time_s for sample in samples]
    channel_names = list(samples[0].values.keys()) if samples else []
    has_vd = "vd_comp" in channel_names
    has_vq = "vq_comp" in channel_names
    n_plots = 2 + (1 if has_vd or has_vq else 0)
    plt.style.use("seaborn-v0_8-whitegrid")
    figure, axes = plt.subplots(
        n_plots, 1, figsize=(11, 6 + n_plots * 1.5), sharex=True, constrained_layout=True
    )
    if n_plots == 2:
        speed_axis, current_axis = axes
        voltage_axis = None
    else:
        speed_axis, current_axis, voltage_axis = axes
    speed_axis.plot(
        time_s,
        [s.values["rotor_speed_rpm"] for s in samples],
        color="tab:green",
        label="Rotor speed",
    )
    speed_axis.axhline(
        target_speed_rpm * 0.95,
        color="gray",
        linestyle=":",
        label="95% of target speed",
    )
    speed_axis.set_ylabel("Speed, rpm")
    speed_axis.legend()
    current_axis.plot(
        time_s,
        [s.values["iq_ref_a"] for s in samples],
        "--",
        color="tab:red",
        label="Iq reference",
    )
    current_axis.plot(
        time_s,
        [s.values["iq_a"] for s in samples],
        color="tab:red",
        label="Iq actual",
    )
    current_axis.plot(
        time_s,
        [s.values["id_a"] for s in samples],
        color="tab:blue",
        label="Id actual",
    )
    current_axis.set_ylabel("Current, A")
    current_axis.legend()
    if voltage_axis is not None:
        if has_vd:
            voltage_axis.plot(
                time_s,
                [s.values["vd_comp"] for s in samples],
                color="tab:blue",
                label="Vd (PI output, decoupled)",
            )
        if has_vq:
            voltage_axis.plot(
                time_s,
                [s.values["vq_comp"] for s in samples],
                color="tab:red",
                label="Vq (PI output, decoupled)",
            )
        voltage_axis.set_ylabel("Voltage ref, A (scaled)")
        voltage_axis.legend()
    axes[-1].set_xlabel("Time, s")
    figure.savefig(path, dpi=170)
    plt.close(figure)
