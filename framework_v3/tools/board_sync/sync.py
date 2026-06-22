from __future__ import annotations

from pathlib import Path

from .cubemx import find_one, startup_cpu
from .cubemx_patch import patch_cubemx_user_code, validate_cubemx_user_code
from .generators import (
    discover_cubemx_init_functions,
    write_board_config,
    write_board_glue,
    write_manifest,
    write_roles,
)
from .ioc import read_ioc


def sync_board(board_dir: Path) -> None:
    board_dir = board_dir.resolve()
    ioc_files = sorted(board_dir.glob("*.ioc"))
    if len(ioc_files) != 1:
        names = ", ".join(path.name for path in ioc_files) or "none"
        raise SystemExit(f"error: expected one .ioc file in {board_dir}, found {names}")

    ioc = ioc_files[0]
    values = read_ioc(ioc)
    family = values.get("Mcu.Family")
    if not family:
        raise SystemExit(f"error: {ioc} does not define Mcu.Family")

    startup = find_one(board_dir, "startup_*.s", "startup file")
    linker = find_one(board_dir, "*.ld", "linker script")
    hal_driver_dirs = sorted((board_dir / "Drivers").glob("STM32*xx_HAL_Driver"))
    if len(hal_driver_dirs) != 1:
        names = ", ".join(str(path.relative_to(board_dir)) for path in hal_driver_dirs) or "none"
        raise SystemExit(f"error: expected one HAL driver directory, found {names}")

    device_dirs = sorted((board_dir / "Drivers/CMSIS/Device/ST").glob("STM32*xx"))
    if len(device_dirs) != 1:
        names = ", ".join(str(path.relative_to(board_dir)) for path in device_dirs) or "none"
        raise SystemExit(f"error: expected one CMSIS device directory, found {names}")

    core_sources = sorted(path.relative_to(board_dir).as_posix() for path in (board_dir / "Core/Src").glob("*.c"))
    ips = [values[f"Mcu.IP{i}"] for i in range(int(values.get("Mcu.IPNb", "0"))) if f"Mcu.IP{i}" in values]

    cpu = startup_cpu(startup)
    cpu_flags = [f"-mcpu={cpu}", "-mthumb"]
    if cpu in {"cortex-m0", "cortex-m0plus"}:
        cpu_flags.append("-mfloat-abi=soft")

    board_config = write_board_config(board_dir, values)
    board_glue = write_board_glue(board_dir, values)
    patched = patch_cubemx_user_code(board_dir, values)
    manifest = write_manifest(
        board_dir=board_dir,
        values=values,
        ioc=ioc,
        startup=startup,
        linker=linker,
        hal_driver_dir=hal_driver_dirs[0],
        device_dir=device_dirs[0],
        cpu=cpu,
        cpu_flags=cpu_flags,
        core_sources=core_sources,
        ips=ips,
        board_glue=board_glue,
    )
    roles = write_roles(board_dir, values)
    validate_cubemx_user_code(board_dir, values)

    board_glue_text = board_glue.read_text(encoding="utf-8")
    for function, _, _ in discover_cubemx_init_functions(board_dir):
        if function != "SystemClock_Config" and f"{function}();" not in board_glue_text:
            raise SystemExit(f"error: {board_glue} is missing {function}()")

    print(f"wrote {manifest}")
    print(f"wrote {board_config}")
    print(f"wrote {board_glue}")
    for path in patched:
        print(f"patched {path}")
    print(f"kept {roles}")
