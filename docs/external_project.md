# External Project Usage

`HSS STM32 framework` is intended to be consumed by application repositories.

Applications should own their board directories. The framework should be a submodule, copied dependency, or CMake `FetchContent` dependency; adding a new board should not require changing the framework repository.

Recommended application shape:

```text
my_app/
  CMakeLists.txt
  boards/
    my_board/
      my_board.ioc
      Core/
      Drivers/
  src/main.c
  third_party/stm32-baremetal-framework/
```

Minimal submodule/copied dependency `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.20)

project(my_app C ASM)

set(HSS_BOARD_PATHS "${CMAKE_CURRENT_LIST_DIR}/boards" CACHE STRING "HSS board search paths")

add_subdirectory(third_party/stm32-baremetal-framework)

hss_add_firmware(my_app
        BOARD my_board
        src/main.c
)
```

Minimal FetchContent `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.20)

project(my_app C ASM)

include(FetchContent)

FetchContent_Declare(
        hss_framework
        GIT_REPOSITORY <framework-repo-url>
        GIT_TAG v0.3.0
)
FetchContent_MakeAvailable(hss_framework)

set(HSS_BOARD_PATHS "${CMAKE_CURRENT_LIST_DIR}/boards" CACHE STRING "HSS board search paths")

hss_add_firmware(my_app
        BOARD my_board
        src/main.c
)
```

Configure with the ARM GCC toolchain file:

```sh
cmake -S . -B build/debug -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=third_party/stm32-baremetal-framework/cmake/arm-gcc-toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Debug
cmake --build build/debug
```

CMake requires `CMAKE_TOOLCHAIN_FILE` before the top-level `project()` enables C/ASM languages. A FetchContent download cannot provide that file for the same first configure unless a separate bootstrap or app-local shim already exists. The official framework toolchain is `arm-none-eabi-gcc`; MCU-specific flags, startup file, linker script, MCU define, HAL paths, CMSIS paths, and OpenOCD target come from generated board metadata.

## Board Lookup

`hss_add_firmware(... BOARD <board> ...)` and `hss_select_board(<board>)` resolve boards in this order:

- absolute board directory path
- relative path containing `/`, resolved from the application source directory
- named board under each directory in `HSS_BOARD_PATHS`
- named board under `examples/boards` for bundled examples and temporary compatibility

`HSS_BOARD_PATHS` is a semicolon-separated CMake list, so applications can point at app-local boards and shared board packs:

```sh
-DHSS_BOARD_PATHS="$PWD/boards;/opt/hss_boards"
```

The framework supports one selected board per build directory. Use separate build directories for different boards.

## Board Sync

Board metadata is generated from CubeMX output. After configuring the project, use the CMake target:

```sh
cmake --build build/debug --target hss_sync_boards
```

If `board_manifest.cmake` is missing and `HSS_BOARD_AUTO_SYNC=ON` (the default), CMake runs the sync tool automatically during configure. Set `-DHSS_BOARD_AUTO_SYNC=OFF` when you want configure to fail instead and run sync manually.

The selected board also syncs before compilation when `HSS_BOARD_SYNC_BEFORE_BUILD=ON` (the default). If CubeMX added or removed source files, re-run CMake configure after sync so CMake reloads `board_manifest.cmake`.

Generated board files:

- `board_manifest.cmake`
- `generated/board_config.h`
- `generated/board_glue.c`
- `board_roles.cmake`

`board_roles.cmake` is created only if missing and is safe to edit.

Generated build outputs:

- `my_app.elf`
- `my_app.bin`
- `my_app.hex`
- `my_app.map`

Generated targets:

- `my_app`
- `flash_my_app`
- `openocd_my_app`
- `vscode_my_app`
- `hss_sync_boards`
- `sync_board_<board>`

On Linux the framework uses the system `openocd` found in `PATH`.
The bundled Windows OpenOCD tools from older framework versions are not used by this CMake scaffold.

To generate VS Code workflow files for the project:

```cmake
hss_add_firmware(my_app
        BOARD my_board
        src/main.c
)

hss_generate_vscode(my_app)
```

or:

```sh
cmake --build build/debug --target vscode_my_app
```

This writes HSS-owned entries to `.vscode/tasks.json` and `.vscode/launch.json`.
See `docs/vscode.md` for details.

For copyable starter projects, see `examples/`.

## Current Verified Examples

These examples are currently expected to build:

- `examples/blue_pill_minimal`
- `examples/stm32g0_minimal`

Their temporary reference boards live in `examples/boards/`.

Build commands:

```sh
cmake -S examples/blue_pill_minimal \
  -B build/examples/blue_pill_minimal \
  -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=$PWD/cmake/arm-gcc-toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Debug
cmake --build build/examples/blue_pill_minimal
```

```sh
cmake -S examples/stm32g0_minimal \
  -B build/examples/stm32g0_minimal \
  -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=$PWD/cmake/arm-gcc-toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Debug
cmake --build build/examples/stm32g0_minimal
```
