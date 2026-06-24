# HSS STM32 Bare-Metal Framework

HSS is a small STM32 HAL-based firmware framework. It sits between STM32CubeMX-generated board code and application code, so an application can reuse the same build, flash, debug, board-role, driver, and protocol helpers across projects.

The framework is intentionally STM32-only. It does not try to hide the STM32 HAL; it uses CubeMX as the source of truth for clocks, pins, startup code, linker scripts, peripheral init, HAL config, interrupts, and MSP code.

Current version: see [`version.txt`](./version.txt).

## What You Get

- CMake firmware target helpers: `hss_add_firmware()`, `hss_sync_boards`, `flash_<target>`, `openocd_<target>`, `vscode_<target>`, and `sync_board_<board>`
- user-owned board directories discovered with `HSS_BOARD_PATHS`
- automatic board metadata generation from CubeMX output
- one ARM GCC toolchain file for STM32 bare-metal builds
- generated board roles such as status LED, console UART, Modbus UART/timer, RS-485 DE, and sensor SPI
- HAL helpers for platform init, GPIO, EXTI dispatch, UART, console stdio, SPI, timers, status LED, and time delay
- optional FreeModbus integration and a native MAX31865 driver
- example boards and firmware projects under [`examples/`](./examples/)

## Quick Start: Blink A LED

This is the shortest intended workflow for a new application:

1. Create a CubeMX project for your board.
2. Generate CubeMX code into `boards/<board_name>/`.
3. Point CMake at that board folder with `HSS_BOARD_PATHS`.
4. Build a tiny app that calls `hss_platform_init()` and `hss_status_led_toggle()`.

Example app layout:

```text
my_app/
  CMakeLists.txt
  boards/
    my_board/
      my_board.ioc
      Core/
      Drivers/
      startup_*.s
      *.ld
  src/
    main.c
  third_party/
    stm32-baremetal-framework/
```

Minimal `CMakeLists.txt`:

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

Minimal `src/main.c`:

```c
#include "hss_hal.h"
#include "main.h"

int main(void)
{
    if (hss_platform_init() != HSS_OK) {
        Error_Handler();
    }

    while (1) {
        if (hss_status_led_toggle() != HSS_OK) {
            Error_Handler();
        }
        hss_delay_ms(500);
    }
}
```

Configure and build:

```sh
cmake -S . -B build/debug -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=third_party/stm32-baremetal-framework/cmake/arm-gcc-toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Debug
cmake --build build/debug
```

Flash:

```sh
cmake --build build/debug --target flash_my_app
```

The build creates:

- `my_app.elf`
- `my_app.bin`
- `my_app.hex`
- `my_app.map`

## CubeMX Setup

Create a normal STM32CubeMX project for your board. Use CubeMX to configure:

- MCU or board selection
- clock tree
- SWD/JTAG debug pins
- GPIO output for a status LED
- optional UART for console/debug
- optional UART/timer for Modbus
- optional SPI and chip-select GPIO for sensors

Recommended CubeMX project settings:

- **Toolchain / IDE**: `Makefile`
- **Generate under root**: disabled
- **Generate peripheral initialization as separate `.c/.h` files**: enabled
- **Keep user code**: enabled
- **Do not generate `main()`**: enabled, if available in your CubeMX version
- **Copy required library files into the project**: enabled, or otherwise make `Drivers/` available inside the board directory

Put the CubeMX project output in your application board folder:

```text
my_app/boards/my_board/
  my_board.ioc
  Core/
  Drivers/
  startup_stm32xxxx.s
  STM32xxxx_FLASH.ld
```

After the first configure, run board sync through CMake:

```sh
cmake --build build/debug --target hss_sync_boards
```

The sync tool writes:

- `board_manifest.cmake`: generated build metadata
- `generated/board_config.h`: generated board capability defines
- `generated/board_glue.c`: generated calls into CubeMX init functions
- `board_roles.cmake`: editable semantic role mapping, created only if missing

CMake also runs sync automatically when `board_manifest.cmake` is missing and `HSS_BOARD_AUTO_SYNC=ON` (the default). The selected board also gets synced before compilation when `HSS_BOARD_SYNC_BEFORE_BUILD=ON` (the default), so normal CubeMX regeneration is picked up by the next build.

If you need to sync before the first CMake configure, the same implementation can still be run directly:

```sh
python3 third_party/stm32-baremetal-framework/tools/sync_board.py boards/my_board
```

If CubeMX adds or removes source files, re-run CMake configure after sync so the build graph can see the changed source list:

```sh
cmake -S . -B build/debug
```

## Board Roles

CubeMX knows pins and peripherals. The framework also needs semantic roles: which pin is the status LED, which UART is the console, which SPI bus is the sensor bus, and so on.

Edit `boards/my_board/board_roles.cmake` after the first sync:

```cmake
# Status LED
set(BOARD_ROLE_STATUS_LED PC13)
set(BOARD_ROLE_STATUS_LED_ACTIVE_LOW ON)

# Console UART for printf/scanf
set(BOARD_ROLE_CONSOLE_UART USART2)

# Optional Modbus RTU roles
set(BOARD_ROLE_MODBUS_UART USART1)
set(BOARD_ROLE_MODBUS_TIMER TIM2)

# Optional manual RS-485 driver-enable pin
set(BOARD_ROLE_MODBUS_RS485_MODE manual)
set(BOARD_ROLE_MODBUS_RS485_DE PA8)
set(BOARD_ROLE_MODBUS_RS485_DE_ACTIVE_HIGH ON)

# Optional sensor SPI role
set(BOARD_ROLE_SENSOR_SPI SPI1)
set(BOARD_ROLE_SENSOR_CS PA4)
set(BOARD_ROLE_SENSOR_CS_ACTIVE_LOW ON)
```

Only enable roles that CubeMX actually generated. For example, do not set `BOARD_ROLE_CONSOLE_UART USART2` unless CubeMX generated `huart2`.

## Board Lookup

Most applications have one board, but variations are useful. Keep them side by side:

```text
my_app/boards/
  rev_a/
  rev_b/
  factory_test/
```

Select one board per build directory:

```cmake
hss_add_firmware(my_app
        BOARD rev_a
        src/main.c
)
```

Lookup order for `BOARD <value>`:

- absolute board directory path
- relative path containing `/`, resolved from the application source directory
- named board under each directory in `HSS_BOARD_PATHS`
- named board under `examples/boards` for framework examples and temporary compatibility

`HSS_BOARD_PATHS` is a semicolon-separated CMake list:

```sh
cmake -S . -B build/rev_a \
  -DHSS_BOARD_PATHS="$PWD/boards;/opt/shared_hss_boards" \
  -DCMAKE_TOOLCHAIN_FILE=third_party/stm32-baremetal-framework/cmake/arm-gcc-toolchain.cmake
```

Use a different build directory for each selected board:

```sh
cmake -S . -B build/rev_a -DHSS_BOARD_PATHS=$PWD/boards
cmake -S . -B build/rev_b -DHSS_BOARD_PATHS=$PWD/boards
```

## FetchContent Usage

The framework can be consumed with CMake `FetchContent`:

```cmake
include(FetchContent)

FetchContent_Declare(
        hss_framework
        GIT_REPOSITORY <framework-repo-url>
        GIT_TAG v0.3.0
)
FetchContent_MakeAvailable(hss_framework)
```

Important CMake detail: `CMAKE_TOOLCHAIN_FILE` must exist before the top-level `project()` enables C/ASM languages. A dependency fetched during configure cannot provide that toolchain file for the same first configure. For the smoothest first project, use a submodule/copied framework or an app-local toolchain shim; a future setup helper or VS Code extension can automate that.

The official framework toolchain is:

```text
cmake/arm-gcc-toolchain.cmake
```

MCU-specific settings live in generated board metadata, not in separate toolchain files.

## OpenOCD And VS Code

If OpenOCD is installed, each firmware target gets:

```sh
cmake --build build/debug --target flash_my_app
cmake --build build/debug --target openocd_my_app
```

OpenOCD interface and target files come from the generated board manifest. The sync tool chooses a default target from the MCU family; edit generated metadata only if your board needs a special OpenOCD setup.

VS Code files can be generated after creating the firmware target:

```cmake
hss_generate_vscode(my_app)
```

or by using:

```cmake
hss_add_firmware(my_app
        BOARD my_board
        GENERATE_VSCODE
        src/main.c
)
```

See [`docs/vscode.md`](./docs/vscode.md) for details.

## Examples

Example firmware projects live in [`examples/`](./examples/):

- `hello_blink`: copyable starter project for platform init plus status LED blink
- `blue_pill_minimal`: Blue Pill smoke test with console/status LED
- `stm32g0_minimal`: STM32G0 smoke test
- `blue_pill_modbus_slave`: first Modbus RTU slave example

Temporary reference boards live in [`examples/boards/`](./examples/boards/). Production applications should keep boards in the application repository and set `HSS_BOARD_PATHS`.

Build the starter example from this repository:

```sh
cmake -S examples/hello_blink \
  -B build/examples/hello_blink \
  -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=$PWD/cmake/arm-gcc-toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Debug
cmake --build build/examples/hello_blink
```

## Troubleshooting

**CMake cannot find the board**

Check that `HSS_BOARD_PATHS` points to the directory containing board folders, not to the board folder itself:

```sh
-DHSS_BOARD_PATHS=$PWD/boards
```

**Board sync fails**

Check that the board directory has exactly one `.ioc`, plus `Core/`, `Drivers/`, a startup `.s`, and a linker `.ld`.

**CubeMX added a new peripheral file but it does not compile**

Build-time sync refreshes `board_manifest.cmake`, but CMake may need to reconfigure before it adds new source files to the build graph. Run `cmake -S . -B build/debug`, then build again.

**`hss_status_led_toggle()` returns an error**

Set `BOARD_ROLE_STATUS_LED` in `board_roles.cmake`, and make sure CubeMX configured that pin as GPIO output.

**`printf()` does nothing**

Set `BOARD_ROLE_CONSOLE_UART`, and make sure CubeMX generated the matching UART handle.

**Flash target is missing**

Install OpenOCD and reconfigure CMake. The framework creates flash/debug targets only when it can find `openocd`.

## More Docs

- [`docs/architecture.md`](./docs/architecture.md): framework shape and principles
- [`docs/cubemx_workflow.md`](./docs/cubemx_workflow.md): board sync details
- [`docs/external_project.md`](./docs/external_project.md): app repository usage
- [`docs/vscode.md`](./docs/vscode.md): generated VS Code tasks and launch config
- [`docs/testing.md`](./docs/testing.md): planned testing direction
- [`docs/development_rules.md`](./docs/development_rules.md): contribution rules
