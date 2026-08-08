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


def plot_decoupling_comparison(
    path: Path,
    samples_off: list[Sample],
    samples_on: list[Sample],
    target_speed_rpm: float,
) -> None:
    import matplotlib.pyplot as plt

    time_off = [sample.time_s for sample in samples_off]
    time_on = [sample.time_s for sample in samples_on]
    plt.style.use("seaborn-v0_8-whitegrid")
    figure, axes = plt.subplots(
        3, 1, figsize=(11, 10), sharex=True, constrained_layout=True
    )
    speed_axis = axes[0]
    speed_axis.plot(time_off, [s.values["speed_rpm"] for s in samples_off],
                    label="Speed (decoupling off)")
    speed_axis.plot(time_on, [s.values["speed_rpm"] for s in samples_on],
                    label="Speed (decoupling on)")
    speed_axis.axhline(target_speed_rpm, color="gray", linestyle=":",
                       label=f"Target {target_speed_rpm:g} rpm")
    speed_axis.set_ylabel("Speed, rpm")
    speed_axis.legend()
    speed_axis.set_title("Decoupling A/B: current-regulator response at speed")

    iq_axis = axes[1]
    iq_axis.plot(time_off, [s.values["iq_ref_a"] for s in samples_off],
                 "--", color="tab:gray", label="Iq ref (off)")
    iq_axis.plot(time_off, [s.values["iq_a"] for s in samples_off],
                 color="tab:blue", label="Iq (off)")
    iq_axis.plot(time_on, [s.values["iq_a"] for s in samples_on],
                 color="tab:orange", label="Iq (on)")
    iq_axis.set_ylabel("q-axis current, A")
    iq_axis.legend()

    id_axis = axes[2]
    id_axis.plot(time_off, [s.values["id_a"] for s in samples_off],
                 color="tab:blue", label="Id (off)")
    id_axis.plot(time_on, [s.values["id_a"] for s in samples_on],
                 color="tab:orange", label="Id (on)")
    id_axis.axhline(0.0, color="gray", linestyle=":")
    id_axis.set_xlabel("Time, s")
    id_axis.set_ylabel("d-axis current, A")
    id_axis.legend()
    figure.savefig(path, dpi=170)
    plt.close(figure)


def plot_decoupling_feedforward(
    path: Path,
    samples: list[Sample],
    target_speed_rpm: float,
) -> None:
    import matplotlib.pyplot as plt

    time_s = [sample.time_s for sample in samples]
    plt.style.use("seaborn-v0_8-whitegrid")
    figure, (feed_axis, speed_axis) = plt.subplots(
        2, 1, figsize=(11, 6.4), sharex=True, constrained_layout=True
    )
    feed_axis.plot(time_s, [s.values["ud_ff"] for s in samples],
                   label="d-axis feedforward (DebugF1)")
    feed_axis.plot(time_s, [s.values["uq_ff"] for s in samples],
                   label="q-axis feedforward (DebugF2)")
    feed_axis.set_ylabel("Feedforward voltage, p.u.")
    feed_axis.legend()
    feed_axis.set_title("Decoupling feedforward at speed")
    speed_axis.plot(time_s, [s.values["speed_rpm"] for s in samples],
                    color="tab:red")
    speed_axis.axhline(target_speed_rpm, color="gray", linestyle=":")
    speed_axis.set_xlabel("Time, s")
    speed_axis.set_ylabel("Speed, rpm", color="tab:red")
    figure.savefig(path, dpi=170)
    plt.close(figure)
