from __future__ import annotations

import json
from argparse import ArgumentParser, Namespace, SUPPRESS
from contextlib import contextmanager
from pathlib import Path
from typing import Iterator

from motor_debug_core.datalogger import Channel
from motor_debug_core.project import ProjectPaths
from motor_debug_core.values import FixedPoint, SignedInt32

from .artifacts import plot_pwm_svpwm, write_csv, write_json
from .common import (
    HardwareSession,
    firmware_metadata,
    resolve_output,
    snapshot_dict,
)
from .svpwm import analyze_pwm_svpwm, compute_sv_gamma


NAME = "pwm-svpwm"
HELP = (
    "Verify symmetric (continuous) SVPWM: firmware math, no bus clamping "
    "and clean model run."
)

# Должно совпадать с PWM_TYPE_SVPWM в Vinclude/V_PWM_Module.h.
PWM_TYPE_SVPWM = 6
TARGET_SPEED_RPM = 300.0


def register(parser: ArgumentParser) -> None:
    parser.add_argument(
        "--start-drive",
        action="store_true",
        help=SUPPRESS,
    )
    parser.add_argument("--timeout", type=float, default=4.0)
    parser.add_argument("--sample-divider", type=int, default=8)
    parser.add_argument("--math-tolerance-ticks", type=int, default=2)
    parser.add_argument("--max-clamped-fraction", type=float, default=0.05)
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("artifacts/e2e/pwm-svpwm"),
    )


@contextmanager
def temporary_pwm_type(
    session: HardwareSession, new_value: int
) -> Iterator[int]:
    """Безопасная временная установка pwm.PWM_type (int16) с восстановлением.

    Использует 16-битный доступ (поле int16 в прошивке), чтобы не задеть
    соседние поля структуры (PWM_typeOld, sector). Восстановление
    гарантируется в ``finally``.
    """
    original = session.target.read_u16("pwm.PWM_type")
    try:
        session.target.write_u16("pwm.PWM_type", new_value)
        actual = session.target.read_u16("pwm.PWM_type")
        if actual != new_value:
            raise RuntimeError(
                f"Write verification failed for pwm.PWM_type: "
                f"0x{actual:04X} != 0x{new_value:04X}."
            )
        yield original
    finally:
        session.drive.stop()
        session.target.write_u16("pwm.PWM_type", original)
        restored = session.target.read_u16("pwm.PWM_type")
        if restored != original:
            raise RuntimeError(
                f"RAM restoration failed for pwm.PWM_type: "
                f"0x{restored:04X} != 0x{original:04X}."
            )


def run(args: Namespace, paths: ProjectPaths) -> None:
    if not 1 <= args.sample_divider <= 255:
        raise ValueError("--sample-divider must be in range 1..255.")
    if args.timeout <= 0:
        raise ValueError("--timeout must be positive.")
    if args.math_tolerance_ticks < 0:
        raise ValueError("--math-tolerance-ticks must not be negative.")
    if args.max_clamped_fraction < 0 or args.max_clamped_fraction > 1:
        raise ValueError("--max-clamped-fraction must be in range 0..1.")
    output = resolve_output(paths.root, args.output)
    output.mkdir(parents=True, exist_ok=True)

    with HardwareSession(paths) as session:
        initial = session.preflight()
        k_pwm = session.target.read_u32("pwm.k_pwm")
        if k_pwm <= 0:
            raise RuntimeError(f"Invalid pwm.k_pwm = {k_pwm}.")
        hardware = session.hardware()

        # Кодеки: для UalphaNorm/UbetaNorm строим IQ24 вручную (сигнал не
        # выведен в cood.xml), масштаб 1.0 = "значение базового вектора",
        # как оставляет PWM_Module_NormInput. Gamma - целые отсчёты таймера.
        iq24_unit = FixedPoint(q=24, scale=1.0, unit="per-unit")
        ticks = SignedInt32(scale=1.0, unit="ticks")

        channels = [
            Channel("ualpha_norm", "pwm.UalphaNorm", iq24_unit),
            Channel("ubeta_norm", "pwm.UbetaNorm", iq24_unit),
            Channel("gamma_a", "pwm.GammaA", ticks),
            Channel("gamma_b", "pwm.GammaB", ticks),
        ]

        with temporary_pwm_type(session, PWM_TYPE_SVPWM) as original_pwm_type:
            with session.temporary_engineering_value(
                "sm_ctrl.run_mode", 5
            ) as original_run_mode_raw:
                with session.temporary_engineering_value(
                    "refs.speed_ref", TARGET_SPEED_RPM, unit="rpm"
                ) as original_speed_ref_raw:
                    before = session.verify_mode(5)
                    capture = session.capture_triggered(
                        channels,
                        divider=args.sample_divider,
                        trigger_bit=4,
                        timeout_s=args.timeout,
                        validated_snapshot=before,
                    )
        restored = session.drive.snapshot()

    metrics = analyze_pwm_svpwm(
        capture.samples,
        k_pwm=k_pwm,
        math_tolerance_ticks=args.math_tolerance_ticks,
    )
    # Граничный критерий "clamped_fraction" ужесточаем параметром сценария.
    metrics["limits"]["max_clamped_fraction"] = args.max_clamped_fraction
    metrics["criteria"]["no_bus_clamping"] = (
        metrics["clamped_fraction"] <= args.max_clamped_fraction
    )
    metrics["criteria"]["runtime_entered_vector_mode"] = (
        capture.runtime.control_state == 5 and capture.runtime.run_mode == 5
    )
    # PWM_type восстанавливается контекстным менеджером temporary_pwm_type,
    # который поднимает RuntimeError при неудачном восстановлении. Если мы
    # дошли сюда - восстановление прошло успешно.
    metrics["criteria"]["pwm_type_restored"] = True
    metrics["criteria"]["run_mode_restored"] = (
        restored.run_mode == original_run_mode_raw
    )
    metrics["criteria"]["speed_reference_restored"] = (
        restored.speed_ref_raw == original_speed_ref_raw
    )
    metrics["criteria"]["capture_window_covers_run"] = metrics["window_s"] >= 0.2
    metrics["passed"] = all(metrics["criteria"].values())

    csv_path = output / "capture.csv"
    png_path = output / "svpwm-trace.png"
    json_path = output / "report.json"
    # Дополним каждую точку ожидаемой SVPWM-Gamma для удобства анализа.
    expected_a: list[int] = []
    expected_b: list[int] = []
    for sample in capture.samples:
        ua = int(sample.raw["ualpha_norm"])
        ub = int(sample.raw["ubeta_norm"])
        ga, gb, _ = compute_sv_gamma(ua, ub, k_pwm)
        expected_a.append(ga)
        expected_b.append(gb)
    for sample, ea, eb in zip(capture.samples, expected_a, expected_b):
        sample.values["gamma_a_expected"] = float(ea)
        sample.values["gamma_b_expected"] = float(eb)
        sample.raw["gamma_a_expected"] = ea
        sample.raw["gamma_b_expected"] = eb
    write_csv(csv_path, capture.samples)
    plot_pwm_svpwm(png_path, capture.samples)
    report = {
        "test": NAME,
        "firmware": firmware_metadata(paths),
        "hardware": hardware,
        "setup": {
            "required_run_mode": 5,
            "required_pwm_type": PWM_TYPE_SVPWM,
            "pwm_type_name": "PWM_TYPE_SVPWM (symmetric SVPWM)",
            "target_speed_rpm": TARGET_SPEED_RPM,
            "k_pwm": k_pwm,
            "original_pwm_type": original_pwm_type,
            "original_run_mode_raw": original_run_mode_raw,
            "original_speed_reference_raw": original_speed_ref_raw,
            "trigger_bit": 4,
            "trigger_timeout_s": args.timeout,
            "sample_divider": args.sample_divider,
            "channels": [channel.name for channel in channels],
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
        raise RuntimeError("PWM SVPWM acceptance criteria failed.")
