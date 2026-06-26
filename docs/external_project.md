# External Project Usage

`HSS STM32 framework` is intended to be consumed by application repositories.

Applications should own their board directories. The preferred framework dependency path is CMake `FetchContent`; a submodule or copied dependency remains supported when a project needs fully offline source checkout behavior.

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
  cmake/
    arm-gcc-toolchain.cmake
```

Minimal FetchContent `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.20)

project(my_app C ASM)

include(FetchContent)

FetchContent_Declare(
        hss_framework
        GIT_REPOSITORY https://github.com/veskotr/stm32-baremetal-framework.git
        GIT_TAG v0.5.0
)
FetchContent_MakeAvailable(hss_framework)

set(HSS_BOARD_PATHS "${CMAKE_CURRENT_LIST_DIR}/boards" CACHE STRING "HSS board search paths")

hss_add_firmware(my_app
        BOARD my_board
        src/main.c
)
```

Submodule/copied dependency fallback:

```cmake
cmake_minimum_required(VERSION 3.20)

project(my_app C ASM)

set(HSS_BOARD_PATHS "${CMAKE_CURRENT_LIST_DIR}/boards" CACHE STRING "HSS board search paths")

add_subdirectory(third_party/stm32-baremetal-framework)

set(HSS_BOARD_PATHS "${CMAKE_CURRENT_LIST_DIR}/boards" CACHE STRING "HSS board search paths")

hss_add_firmware(my_app
        BOARD my_board
        src/main.c
)
```

Configure FetchContent firmware builds with an app-local ARM GCC toolchain file:

```sh
cmake -S . -B build/debug -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=cmake/arm-gcc-toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Debug
cmake --build build/debug
```

The app-local toolchain file can be a small project-owned file that selects
`arm-none-eabi-gcc`. HSS still supplies board CPU flags, linker scripts,
startup files, HAL include paths, OpenOCD metadata, and post-build artifacts.

Example:

```cmake
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_EXECUTABLE_SUFFIX ".elf")
```

For submodule/copied dependency projects, using the framework-provided toolchain
file directly is also supported:

```sh
cmake -S . -B build/debug -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=third_party/stm32-baremetal-framework/cmake/arm-gcc-toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Debug
cmake --build build/debug
```

CMake requires `CMAKE_TOOLCHAIN_FILE` before the top-level `project()` enables C/ASM languages. Because FetchContent normally downloads HSS after `project()`, FetchContent firmware projects should not rely on the fetched copy for the first-configure toolchain file.

## Host Tests

User projects can enable host tests in a native build:

```cmake
include(FetchContent)

FetchContent_Declare(
        hss_framework
        GIT_REPOSITORY https://github.com/veskotr/stm32-baremetal-framework.git
        GIT_TAG v0.5.0
)
FetchContent_MakeAvailable(hss_framework)

hss_add_host_test(my_app_unit_tests
        SOURCES tests/test_state_machine.c src/state_machine.c
)

hss_add_host_integration_test(my_app_spi_tests
        SOURCES tests/test_spi_flow.c src/sensor_logic.c
        LIBRARIES hss_hal_host
)
```

Configure and run:

```sh
cmake -S . -B build/host-tests -G Ninja -DHSS_ENABLE_TESTS=ON
cmake --build build/host-tests
ctest --test-dir build/host-tests --output-on-failure
```

Use a separate build directory for firmware:

```sh
cmake -S . -B build/firmware -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=cmake/arm-gcc-toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Debug
cmake --build build/firmware
```

## Project Config

Firmware targets can use an HSS config file with optional profile overlays:

```text
my_app/
  CMakeLists.txt
  hss.conf
  hss-dev.conf
  hss-release.conf
```

```cmake
hss_add_firmware(my_app
        BOARD my_board
        CONFIG hss.conf
        src/main.c
)
```

Select active profiles from CMake:

```sh
cmake -S . -B build/dev \
  -DCMAKE_TOOLCHAIN_FILE=third_party/stm32-baremetal-framework/cmake/arm-gcc-toolchain.cmake \
  -DHSS_CONFIG_PROFILES=dev
```

Profiles are loaded after the base config in order. For example,
`-DHSS_CONFIG_PROFILES=release\;hw_rev_b` loads `hss.conf`,
`hss-release.conf`, and `hss-hw_rev_b.conf`, with later files overriding
earlier values.

Profiles that may be absent can be listed as optional:

```sh
-DHSS_CONFIG_PROFILES=release\;local -DHSS_CONFIG_OPTIONAL_PROFILES=local
```

Additional profile files can be loaded explicitly:

```cmake
hss_add_firmware(my_app
        BOARD my_board
        CONFIG hss.conf
        PROFILES release
        PROFILE_FILES boards/rev_b.conf
        src/main.c
)
```

The config generator writes target-local files under the build directory:

- `hss_config.h`
- `hss_config.cmake`
- `hss_config.meta`

Application and firmware code can include:

```c
#include "hss_config.h"
```

Framework keys use the `HSS_` prefix and are validated when known. C-facing
custom application properties should use `APP_` or `CONFIG_`. `APP_` values are
emitted as `CONFIG_APP_<NAME>` macros, so `APP_DEVICE_NAME` becomes
`CONFIG_APP_DEVICE_NAME`. Other custom keys remain available to CMake and
metadata but stay out of `hss_config.h`.

For details, see [`docs/config_system.md`](./config_system.md).

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

## Generic EXTI Roles

Applications can declare named external interrupt inputs in `board_roles.cmake`.
The name after `BOARD_ROLE_EXTI_` belongs to the application; the framework only
generates GPIO/IRQ metadata and a small IRQ shim.

```cmake
set(BOARD_ROLE_EXTI_MAX31865_DRDY PA3)
set(BOARD_ROLE_EXTI_MAX31865_DRDY_ACTIVE_LOW ON)
set(BOARD_ROLE_EXTI_MAX31865_DRDY_TRIGGER falling) # rising, falling, or both
set(BOARD_ROLE_EXTI_MAX31865_DRDY_IRQ_PRIORITY 0)
set(BOARD_ROLE_EXTI_MAX31865_DRDY_IRQ_SUBPRIORITY 0)
```

The generated `hss_board_roles.h` exposes macros such as:

```c
HSS_BOARD_EXTI_MAX31865_DRDY_PIN
HSS_BOARD_EXTI_MAX31865_DRDY_IRQN
HSS_BOARD_EXTI_MAX31865_DRDY_INPUT
```

Firmware can then use the generic helper:

```c
#include "hss_board_roles.h"
#include "hss_exti.h"

hss_exti_input_t drdy = HSS_BOARD_EXTI_MAX31865_DRDY_INPUT;
hss_exti_input_register_callback(&drdy, app_drdy_callback, NULL);
hss_exti_input_enable(&drdy);
```

For boards that declare EXTI roles, the framework generates and compiles the
matching `EXTI*_IRQHandler()` shim for the selected STM32 family. CubeMX should
still configure the pin as GPIO EXTI so HAL sets up the port/EXTI routing.

## Sensor SPI Role Configuration

Existing sensor SPI roles can also declare the attached device's SPI settings:

```cmake
set(BOARD_ROLE_SENSOR_SPI SPI1)
set(BOARD_ROLE_SENSOR_CS PA4)
set(BOARD_ROLE_SENSOR_CS_ACTIVE_LOW ON)
set(BOARD_ROLE_SENSOR_CS_IDLE inactive)
set(BOARD_ROLE_SENSOR_SPI_MODE 3)
set(BOARD_ROLE_SENSOR_SPI_BAUD_PRESCALER 16)
set(BOARD_ROLE_SENSOR_SPI_NSS software)
set(BOARD_ROLE_SENSOR_SPI_FIRST_BIT msb)
```

Firmware can explicitly apply the role configuration:

```c
hss_sensor_spi_deselect();
hss_sensor_spi_configure_for_role();
```

This is intentionally explicit for now. CubeMX still owns the normal SPI
initialization, while the helper gives applications a framework-owned place for
device-specific bring-up corrections.

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
