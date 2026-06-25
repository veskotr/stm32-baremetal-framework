from __future__ import annotations

import argparse
from pathlib import Path

from .model import ConfigError, ConfigWarnings
from .model import SCHEMA
from .parser import load_config
from .render import generated_cmake, generated_header, generated_meta


def write_if_changed(path: Path, content: str) -> None:
    if path.exists() and path.read_text(encoding="utf-8") == content:
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding="utf-8")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Generate HSS config outputs")
    parser.add_argument("--config", type=Path, help="Base config file")
    parser.add_argument("--out-dir", type=Path, help="Generated output directory")
    parser.add_argument("--target", help="Firmware target name")
    parser.add_argument("--profile", action="append", default=[], help="Active profile name")
    parser.add_argument("--optional-profile", action="append", default=[], help="Profile name that may be missing")
    parser.add_argument("--profile-file", action="append", default=[], type=Path, help="Additional profile/config file")
    parser.add_argument("--schema-help", action="store_true", help="Print supported HSS config keys and exit")
    return parser.parse_args()


def print_schema_help() -> None:
    print("Supported HSS config keys:")
    for key in sorted(SCHEMA):
        entry = SCHEMA[key]
        details = [f"type={entry.kind}", f"default={entry.default}"]
        if entry.minimum is not None:
            details.append(f"min={entry.minimum}")
        if entry.maximum is not None:
            details.append(f"max={entry.maximum}")
        print(f"  {key} ({', '.join(details)})")
        print(f"    {entry.description}")


def main() -> int:
    args = parse_args()
    if args.schema_help:
        print_schema_help()
        return 0
    if args.config is None or args.out_dir is None or args.target is None:
        print("hss config error: --config, --out-dir, and --target are required unless --schema-help is used")
        return 1

    base_config = args.config.resolve()
    out_dir = args.out_dir.resolve()
    profiles = args.profile

    try:
        warnings = ConfigWarnings()
        values, loaded_files = load_config(
            base_config,
            profiles,
            warnings,
            optional_profiles=args.optional_profile,
            profile_files=args.profile_file,
        )
        for warning in warnings.messages:
            print(warning)
        write_if_changed(out_dir / "hss_config.h", generated_header(values, profiles))
        write_if_changed(out_dir / "hss_config.cmake", generated_cmake(values, profiles, loaded_files))
        write_if_changed(out_dir / "hss_config.meta", generated_meta(values, profiles, loaded_files))
    except ConfigError as exc:
        print(f"hss config error: {exc}")
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
