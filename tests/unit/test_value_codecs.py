from __future__ import annotations

import sys
import unittest
from pathlib import Path


TOOLKIT = (
    Path(__file__).resolve().parents[2]
     / ".opencode/skills/motorcontrol-demo/scripts"
)
sys.path.insert(0, str(TOOLKIT))

from motor_debug_core.values import FixedPoint, Float32, SignedInt32  # noqa: E402


class ValueCodecTests(unittest.TestCase):
    def test_fixed_point_round_trip_in_engineering_units(self) -> None:
        codec = FixedPoint(q=24, scale=500.0, unit="rpm")

        for value in (-500.0, -0.25, 0.0, 500.0):
            self.assertAlmostEqual(codec.decode(codec.encode(value)), value, places=4)

    def test_float_round_trip(self) -> None:
        codec = Float32()

        for value in (-1.5, 0.0, 3.25):
            self.assertAlmostEqual(codec.decode(codec.encode(value)), value, places=6)

    def test_signed_integer_round_trip_with_scale(self) -> None:
        codec = SignedInt32(scale=0.5)

        for value in (-2.0, 0.0, 2.0):
            self.assertEqual(codec.decode(codec.encode(value)), value)

    def test_fixed_point_rejects_overflow(self) -> None:
        codec = FixedPoint(q=24, scale=1.0)

        with self.assertRaises(ValueError):
            codec.encode(128.0)


if __name__ == "__main__":
    unittest.main()
