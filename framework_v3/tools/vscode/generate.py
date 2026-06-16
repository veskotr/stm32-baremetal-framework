from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any


HSS_PREFIX = "HSS:"


def load_json_object(path: Path) -> dict[str, Any]:
    if not path.exists():
        return {}
    with path.open("r", encoding="utf-8") as handle:
        data = json.load(handle)
    if not isinstance(data, dict):
        raise SystemExit(f"error: {path} does not contain a JSON object")
    return data


def write_json(path: Path, data: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8") as handle:
        json.dump(data, handle, indent=4)
        handle.write("\n")


def items_without_generated_names(items: Any, key: str, generated_names: set[str]) -> list[dict[str, Any]]:
    if not isinstance(items, list):
        return []

    kept: list[dict[str, Any]] = []
    for item in items:
        if not isinstance(item, dict):
            continue
        value = item.get(key)
        if not isinstance(value, str) or value not in generated_names:
            kept.append(item)
    return kept


def make_tasks(args: argparse.Namespace) -> list[dict[str, Any]]:
    configure_args = [
        "-S",
        args.source_dir,
        "-B",
        args.build_dir,
        f"-DCMAKE_TOOLCHAIN_FILE={args.toolchain_file}",
        f"-DCMAKE_BUILD_TYPE={args.build_type}",
    ]
    if args.generator:
        configure_args.extend(["-G", args.generator])

    return [
        {
            "label": f"{HSS_PREFIX} configure {args.target}",
            "type": "process",
            "command": "cmake",
            "args": configure_args,
            "problemMatcher": [],
        },
        {
            "label": f"{HSS_PREFIX} build {args.target}",
            "type": "process",
            "command": "cmake",
            "args": ["--build", args.build_dir, "--target", args.target],
            "group": {"kind": "build", "isDefault": True},
            "problemMatcher": ["$gcc"],
        },
        {
            "label": f"{HSS_PREFIX} flash {args.target}",
            "type": "process",
            "command": "cmake",
            "args": ["--build", args.build_dir, "--target", f"flash_{args.target}"],
            "dependsOn": f"{HSS_PREFIX} build {args.target}",
            "problemMatcher": [],
        },
        {
            "label": f"{HSS_PREFIX} openocd {args.target}",
            "type": "process",
            "command": "cmake",
            "args": ["--build", args.build_dir, "--target", f"openocd_{args.target}"],
            "dependsOn": f"{HSS_PREFIX} build {args.target}",
            "problemMatcher": [],
        },
        {
            "label": f"{HSS_PREFIX} sync board {args.board}",
            "type": "process",
            "command": "cmake",
            "args": ["--build", args.build_dir, "--target", f"sync_board_{args.board_target_suffix}"],
            "problemMatcher": [],
        },
    ]


def make_launch_config(args: argparse.Namespace) -> dict[str, Any]:
    return {
        "name": f"{HSS_PREFIX} Debug {args.target}",
        "type": "cortex-debug",
        "request": "launch",
        "cwd": "${workspaceFolder}",
        "executable": args.elf,
        "servertype": "openocd",
        "serverpath": args.openocd,
        "gdbPath": args.gdb,
        "configFiles": [args.openocd_interface, args.openocd_target],
        "serverArgs": ["-c", f"transport select {args.openocd_transport}"],
        "runToEntryPoint": "main",
        "preLaunchTask": f"{HSS_PREFIX} build {args.target}",
        "postLaunchCommands": ["monitor reset halt"],
        "showDevDebugOutput": "none",
    }


def update_tasks(path: Path, args: argparse.Namespace) -> None:
    data = load_json_object(path)
    generated_tasks = make_tasks(args)
    generated_labels = {task["label"] for task in generated_tasks}
    tasks = items_without_generated_names(data.get("tasks"), "label", generated_labels)
    tasks.extend(generated_tasks)

    data["version"] = "2.0.0"
    data["tasks"] = tasks
    write_json(path, data)


def update_launch(path: Path, args: argparse.Namespace) -> None:
    data = load_json_object(path)
    generated_config = make_launch_config(args)
    configs = items_without_generated_names(data.get("configurations"), "name", {generated_config["name"]})
    configs.append(generated_config)

    data["version"] = "0.2.0"
    data["configurations"] = configs
    write_json(path, data)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Generate VS Code workflow files for an HSS firmware target.")
    parser.add_argument("--workspace", required=True)
    parser.add_argument("--source-dir", required=True)
    parser.add_argument("--build-dir", required=True)
    parser.add_argument("--target", required=True)
    parser.add_argument("--board", required=True)
    parser.add_argument("--board-target-suffix", required=True)
    parser.add_argument("--elf", required=True)
    parser.add_argument("--toolchain-file", required=True)
    parser.add_argument("--build-type", default="Debug")
    parser.add_argument("--generator", default="")
    parser.add_argument("--openocd", default="openocd")
    parser.add_argument("--openocd-interface", required=True)
    parser.add_argument("--openocd-target", required=True)
    parser.add_argument("--openocd-transport", default="swd")
    parser.add_argument("--gdb", default="arm-none-eabi-gdb")
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    vscode_dir = Path(args.workspace) / ".vscode"
    update_tasks(vscode_dir / "tasks.json", args)
    update_launch(vscode_dir / "launch.json", args)


if __name__ == "__main__":
    main()
