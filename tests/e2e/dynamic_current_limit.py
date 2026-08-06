from __future__ import annotations

import json
import time
from argparse import ArgumentParser, Namespace, SUPPRESS
from contextlib import contextmanager
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Iterator

from motor_debug_core.drive import SafetySnapshot
from motor_debug_core.project import ProjectPaths
from motor_debug_core.values import FixedPoint, Float32

from .artifacts import write_csv, write_json
from .common import (
    HardwareSession,
    firmware_metadata,
    resolve_output,
    snapshot_dict,
)
from .metrics import analyze_dynamic_limit, expected_voltage_limit

NAME = "dynamic-current-limit"
HELP = (
    "Verify current-regulator voltage limits track the DC-link voltage "
    "and the PWM modulation capability (drive stays stopped)."
)

_CONTROLLERS = ("pid_id", "pid_iq", "pid_ia")


@dataclass(frozen=True)
class _Point:
    label: str
    udc_v: float
    u_lim: float
    udc_meas: float
    u_lim_actual: float
    limits: dict[str, tuple[float, float]]


def register(parser: ArgumentParser) -> None:
    parser.add_argument(
        "--start-drive",
        action="store_true",
        help=SUPPRESS,
    )
    parser.add_argument("--settle-s", type=float, default=0.15)
    parser.add_argument("--floor", type=float, default=0.05)
    parser.add_argument("--tolerance", type=float, default=5e-3)
    parser.add_argument("--low-ratio", type=float, default=0.55)
    parser.add_argument("--high-ratio", type=float, default=0.95)
    parser.add_argument("--alt-u-lim", type=float, default=0.5)
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("artifacts/e2e/dynamic-current-limit"),
    )


def _iq():
    return FixedPoint(q=24)


@contextmanager
def _temporary(session: HardwareSession, expression: str, value: float) -> Iterator[None]:
    codec = _iq()
    if expression.endswith("motorInternals.udc"):
        codec = Float32()
    original = session.target.read_u32(expression)
    try:
        encoded = codec.encode(value)
        session.target.write_u32(expression, encoded)
        actual = session.target.read_u32(expression)
        if actual != encoded:
            raise RuntimeError(
                f"Write verification failed for {expression}: "
                f"0x{actual:08X} != 0x{encoded:08X}."
            )
        yield
    finally:
        session.target.write_u32(expression, original)
        restored = session.target.read_u32(expression)
        if restored != original:
            raise RuntimeError(
                f"RAM restoration failed for {expression}: "
                f"0x{restored:08X} != 0x{original:08X}."
            )


def _read_point(session: HardwareSession) -> dict[str, float]:
    codec = _iq()
    return {
        "udc_meas": codec.decode(session.target.read_u32("adc.Udc_meas")),
        "u_lim": codec.decode(session.target.read_u32("pwm.U_lim")),
    }


def _read_limits(session: HardwareSession) -> dict[str, tuple[float, float]]:
    codec = _iq()
    limits: dict[str, tuple[float, float]] = {}
    for controller in _CONTROLLERS:
        maximum = codec.decode(
            session.target.read_u32(f"{controller}.pid_out_max")
        )
        minimum = codec.decode(
            session.target.read_u32(f"{controller}.pid_out_min")
        )
        limits[controller] = (maximum, minimum)
    return limits


def run(args: Namespace, paths: ProjectPaths) -> None:
    if args.settle_s <= 0:
        raise ValueError("--settle-s must be positive.")
    if not 0 < args.floor < 1:
        raise ValueError("--floor must be in range (0, 1).")
    if args.tolerance <= 0:
        raise ValueError("--tolerance must be positive.")
    if not 0 < args.low_ratio < args.high_ratio < 1:
        raise ValueError("--low-ratio must be below --high-ratio in range (0, 1).")
    if not 0 < args.alt_u_lim < 1:
        raise ValueError("--alt-u-lim must be in range (0, 1).")

    output = resolve_output(paths.root, args.output)
    output.mkdir(parents=True, exist_ok=True)

    with HardwareSession(paths) as session:
        initial = session.preflight()
        udc_nom = session.target.read_u16("drv_params.Udc_nom")
        u_max = _iq().decode(session.target.read_u32("sm_prot.Umax_protect"))
        if udc_nom <= 0:
            raise RuntimeError(f"Unexpected Udc_nom: {udc_nom}.")
        if u_max <= 0:
            raise RuntimeError(f"Unexpected Umax_protect: {u_max:g} pu.")

        safe_ratio = min(args.high_ratio, 0.95 * u_max)
        low_ratio = min(args.low_ratio, 0.7 * safe_ratio)
        if safe_ratio <= low_ratio:
            raise RuntimeError(
                "Available Udc headroom is too small to create two "
                "distinct operating points."
            )

        baseline = _read_point(session)
        baseline_limits = _read_limits(session)
        original_u_lim = session.target.read_u32("pwm.U_lim")

        points: list[_Point] = [
            _Point(
                label="baseline",
                udc_v=baseline["udc_meas"] * udc_nom,
                u_lim=baseline["u_lim"],
                udc_meas=baseline["udc_meas"],
                u_lim_actual=baseline["u_lim"],
                limits=baseline_limits,
            )
        ]

        def capture(label: str, udc_v: float, u_lim: float) -> None:
            with _temporary(session, "model.motorInternals.udc", udc_v):
                with _temporary(session, "pwm.U_lim", u_lim):
                    time.sleep(args.settle_s)
                    reading = _read_point(session)
                    limits = _read_limits(session)
            points.append(
                _Point(
                    label=label,
                    udc_v=udc_v,
                    u_lim=u_lim,
                    udc_meas=reading["udc_meas"],
                    u_lim_actual=reading["u_lim"],
                    limits=limits,
                )
            )

        capture("low-udc", low_ratio * udc_nom, baseline["u_lim"])
        capture("high-udc", safe_ratio * udc_nom, baseline["u_lim"])
        capture("high-udc-alt-pwm", safe_ratio * udc_nom, args.alt_u_lim)

        points_by_controller: dict[str, dict[str, dict[str, float]]] = {}
        for controller in _CONTROLLERS:
            points_by_controller[controller] = {}
            for point in points:
                out_max, out_min = point.limits[controller]
                points_by_controller[controller][point.label] = {
                    "udc_meas": point.udc_meas,
                    "u_lim": point.u_lim_actual,
                    "out_max": out_max,
                    "out_min": out_min,
                }
        metrics_by_controller = {
            controller: analyze_dynamic_limit(
                values,
                floor=args.floor,
                tolerance=args.tolerance,
            )
            for controller, values in points_by_controller.items()
        }
        passed = all(item["passed"] for item in metrics_by_controller.values())

        restored_u_lim = session.target.read_u32("pwm.U_lim")
        restored_snapshot = session.drive.snapshot()
        restored_criteria = {
            "u_lim_restored": restored_u_lim == original_u_lim,
            "still_stopped": restored_snapshot.control_state == 0,
        }
        if not restored_criteria["u_lim_restored"]:
            passed = False

        csv_path = output / "limits.csv"
        png_path = output / "limits.png"
        json_path = output / "report.json"
        _write_csv(csv_path, points, args.floor)
        _plot(png_path, points, args.floor)

        report = {
            "test": NAME,
            "firmware": firmware_metadata(paths),
            "hardware": session.hardware(),
            "setup": {
                "controllers": list(_CONTROLLERS),
                "udc_nom_v": udc_nom,
                "umax_protect_pu": u_max,
                "low_ratio": low_ratio,
                "safe_ratio": safe_ratio,
                "alt_u_lim": args.alt_u_lim,
                "settle_s": args.settle_s,
                "floor": args.floor,
                "tolerance": args.tolerance,
            },
            "snapshots": {
                "initial": snapshot_dict(initial),
                "restored": snapshot_dict(restored_snapshot),
            },
            "points": [asdict(point) for point in points],
            "metrics_by_controller": metrics_by_controller,
            "criteria": restored_criteria,
            "passed": passed,
            "artifacts": {
                "csv": csv_path.name,
                "plot": png_path.name,
            },
        }
        write_json(json_path, report)
        print(json.dumps(metrics_by_controller, ensure_ascii=False, indent=2))
        print(f"JSON: {json_path}")
        print(f"CSV: {csv_path}")
        print(f"PNG: {png_path}")
        if not passed:
            raise RuntimeError(
                "Dynamic current-limit tracking acceptance criteria failed."
            )


def _write_csv(path: Path, points: list[_Point], floor: float) -> None:
    import csv

    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8-sig", newline="") as destination:
        writer = csv.DictWriter(
            destination,
            fieldnames=[
                "point",
                "udc_target_v",
                "u_lim_target",
                "udc_meas_pu",
                "u_lim_pu",
                "controller",
                "out_max_pu",
                "out_min_pu",
                "expected_max_pu",
                "error_pu",
            ],
            lineterminator="\n",
        )
        writer.writeheader()
        for point in points:
            for controller, (out_max, out_min) in point.limits.items():
                expected = expected_voltage_limit(
                    point.udc_meas, point.u_lim_actual, floor=floor
                )
                writer.writerow(
                    {
                        "point": point.label,
                        "udc_target_v": point.udc_v,
                        "u_lim_target": point.u_lim,
                        "udc_meas_pu": point.udc_meas,
                        "u_lim_pu": point.u_lim_actual,
                        "controller": controller,
                        "out_max_pu": out_max,
                        "out_min_pu": out_min,
                        "expected_max_pu": expected,
                        "error_pu": abs(out_max - expected),
                    }
                )


def _plot(path: Path, points: list[_Point], floor: float) -> None:
    import matplotlib.pyplot as plt

    plt.style.use("seaborn-v0_8-whitegrid")
    figure, axis = plt.subplots(figsize=(11, 6.4), constrained_layout=True)
    colors = {"pid_id": "tab:blue", "pid_iq": "tab:orange", "pid_ia": "tab:green"}
    for point in points:
        for controller, (out_max, _out_min) in point.limits.items():
            expected = expected_voltage_limit(
                point.udc_meas, point.u_lim_actual, floor=floor
            )
            axis.scatter(
                point.udc_meas * point.u_lim_actual,
                out_max,
                color=colors[controller],
                s=40,
            )
    axis.plot([0, 1], [0, 1], "k--", linewidth=0.8, label="y = x")
    axis.set_title("Dynamic current-regulator voltage limits")
    axis.set_xlabel("udc_meas * u_lim, pu")
    axis.set_ylabel("pid_out_max, pu")
    axis.legend(
        handles=[plt.Line2D([], [], marker="o", color=c, linestyle="", label=n)
                 for n, c in colors.items()]
    )
    figure.savefig(path, dpi=170)
    plt.close(figure)
