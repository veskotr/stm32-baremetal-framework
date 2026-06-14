# Handoff Notes

This document captures the current state of `framework_v3` for future coding sessions.

## Current State

`framework_v3` now has a working first scaffold:

- CubeMX-generated board projects copied into `framework_v3/boards/`
- board sync tooling under `framework_v3/tools/board_sync/`
- generated board metadata and glue for both current boards
- CMake framework entrypoint at `framework_v3/CMakeLists.txt`
- reusable CMake functions in `framework_v3/cmake/hss_framework.cmake`
- ARM GCC toolchain file in `framework_v3/cmake/arm-gcc-toolchain.cmake`
- first framework-owned HAL glue module under `framework_v3/hal/`
- example firmware projects in `framework_v3/examples/`
- docs for CubeMX sync, external project usage, examples, and testing direction

The framework is C-first. C++ should not be required by the framework core.

## Working Boards

Current boards:

- `blue_pill_temp_transmitter`
- `stm32g071kb_temp_transmitter`

Both boards have been synced with:

```sh
python3 framework_v3/tools/sync_board.py framework_v3/boards/<board>
```

Generated board files:

- `board_manifest.cmake`
- `generated/board_config.h`
- `generated/board_glue.c`
- `board_roles.cmake`

`board_roles.cmake` is framework-owned but manually editable. The sync tool creates it only if missing.

## Verified Builds

The following examples were configured and built successfully on Linux with system `arm-none-eabi-gcc`, Ninja, CMake, and system `openocd` available:

```sh
cmake -S framework_v3/examples/blue_pill_minimal \
  -B /tmp/hss-example-blue \
  -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=framework_v3/cmake/arm-gcc-toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Debug
cmake --build /tmp/hss-example-blue
```

```sh
cmake -S framework_v3/examples/stm32g0_minimal \
  -B /tmp/hss-example-g0 \
  -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=framework_v3/cmake/arm-gcc-toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Debug
cmake --build /tmp/hss-example-g0
```

Both builds generated:

- `.elf`
- `.bin`
- `.hex`
- `.map`
- size output

## Intended User Workflow

For a real application repository, the framework should be added as a submodule or copied dependency.

Example:

```cmake
cmake_minimum_required(VERSION 3.20)

project(my_app C ASM)

add_subdirectory(third_party/framework_v3)

hss_add_firmware(my_app
        BOARD blue_pill_temp_transmitter
        src/main.c
)
```

The framework repo may contain examples and framework tests, but production applications should not be added to this repository.

The current CMake scaffold supports one selected board per build directory. Use separate build directories for different boards.

## Linux OpenOCD Policy

On Linux, use the system `openocd` found in `PATH`.

The bundled Windows OpenOCD tools from older framework versions are not used by `framework_v3` on Linux. Windows support can later add a platform-specific path or generated VS Code settings.

## Next Best Steps

Good next implementation tasks:

1. Expand `hal/` carefully, probably UART/SPI/timer next only after role handling is clearer.
2. Decide which existing v1 helpers should be ported and which should be replaced.
3. Add a small host-test scaffold so C framework logic can run on desktop.
4. Add VS Code task generation for configure/build/sync/flash/openocd.
5. Improve board role handling so integrations can ask for semantic resources instead of raw peripherals.
6. Add memory/reporting improvements beyond `arm-none-eabi-size`.

Keep changes small and verify with both examples after changing CMake, sync tooling, or generated board files.
