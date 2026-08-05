from __future__ import annotations

import argparse
from pathlib import Path

from motor_debug_core.project import ProjectPaths

from . import current_mode4, power_limit, vector_500


SCENARIOS = {
    current_mode4.NAME: current_mode4,
    power_limit.NAME: power_limit,
    vector_500.NAME: vector_500,
}


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="MotorControlDemo hardware end-to-end tests"
    )
    parser.add_argument(
        "--vector-ide",
        type=Path,
        default=Path(r"C:\VectorIDEv1.3.3"),
    )
    commands = parser.add_subparsers(dest="command", required=True)
    for name, scenario in SCENARIOS.items():
        subparser = commands.add_parser(name, help=scenario.HELP)
        scenario.register(subparser)
    return parser


def main() -> None:
    args = build_parser().parse_args()
    paths = ProjectPaths.discover(Path(__file__), args.vector_ide)
    SCENARIOS[args.command].run(args, paths)
