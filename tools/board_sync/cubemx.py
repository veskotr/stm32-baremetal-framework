from __future__ import annotations

import re
from pathlib import Path


def find_one(board_dir: Path, pattern: str, label: str) -> Path:
    matches = sorted(board_dir.glob(pattern))
    if not matches:
        raise SystemExit(f"error: could not find {label} with pattern {pattern}")
    if len(matches) > 1:
        names = ", ".join(str(match.relative_to(board_dir)) for match in matches)
        raise SystemExit(f"error: expected one {label}, found: {names}")
    return matches[0]


def startup_define(startup: Path) -> str:
    text = startup.read_text(encoding="utf-8", errors="ignore")
    match = re.search(r"STM32[A-Z0-9]+x[A-Z]\b|STM32[A-Z0-9]+xx\b", text)
    if match:
        return match.group(0)

    stem = startup.stem.removeprefix("startup_")
    if stem.endswith("xx"):
        return stem.upper()[:-2] + "xx"
    if stem[-1:].islower():
        return stem[:-1].upper() + stem[-1:].upper()
    return stem.upper()


def startup_cpu(startup: Path) -> str:
    text = startup.read_text(encoding="utf-8", errors="ignore")
    match = re.search(r"^\s*\.cpu\s+([A-Za-z0-9_+-]+)", text, flags=re.MULTILINE)
    if not match:
        raise SystemExit(f"error: could not detect .cpu in {startup}")
    return match.group(1)


def openocd_target_for_family(family: str) -> str:
    family = family.lower()
    if family.startswith("stm32") and len(family) >= 7:
        return f"target/{family[:7]}x.cfg"
    raise SystemExit(f"error: cannot infer OpenOCD target for MCU family {family}")


def enabled_instances(values: dict[str, str], prefix: str) -> list[int]:
    instances: list[int] = []
    for value in values.values():
        match = re.fullmatch(rf"{prefix}([0-9]+)", value)
        if match:
            instances.append(int(match.group(1)))
    return sorted(set(instances))


def gpio_ports(values: dict[str, str]) -> list[str]:
    ports: set[str] = set()
    for key in values:
        match = re.match(r"P([A-K])(?:[0-9]|\\ |-|$)", key)
        if match:
            ports.add(f"GPIO{match.group(1)}")
    return sorted(ports)


def init_functions(values: dict[str, str]) -> list[tuple[str, str]]:
    functions = values.get("ProjectManager.functionlistsort", "")
    result: list[tuple[str, str]] = []
    for item in functions.split(","):
        parts = item.split("-")
        if len(parts) >= 3:
            result.append((parts[1], parts[2]))
    return result
