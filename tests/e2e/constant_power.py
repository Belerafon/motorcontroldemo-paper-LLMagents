from __future__ import annotations

import json
from argparse import ArgumentParser, Namespace, SUPPRESS
from pathlib import Path

from motor_debug_core.datalogger import Channel
from motor_debug_core.project import ProjectPaths
from motor_debug_core.values import FixedPoint

from .artifacts import plot_constant_power, write_csv, write_json
from .common import (
    HardwareSession,
    firmware_metadata,
    resolve_output,
    snapshot_dict,
)
from .metrics import analyze_constant_power


NAME = "constant-power"
HELP = "Validate constant-power acceleration with a dynamic power limit."

#Базовые величины программы (см. cood.xml и drv_params):
#cur_par.speed - о.е. от 500 об/мин, ток - о.е. от 5 А, мощность - о.е. от базовой.
BASE_SPEED_RPM = 500.0
BASE_CURRENT_A = 5.0
RUN_MODE = 5  #CTRL_RUN_VECTOR_SM

#Ручки ограничения мощности в прошивке (см. PowerLimitIqCeiling в SM_Ctrl.c):
#IQ24, о.е. В словаре объектов CANopen это группа "Для отладки".
POWER_LIMIT_ENABLE = "DebugW1"   #0 - выкл, 1 - вкл
POWER_LIMIT_SETPOINT = "DebugW2"  #уставка мощности, о.е.
POWER_LIMIT_INTEGRAL = "DebugW3"  #коэфф. интегр. коррекции, 0 = чистый feedforward
POWER_LIMIT_CEILING = "Debug1"    #диагностика: применённый потолок Iq, о.е.


def _read_pu(session: HardwareSession, expression: str, codec: FixedPoint) -> float:
    return codec.decode(session.target.read_u32(expression))


def _write_pu(
    session: HardwareSession, expression: str, value_pu: float, codec: FixedPoint
) -> None:
    session.target.write_u32(expression, codec.encode(value_pu))


def register(parser: ArgumentParser) -> None:
    parser.add_argument("--start-drive", action="store_true", help=SUPPRESS)
    parser.add_argument("--timeout", type=float, default=5.0)
    parser.add_argument("--sample-divider", type=int, default=5)
    parser.add_argument("--target-speed-rpm", type=float, default=500.0)
    parser.add_argument("--current-limit-a", type=float, default=5.0)
    parser.add_argument("--power-limit-pu", type=float, default=0.05)
    parser.add_argument("--power-integral-gain-pu", type=float, default=0.0)
    parser.add_argument("--max-power-cv", type=float, default=0.45)
    parser.add_argument("--max-power-half-ratio-deviation", type=float, default=0.45)
    parser.add_argument("--min-power-fraction-of-limit", type=float, default=0.50)
    parser.add_argument("--max-power-fraction-of-limit", type=float, default=2.0)
    parser.add_argument("--region-lo-fraction", type=float, default=0.35)
    parser.add_argument("--region-hi-fraction", type=float, default=0.90)
    parser.add_argument("--min-region-samples", type=int, default=32)
    parser.add_argument("--reached-fraction", type=float, default=0.85)
    parser.add_argument("--current-limit-margin-percent", type=float, default=40.0)
    parser.add_argument("--max-ceiling-violation-fraction", type=float, default=0.10)
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("artifacts/e2e/constant-power"),
    )


def run(args: Namespace, paths: ProjectPaths) -> None:
    if not 1 <= args.sample_divider <= 255:
        raise ValueError("--sample-divider must be in range 1..255.")
    if args.timeout <= 0:
        raise ValueError("--timeout must be positive.")
    if args.target_speed_rpm <= 0:
        raise ValueError("--target-speed-rpm must be positive.")
    if args.current_limit_a <= 0:
        raise ValueError("--current-limit-a must be positive.")
    if not 0.0 < args.power_limit_pu <= 1.0:
        raise ValueError("--power-limit-pu must be in range (0..1].")
    if min(
        args.max_power_cv,
        args.max_power_half_ratio_deviation,
        args.min_power_fraction_of_limit,
        args.max_power_fraction_of_limit,
        args.region_lo_fraction,
        args.region_hi_fraction,
        args.reached_fraction,
        args.current_limit_margin_percent,
        args.max_ceiling_violation_fraction,
    ) < 0:
        raise ValueError("Acceptance limits must not be negative.")
    if not 0.0 < args.region_lo_fraction < args.region_hi_fraction < 1.0:
        raise ValueError(
            "Region bounds must satisfy 0 < lo < hi < 1."
        )
    if args.min_region_samples < 1:
        raise ValueError("--min-region-samples must be positive.")

    output = resolve_output(paths.root, args.output)
    output.mkdir(parents=True, exist_ok=True)

    pu_codec = FixedPoint(q=24, scale=1.0, unit="pu")

    with HardwareSession(paths) as session:
        initial = session.preflight()
        hardware = session.hardware()
        speed_controller = session.read_controller("pid_spd")
        ramp_time = session.read_engineering("rmp.T", unit="s")

        #Сохраняем скомпилированные/текущие значения "ручек" ограничения мощности.
        original_enable = session.target.read_u32(POWER_LIMIT_ENABLE)
        original_setpoint = session.target.read_u32(POWER_LIMIT_SETPOINT)
        original_integral = session.target.read_u32(POWER_LIMIT_INTEGRAL)

        channels = [
            Channel(
                "speed_rpm",
                "cur_par.speed",
                session.dictionary.codec("cur_par.speed", unit="rpm"),
            ),
            Channel(
                "iq_a",
                "pid_iq.pid_fdb_reg3",
                session.dictionary.codec("pid_iq.pid_fdb_reg3", unit="A"),
            ),
            Channel(
                "power_meas_pu",
                "cur_par.power",
                FixedPoint(q=24, scale=1.0, unit="pu"),
            ),
            Channel(
                "iq_ceiling_pu",
                POWER_LIMIT_CEILING,
                FixedPoint(q=24, scale=1.0, unit="pu"),
            ),
        ]

        capture = None
        original_run_mode_raw = None
        original_speed_ref_raw = None
        original_iq_ref_raw = None
        try:
            #Включаем ограничение мощности и задаём уставку (временные значения в RAM).
            _write_pu(session, POWER_LIMIT_ENABLE, 1.0, pu_codec)
            _write_pu(session, POWER_LIMIT_SETPOINT, args.power_limit_pu, pu_codec)
            _write_pu(session, POWER_LIMIT_INTEGRAL, args.power_integral_gain_pu, pu_codec)
            for expression, expected in (
                (POWER_LIMIT_ENABLE, 1.0),
                (POWER_LIMIT_SETPOINT, args.power_limit_pu),
                (POWER_LIMIT_INTEGRAL, args.power_integral_gain_pu),
            ):
                actual = _read_pu(session, expression, pu_codec)
                if abs(actual - expected) > 1e-3:
                    raise RuntimeError(
                        f"Power-limit handle {expression} write failed: "
                        f"{actual:.4f} != {expected:.4f}."
                    )

            with session.temporary_engineering_value(
                "sm_ctrl.run_mode", RUN_MODE
            ) as original_run_mode_raw:
                with session.temporary_engineering_value(
                    "refs.speed_ref", args.target_speed_rpm, unit="rpm"
                ) as original_speed_ref_raw:
                    with session.temporary_engineering_value(
                        "refs.Iq_ref", args.current_limit_a, unit="A"
                    ) as original_iq_ref_raw:
                        before = session.verify_mode(RUN_MODE)
                        capture = session.capture_triggered(
                            channels,
                            divider=args.sample_divider,
                            trigger_bit=4,
                            timeout_s=args.timeout,
                            validated_snapshot=before,
                        )
        finally:
            #Гарантированно возвращаем все временные "ручки" в исходное состояние.
            session.drive.stop()
            session.target.write_u32(POWER_LIMIT_ENABLE, original_enable)
            session.target.write_u32(POWER_LIMIT_SETPOINT, original_setpoint)
            session.target.write_u32(POWER_LIMIT_INTEGRAL, original_integral)

        restored = session.drive.snapshot()
        restored_enable = session.target.read_u32(POWER_LIMIT_ENABLE)
        restored_setpoint = session.target.read_u32(POWER_LIMIT_SETPOINT)
        restored_integral = session.target.read_u32(POWER_LIMIT_INTEGRAL)
        if (
            restored_enable != original_enable
            or restored_setpoint != original_setpoint
            or restored_integral != original_integral
        ):
            raise RuntimeError("Power-limit handles were not restored after the test.")
        if original_run_mode_raw is None or (
            restored.run_mode != original_run_mode_raw
            or restored.speed_ref_raw != original_speed_ref_raw
            or restored.iq_ref_raw != original_iq_ref_raw
        ):
            raise RuntimeError("Temporary mode/speed/current settings were not restored.")

    assert capture is not None
    metrics = analyze_constant_power(
        capture.samples,
        target_speed_rpm=args.target_speed_rpm,
        power_limit_pu=args.power_limit_pu,
        current_limit_a=args.current_limit_a,
        base_speed_rpm=BASE_SPEED_RPM,
        base_current_a=BASE_CURRENT_A,
        region_lo_fraction=args.region_lo_fraction,
        region_hi_fraction=args.region_hi_fraction,
        max_power_cv=args.max_power_cv,
        max_power_half_ratio_deviation=args.max_power_half_ratio_deviation,
        min_power_fraction_of_limit=args.min_power_fraction_of_limit,
        max_power_fraction_of_limit=args.max_power_fraction_of_limit,
        min_region_samples=args.min_region_samples,
        reached_fraction=args.reached_fraction,
        current_limit_margin_percent=args.current_limit_margin_percent,
        max_ceiling_violation_fraction=args.max_ceiling_violation_fraction,
    )
    metrics["criteria"]["runtime_entered_vector_mode"] = (
        capture.runtime.control_state == RUN_MODE and capture.runtime.run_mode == RUN_MODE
    )
    metrics["criteria"]["power_limit_handles_restored"] = (
        restored_enable == original_enable
        and restored_setpoint == original_setpoint
        and restored_integral == original_integral
    )
    metrics["criteria"]["mode_and_refs_restored"] = (
        restored.run_mode == original_run_mode_raw
        and restored.speed_ref_raw == original_speed_ref_raw
        and restored.iq_ref_raw == original_iq_ref_raw
    )
    window_s = metrics["window_s"]
    #Окно должно покрывать разгон: достигли ли целевой зоны, проверяет отдельный
    #критерий reached_near_target. Здесь оставляем лишь санитарную проверку, что
    #окно ненулевое и сопоставимо с событием (>= 5 мс).
    metrics["criteria"]["capture_window_present"] = window_s >= 0.005
    metrics["passed"] = all(metrics["criteria"].values())

    csv_path = output / "capture.csv"
    png_path = output / "constant-power.png"
    json_path = output / "report.json"
    write_csv(csv_path, capture.samples)
    plot_constant_power(
        png_path,
        capture.samples,
        power_limit_pu=args.power_limit_pu,
        base_current_a=BASE_CURRENT_A,
    )
    report = {
        "test": NAME,
        "firmware": firmware_metadata(paths),
        "hardware": hardware,
        "setup": {
            "required_run_mode": RUN_MODE,
            "target_speed_rpm": args.target_speed_rpm,
            "current_limit_a": args.current_limit_a,
            "power_limit_pu": args.power_limit_pu,
            "power_integral_gain_pu": args.power_integral_gain_pu,
            "ramp_time_s": ramp_time,
            "trigger_bit": 4,
            "trigger_timeout_s": args.timeout,
            "sample_divider": args.sample_divider,
            "speed_controller": speed_controller,
            "power_limit_handles": {
                "enable": POWER_LIMIT_ENABLE,
                "setpoint": POWER_LIMIT_SETPOINT,
                "integral": POWER_LIMIT_INTEGRAL,
                "ceiling_diagnostic": POWER_LIMIT_CEILING,
            },
            "original_enable_raw": original_enable,
            "original_setpoint_raw": original_setpoint,
            "original_integral_raw": original_integral,
            "original_run_mode_raw": original_run_mode_raw,
            "original_speed_ref_raw": original_speed_ref_raw,
            "original_iq_ref_raw": original_iq_ref_raw,
        },
        "snapshots": {
            "initial": snapshot_dict(initial),
            "before": snapshot_dict(capture.before),
            "runtime": snapshot_dict(capture.runtime),
            "after": snapshot_dict(capture.after),
            "restored": snapshot_dict(restored),
        },
        "metrics": metrics,
        "artifacts": {"csv": csv_path.name, "plot": png_path.name},
    }
    write_json(json_path, report)
    print(json.dumps(metrics, ensure_ascii=False, indent=2))
    print(f"JSON: {json_path}")
    print(f"CSV: {csv_path}")
    print(f"PNG: {png_path}")
    if not metrics["passed"]:
        raise RuntimeError("Constant-power acceptance criteria failed.")
