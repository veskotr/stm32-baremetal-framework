from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path


@dataclass(frozen=True)
class SchemaEntry:
    kind: str
    default: object
    description: str
    minimum: int | None = None
    maximum: int | None = None


SCHEMA = {
    "HSS_ENABLE_FREEMODBUS": SchemaEntry(
        "bool",
        False,
        "Enable the FreeModbus protocol integration for this firmware target.",
    ),
    "HSS_ENABLE_MAX31865": SchemaEntry(
        "bool",
        False,
        "Enable the MAX31865 RTD driver integration for this firmware target.",
    ),
    "HSS_ENABLE_MODBUS_DEBUG": SchemaEntry(
        "bool",
        False,
        "Enable optional Modbus debug counters and related debug state.",
    ),
    "HSS_CONSOLE_STDIO_TIMEOUT_MS": SchemaEntry(
        "int",
        1000,
        "Timeout used by framework stdio wrappers for console UART reads/writes.",
        minimum=0,
    ),
}


@dataclass(frozen=True)
class SourceLocation:
    path: Path
    line: int


@dataclass(frozen=True)
class ConfigValue:
    key: str
    value: object
    kind: str
    source: SourceLocation


class ConfigError(Exception):
    pass


class ConfigWarnings:
    def __init__(self) -> None:
        self.messages: list[str] = []

    def warn(self, path: Path, line: int, message: str) -> None:
        self.messages.append(f"{path}:{line}: warning: {message}")
