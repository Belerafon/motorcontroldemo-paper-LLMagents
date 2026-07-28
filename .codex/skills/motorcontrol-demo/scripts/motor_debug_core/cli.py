from __future__ import annotations

import argparse
from pathlib import Path

from .drive import Drive
from .project import Project, ProjectPaths
from .scenarios import SCENARIOS
from .target import SymbolTable, Target
from .transport import OpenOcdTransport


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="MotorControlDemo debug toolkit")
    parser.add_argument(
        "--vector-ide", type=Path, default=Path(r"C:\VectorIDEv1.3.3")
    )
    commands = parser.add_subparsers(dest="command", required=True)
    build = commands.add_parser("build", help="Build the current Debug firmware.")
    build.add_argument("--incremental", action="store_true")
    build.add_argument("--jobs", type=int, default=8)
    commands.add_parser("flash", help="Program and verify the current ELF.")
    commands.add_parser("status", help="Read safe drive status without halting.")
    for scenario_type in SCENARIOS.values():
        scenario_parser = commands.add_parser(
            scenario_type.name, help=scenario_type.help
        )
        scenario_type.register(scenario_parser)
    return parser


def main() -> None:
    args = build_parser().parse_args()
    paths = ProjectPaths.discover(Path(__file__), args.vector_ide)
    project = Project(paths)
    if args.command == "build":
        project.build(incremental=args.incremental, jobs=args.jobs)
    elif args.command == "flash":
        project.flash()
    elif args.command == "status":
        with OpenOcdTransport(paths) as transport:
            target = Target(transport, SymbolTable(paths))
            print(Drive(target).snapshot(wait_ready=True).describe())
    else:
        SCENARIOS[args.command]().run(project, args)


if __name__ == "__main__":
    main()
