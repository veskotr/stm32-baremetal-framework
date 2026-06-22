from __future__ import annotations

import re
from pathlib import Path

from .cubemx import enabled_instances


def _read(path: Path) -> str:
    return path.read_text(encoding="utf-8")


def _write_if_changed(path: Path, original: str, updated: str) -> bool:
    if updated == original:
        return False
    path.write_text(updated, encoding="utf-8")
    return True


def _add_lines_to_user_block(text: str, block: str, lines: list[str]) -> str:
    begin = f"/* USER CODE BEGIN {block} */"
    end = f"/* USER CODE END {block} */"
    all_lines = text.splitlines(keepends=True)
    begin_index = next((index for index, line in enumerate(all_lines) if begin in line), -1)
    end_index = next(
        (index for index, line in enumerate(all_lines[begin_index + 1 :], begin_index + 1) if end in line),
        -1,
    )
    if begin_index < 0 or end_index < 0:
        raise SystemExit(f"error: missing CubeMX user block {block}")

    body = "".join(all_lines[begin_index + 1 : end_index])
    missing = [line for line in lines if line not in body]
    if not missing:
        return text

    insert_lines = [f"{line}\n" for line in missing]
    if body.strip() and not body.endswith("\n\n"):
        insert_lines.insert(0, "\n")
    all_lines[end_index:end_index] = insert_lines
    return "".join(all_lines)


def _replace_irq_user_block(text: str, irq_name: str, lines: list[str]) -> str:
    block = f"{irq_name}_IRQn 0"
    return _add_lines_to_user_block(text, block, lines)


def _uart_handler_name(index: int) -> str:
    return f"uart{index}_irq_handler"


def _uart_irq_name(index: int) -> str:
    return f"USART{index}"


def _uart_define(index: int) -> str:
    return f"BOARD_UART{index}"


def _uart_hook_lines(index: int) -> list[str]:
    handler = _uart_handler_name(index)
    define = _uart_define(index)
    return [
        f"#if {define}",
        f"    if ({handler}())",
        "    {",
        "        return;",
        "    }",
        "#endif",
    ]


def _weak_uart_hook(index: int) -> list[str]:
    handler = _uart_handler_name(index)
    return [
        f"__weak int {handler}(void)",
        "{",
        "    return 0;",
        "}",
    ]


def _systick_hook_lines() -> list[str]:
    return [
        "__attribute__((weak)) void board_systick_hook(void)",
        "{",
        "    /* default: do nothing */",
        "}",
    ]


def _patch_main_header(path: Path, uart_instances: list[int]) -> bool:
    original = _read(path)
    text = original
    if '#include "board_config.h"' not in text:
        text = _add_lines_to_user_block(text, "Includes", ['#include "board_config.h"'])

    prototypes = ["void clock_init(void);", "void board_systick_hook(void);"]
    prototypes.extend(f"int {_uart_handler_name(index)}(void);" for index in uart_instances)
    prototypes = [prototype for prototype in prototypes if prototype not in text]
    text = _add_lines_to_user_block(text, "EFP", prototypes)
    return _write_if_changed(path, original, text)


def _patch_irq_source(path: Path, uart_instances: list[int]) -> bool:
    original = _read(path)
    text = original

    if "void board_systick_hook(void)" not in text:
        text = _add_lines_to_user_block(text, "0", _systick_hook_lines())
    if "board_systick_hook();" not in text:
        text = _add_lines_to_user_block(text, "SysTick_IRQn 1", ["board_systick_hook();"])

    weak_lines: list[str] = []
    for index in uart_instances:
        irq = _uart_irq_name(index)
        if re.search(rf"\bvoid\s+{irq}_IRQHandler\s*\(", text) is None:
            continue
        text = _replace_irq_user_block(text, irq, _uart_hook_lines(index))
        if f"int {_uart_handler_name(index)}(void)" not in text:
            if weak_lines:
                weak_lines.append("")
            weak_lines.extend(_weak_uart_hook(index))

    if weak_lines:
        text = _add_lines_to_user_block(text, "1", weak_lines)

    return _write_if_changed(path, original, text)


def patch_cubemx_user_code(board_dir: Path, values: dict[str, str]) -> list[Path]:
    uart_instances = enabled_instances(values, "USART") + enabled_instances(values, "UART")
    uart_instances = sorted(set(uart_instances))

    changed: list[Path] = []
    main_header = board_dir / "Core/Inc/main.h"
    irq_sources = sorted((board_dir / "Core/Src").glob("stm32*_it.c"))

    if main_header.exists() and _patch_main_header(main_header, uart_instances):
        changed.append(main_header)

    for irq_source in irq_sources:
        if _patch_irq_source(irq_source, uart_instances):
            changed.append(irq_source)

    return changed


def validate_cubemx_user_code(board_dir: Path, values: dict[str, str]) -> None:
    uart_instances = enabled_instances(values, "USART") + enabled_instances(values, "UART")
    uart_instances = sorted(set(uart_instances))

    main_header = board_dir / "Core/Inc/main.h"
    if main_header.exists():
        text = _read(main_header)
        for prototype in ["void clock_init(void);", "void board_systick_hook(void);"]:
            if prototype not in text:
                raise SystemExit(f"error: {main_header} is missing {prototype}")
        for index in uart_instances:
            prototype = f"int {_uart_handler_name(index)}(void);"
            if prototype not in text:
                raise SystemExit(f"error: {main_header} is missing {prototype}")

    for irq_source in sorted((board_dir / "Core/Src").glob("stm32*_it.c")):
        text = _read(irq_source)
        if "board_systick_hook();" not in text:
            raise SystemExit(f"error: {irq_source} does not call board_systick_hook() from SysTick")
        for index in uart_instances:
            irq = _uart_irq_name(index)
            if re.search(rf"\bvoid\s+{irq}_IRQHandler\s*\(", text) is None:
                continue
            hook = f"if ({_uart_handler_name(index)}())"
            if hook not in text:
                raise SystemExit(f"error: {irq_source} is missing {hook}")
