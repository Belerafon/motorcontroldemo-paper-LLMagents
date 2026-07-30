from __future__ import annotations

import xml.etree.ElementTree as ET
from pathlib import Path

from .values import FixedPoint, Float32, SignedInt32, ValueCodec


class ObjectDictionary:
    """Reads value formats and engineering scales from the project cood.xml."""

    def __init__(self, path: Path):
        root = ET.parse(path).getroot()
        scale_numbers = root.find("scale_numbers")
        if scale_numbers is None:
            raise RuntimeError("cood.xml does not contain scale_numbers.")
        self.scales = {
            int(element.tag.split("_", 1)[1]): float(element.attrib["possible_value"])
            for element in scale_numbers
            if element.attrib.get("possible_value")
        }
        self.formats: dict[str, dict[str, str]] = {}
        for element in root.iter():
            address = element.attrib.get("addr")
            format_element = element.find("format")
            if address and format_element is not None:
                self.formats[address] = dict(format_element.attrib)

    def codec(self, expression: str, *, unit: str = "") -> ValueCodec:
        try:
            value_format = self.formats[expression]
        except KeyError as error:
            raise KeyError(f"No object-dictionary format for {expression!r}.") from error
        format_type = value_format.get("Type")
        if format_type == "IQ":
            q = int(value_format["QValue"])
            scale = self.scales[int(value_format["ScaleNum"])]
            return FixedPoint(q=q, scale=scale, unit=unit)
        if format_type == "Float":
            return Float32(unit=unit)
        if format_type == "StandartNum":
            return SignedInt32(unit=unit)
        raise ValueError(
            f"Unsupported format {format_type!r} for {expression!r}."
        )
