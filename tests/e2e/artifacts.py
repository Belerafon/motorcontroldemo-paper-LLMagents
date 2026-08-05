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


def plot_power_limit(
    path: Path,
    samples: list[Sample],
    phases: list[dict[str, object]],
    natural_power_w: float,
) -> None:
    import matplotlib.pyplot as plt

    time_s = [sample.time_s for sample in samples]
    plt.style.use("seaborn-v0_8-whitegrid")
    figure, (power_axis, drive_axis) = plt.subplots(
        2, 1, figsize=(11, 7.6), sharex=True, constrained_layout=True
    )
    power_axis.plot(
        time_s, [s.values["power_w"] for s in samples], label="Measured power"
    )
    palette = ["tab:green", "tab:purple", "tab:orange", "tab:brown"]
    for index, phase in enumerate(phases):
        if phase.get("setpoint_w") is None:
            power_axis.axvline(
                float(phase["start_s"]), color="gray", linestyle=":", linewidth=1.0
            )
            continue
        color = palette[index % len(palette)]
        power_axis.axhline(
            float(phase["setpoint_w"]),
            color=color,
            linestyle="--",
            linewidth=1.3,
            label=f"{phase['name']} setpoint",
        )
        power_axis.axvline(
            float(phase["start_s"]), color=color, linestyle=":", linewidth=1.0
        )
    power_axis.axhline(
        natural_power_w, color="gray", linestyle="-.", label="Natural power"
    )
    power_axis.set_title("Constant-power limiter: stepped setpoint tracking")
    power_axis.set_ylabel("Electrical power, W")
    power_axis.legend(fontsize=8)

    drive_axis.plot(
        time_s, [s.values["speed_rpm"] for s in samples], label="Speed"
    )
    limit_axis = drive_axis.twinx()
    limit_axis.plot(
        time_s,
        [s.values["limit_a"] for s in samples],
        color="tab:red",
        alpha=0.85,
        label="Effective Iq limit (Debug1)",
    )
    drive_axis.set_xlabel("Time, s")
    drive_axis.set_ylabel("Speed, rpm")
    limit_axis.set_ylabel("Effective Iq limit, A", color="tab:red")
    limit_axis.tick_params(axis="y", labelcolor="tab:red")
    handles_a, labels_a = drive_axis.get_legend_handles_labels()
    handles_b, labels_b = limit_axis.get_legend_handles_labels()
    drive_axis.legend(handles_a + handles_b, labels_a + labels_b, fontsize=8)
    figure.savefig(path, dpi=170)
    plt.close(figure)
