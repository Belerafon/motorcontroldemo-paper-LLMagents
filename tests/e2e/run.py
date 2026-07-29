"""Direct entry point for MotorControlDemo hardware end-to-end tests."""

from __future__ import annotations

import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
TOOLKIT = ROOT / ".codex/skills/motorcontrol-demo/scripts"
for path in (ROOT, TOOLKIT):
    value = str(path)
    if value not in sys.path:
        sys.path.insert(0, value)

from tests.e2e.cli import main  # noqa: E402


if __name__ == "__main__":
    main()
