from __future__ import annotations

import re
from pathlib import Path
from typing import Iterable, Sequence

from .model import ConfigError, ConfigValue, ConfigWarnings, SCHEMA, SourceLocation


KEY_RE = re.compile(r"^[A-Z][A-Z0-9_]*$")
PROFILE_RE = re.compile(r"^[a-z0-9_]+$")
UNQUOTED_STRING_RE = re.compile(r"^[A-Za-z0-9_./:+-]+$")

BOOL_VALUES = {
    "y": True,
    "yes": True,
    "true": True,
    "on": True,
    "n": False,
    "no": False,
    "false": False,
    "off": False,
}


def strip_comment(line: str) -> str:
    in_string = False
    escaped = False
    output: list[str] = []
    for char in line:
        if escaped:
            output.append(char)
            escaped = False
            continue
        if char == "\\" and in_string:
            output.append(char)
            escaped = True
            continue
        if char == '"':
            in_string = not in_string
            output.append(char)
            continue
        if char == "#" and not in_string:
            break
        output.append(char)
    return "".join(output).strip()


def parse_string(raw: str, source: SourceLocation) -> str:
    if len(raw) < 2 or not raw.startswith('"') or not raw.endswith('"'):
        raise ConfigError(f"{source.path}:{source.line}: invalid quoted string")
    try:
        return bytes(raw[1:-1], "utf-8").decode("unicode_escape")
    except UnicodeDecodeError as exc:
        raise ConfigError(f"{source.path}:{source.line}: invalid string escape: {exc}") from exc


def infer_value(raw: str, source: SourceLocation) -> tuple[object, str]:
    lowered = raw.lower()
    if lowered in BOOL_VALUES:
        return BOOL_VALUES[lowered], "bool"
    if raw.startswith('"') or raw.endswith('"'):
        return parse_string(raw, source), "string"
    if re.fullmatch(r"[+-]?[0-9]+", raw):
        return int(raw, 10), "int"
    if re.fullmatch(r"0[xX][0-9A-Fa-f]+", raw):
        return int(raw, 16), "int"
    if UNQUOTED_STRING_RE.fullmatch(raw):
        return raw, "string"
    raise ConfigError(f"{source.path}:{source.line}: could not parse value '{raw}'")


def coerce_schema_value(key: str, value: object, kind: str, source: SourceLocation) -> tuple[object, str]:
    schema = SCHEMA.get(key)
    if schema is None:
        if key.startswith("HSS_"):
            raise ConfigError(f"{source.path}:{source.line}: unknown HSS config key '{key}'")
        return value, kind
    if schema.kind == kind:
        coerced_value = value
    elif schema.kind == "bool" and kind == "int" and value in (0, 1):
        return bool(value), "bool"
    else:
        raise ConfigError(
            f"{source.path}:{source.line}: '{key}' expects {schema.kind}, got {kind}"
        )

    if schema.kind == "int":
        int_value = int(coerced_value)
        if schema.minimum is not None and int_value < schema.minimum:
            raise ConfigError(
                f"{source.path}:{source.line}: '{key}' must be >= {schema.minimum}"
            )
        if schema.maximum is not None and int_value > schema.maximum:
            raise ConfigError(
                f"{source.path}:{source.line}: '{key}' must be <= {schema.maximum}"
            )
    return coerced_value, schema.kind


def parse_file(path: Path, warnings: ConfigWarnings | None = None) -> dict[str, ConfigValue]:
    values: dict[str, ConfigValue] = {}
    for line_number, raw_line in enumerate(path.read_text(encoding="utf-8").splitlines(), start=1):
        line = strip_comment(raw_line)
        if not line:
            continue
        if "=" not in line:
            if warnings is not None:
                warnings.warn(path, line_number, "ignored line; expected KEY=VALUE")
                continue
            raise ConfigError(f"{path}:{line_number}: expected KEY=VALUE")
        key, raw_value = line.split("=", 1)
        key = key.strip()
        raw_value = raw_value.strip()
        if not KEY_RE.fullmatch(key):
            if warnings is not None:
                warnings.warn(path, line_number, f"ignored line with invalid config key '{key}'")
                continue
            raise ConfigError(f"{path}:{line_number}: invalid config key '{key}'")
        if raw_value == "":
            if warnings is not None:
                warnings.warn(path, line_number, f"ignored line with missing value for '{key}'")
                continue
            raise ConfigError(f"{path}:{line_number}: missing value for '{key}'")
        source = SourceLocation(path=path, line=line_number)
        try:
            value, kind = infer_value(raw_value, source)
            value, kind = coerce_schema_value(key, value, kind, source)
        except ConfigError as exc:
            if warnings is not None:
                warnings.warn(path, line_number, f"ignored line; {exc}")
                continue
            raise
        values[key] = ConfigValue(key=key, value=value, kind=kind, source=source)
    return values


def profile_path(base_config: Path, profile: str) -> Path:
    return base_config.with_name(f"{base_config.stem}-{profile}{base_config.suffix}")


def resolve_profile_file(base_config: Path, profile_file: Path) -> Path:
    if profile_file.is_absolute():
        return profile_file
    return base_config.parent / profile_file


def load_config(
    base_config: Path,
    profiles: Iterable[str],
    warnings: ConfigWarnings | None = None,
    optional_profiles: Iterable[str] = (),
    profile_files: Sequence[Path] = (),
) -> tuple[dict[str, ConfigValue], list[Path]]:
    if not base_config.exists():
        raise ConfigError(f"config file not found: {base_config}")

    loaded_files = [base_config]
    merged = parse_file(base_config, warnings)
    optional_profile_set = set(optional_profiles)
    for profile in profiles:
        if not PROFILE_RE.fullmatch(profile):
            raise ConfigError(
                f"invalid profile '{profile}'; use lowercase letters, numbers, and underscores"
            )
        path = profile_path(base_config, profile)
        if not path.exists():
            if profile in optional_profile_set:
                if warnings is not None:
                    warnings.warn(path, 0, f"optional profile '{profile}' not found")
                continue
            raise ConfigError(f"selected profile '{profile}' is missing config file: {path}")
        loaded_files.append(path)
        merged.update(parse_file(path, warnings))
    for profile_file in profile_files:
        path = resolve_profile_file(base_config, profile_file)
        if not path.exists():
            raise ConfigError(f"profile file not found: {path}")
        loaded_files.append(path)
        merged.update(parse_file(path, warnings))
    apply_defaults(merged)
    return merged, loaded_files


def apply_defaults(values: dict[str, ConfigValue]) -> None:
    default_source = SourceLocation(path=Path("<default>"), line=0)
    for key, schema in SCHEMA.items():
        if key not in values:
            values[key] = ConfigValue(
                key=key,
                value=schema.default,
                kind=schema.kind,
                source=default_source,
            )
