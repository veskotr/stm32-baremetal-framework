from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path

HSS_EEPROM_RECORD_SIZE_BYTES = 8


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
    "HSS_ENABLE_EEPROM_EMULATION": SchemaEntry(
        "bool",
        False,
        "Enable flash-backed EEPROM emulation support for this firmware target.",
    ),
    "HSS_CONSOLE_STDIO_TIMEOUT_MS": SchemaEntry(
        "int",
        1000,
        "Timeout used by framework stdio wrappers for console UART reads/writes.",
        minimum=0,
    ),
    "HSS_EEPROM_FLASH_ORIGIN": SchemaEntry(
        "int",
        0,
        "Flash origin address reserved for EEPROM emulation.",
        minimum=0,
    ),
    "HSS_EEPROM_FLASH_SIZE": SchemaEntry(
        "int",
        0,
        "Total flash size reserved for EEPROM emulation.",
        minimum=0,
    ),
    "HSS_EEPROM_PAGE_SIZE": SchemaEntry(
        "int",
        0,
        "Flash page size used by the EEPROM emulation region.",
        minimum=0,
    ),
    "HSS_EEPROM_SLOT_COUNT": SchemaEntry(
        "int",
        0,
        "Maximum number of distinct EEPROM keys cached in RAM.",
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
