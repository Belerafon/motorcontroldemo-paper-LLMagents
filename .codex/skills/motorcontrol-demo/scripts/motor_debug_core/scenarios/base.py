from __future__ import annotations

from abc import ABC, abstractmethod
from argparse import ArgumentParser, Namespace

from ..project import Project


class Scenario(ABC):
    name: str
    help: str

    @classmethod
    def register(cls, parser: ArgumentParser) -> None:
        parser.add_argument(
            "--start-drive",
            action="store_true",
            help="Explicitly authorize the scenario to start the drive.",
        )

    @abstractmethod
    def run(self, project: Project, args: Namespace) -> None: ...
