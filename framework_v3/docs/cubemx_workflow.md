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

The existing board directories under `framework_v3/boards/` are the current reference inputs.

## Sync Flow

After creating or regenerating a CubeMX project, run:

```sh
python3 framework_v3/tools/sync_board.py framework_v3/boards/<board>
```

The wrapper above calls the `tools/board_sync/` package. New automation should follow the same shape: a small command entrypoint with reusable logic in a focused package.

The sync script reads the `.ioc` and generated folder layout, then writes:

- `board_manifest.cmake`: generated framework metadata
- `generated/board_config.h`: generated framework capability defines
- `generated/board_glue.c`: generated compatibility glue such as `clock_init()`, `board_clock_config()`, and `board_init()`
- `board_roles.cmake`: editable semantic role mapping, created only if missing

Regeneration flow:

1. Regenerate the project in CubeMX.
2. Run the sync script.
3. Review changes to generated metadata.
4. Keep semantic roles in `board_roles.cmake`.

The sync script must not edit CubeMX-generated source files. Framework hooks should be generated into the board `generated/` directory or live in framework-owned files outside CubeMX output.

## Current Boards

Current synced boards:

- `framework_v3/boards/blue_pill_temp_transmitter`
- `framework_v3/boards/stm32g071kb_temp_transmitter`

After regenerating either board in CubeMX, rerun the sync command for that board and rebuild the matching example.
