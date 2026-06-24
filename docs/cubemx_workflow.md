# CubeMX Workflow

CubeMX remains the source of truth for board configuration.

CubeMX owns:
- `.ioc`
- `Core/`
- `Drivers/`
- startup files
- linker scripts
- HAL configuration
- interrupt and MSP code

The framework owns:
- board metadata derived from CubeMX output
- semantic role mappings that CubeMX cannot know
- CMake build integration
- build, flash, debug, and report targets

## Expected CubeMX Settings

Use these settings for board projects that will be consumed by the framework:

- Toolchain: `Makefile`
- Generate under root: disabled
- Generate peripheral initialization as separate `.c/.h` files: enabled
- Keep user code: enabled
- Do not generate `main()`: enabled
- Copy required library files into the project or otherwise make `Drivers/` available in the board directory

Application repositories should keep board directories outside the framework tree and point CMake at them with `HSS_BOARD_PATHS`. The temporary reference boards for framework examples live under `examples/boards/`.

## Sync Flow

After creating or regenerating a CubeMX project, run the CMake sync target:

```sh
cmake --build build/debug --target hss_sync_boards
```

If the project has not been configured yet and the board does not have generated metadata, configure with `HSS_BOARD_AUTO_SYNC=ON` or run the underlying sync script once:

```sh
python3 third_party/stm32-baremetal-framework/tools/sync_board.py boards/<board>
```

The wrapper above calls the `tools/board_sync/` package. New automation should follow the same shape: a small command entrypoint with reusable logic in a focused package.

The sync script reads the `.ioc` and generated folder layout, then writes:

- `board_manifest.cmake`: generated framework metadata
- `generated/board_config.h`: generated framework capability defines
- `generated/board_glue.c`: generated compatibility glue such as `clock_init()`, `board_clock_config()`, and `board_init()`
- `board_roles.cmake`: editable semantic role mapping, created only if missing

Regeneration flow:

1. Regenerate the project in CubeMX.
2. Build normally; `HSS_BOARD_SYNC_BEFORE_BUILD=ON` runs board sync before compiling the selected board.
3. If CubeMX added or removed source files, reconfigure CMake so the build graph reloads `board_manifest.cmake`.
4. Review changes to generated metadata.
5. Keep semantic roles in `board_roles.cmake`.

The sync script must not edit CubeMX-generated source files. Framework hooks should be generated into the board `generated/` directory or live in framework-owned files outside CubeMX output.

## Automatic Sync

When CMake selects a board and `board_manifest.cmake` is missing, `HSS_BOARD_AUTO_SYNC=ON` lets configure run the sync tool automatically. Disable it with `-DHSS_BOARD_AUTO_SYNC=OFF` if you prefer a hard failure and a manual sync command.

When `HSS_BOARD_SYNC_BEFORE_BUILD=ON`, the selected board's `sync_board_<board>` target runs before board compilation. `hss_sync_boards` is the stable aggregate target users can run manually without knowing the selected board's generated target suffix. Generated files are only rewritten when content changes, so a normal no-change build should not rebuild just because sync ran.

## Current Reference Boards

Current synced boards:

- `examples/boards/blue_pill_temp_transmitter`
- `examples/boards/stm32g071kb_temp_transmitter`

After regenerating either board in CubeMX, rerun the sync command for that board and rebuild the matching example.
