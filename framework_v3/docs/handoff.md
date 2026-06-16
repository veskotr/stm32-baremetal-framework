# Handoff Notes

This document captures the current state of `framework_v3` for future coding sessions.

## Current State

`framework_v3` is currently at version `0.1.0`.

`0.1.0` is the first usable scaffold version:

- CubeMX-generated board projects copied into `framework_v3/boards/`
- board sync tooling under `framework_v3/tools/board_sync/`
- generated board metadata and glue for both current boards
- CMake framework entrypoint at `framework_v3/CMakeLists.txt`
- reusable CMake functions in `framework_v3/cmake/hss_framework.cmake`
- ARM GCC toolchain file in `framework_v3/cmake/arm-gcc-toolchain.cmake`
- first common type module under `framework_v3/common/`
- first framework-owned HAL glue module under `framework_v3/hal/`
- first role-backed helper: status LED API generated from `board_roles.cmake`
- placeholder driver module target under `framework_v3/drivers/`
- first opt-in protocol integration under `framework_v3/protocols/freemodbus/`
- example firmware projects in `framework_v3/examples/`
- docs for CubeMX sync, external project usage, examples, and testing direction

The framework is C-first. C++ should not be required by the framework core.

CMake declares the version in `framework_v3/CMakeLists.txt` through `project(... VERSION 0.1.0 ...)` and exposes it internally as `HSS_FRAMEWORK_VERSION`.

The common module currently defines `hss_result_t`, simple result predicates, string conversion, and a HAL status mapping helper in `hal/`. Framework APIs should return `hss_result_t` when the caller can handle a recoverable failure; fire-and-forget calls such as delay and IRQ enable/disable can stay `void`.

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

CMake currently converts selected board roles into a generated `hss_board_roles.h` under the build directory. Supported roles currently include:

```cmake
set(BOARD_ROLE_CONSOLE_UART USART1)
# set(BOARD_ROLE_DEBUG_UART USART2)
set(BOARD_ROLE_MODBUS_UART USART1)
set(BOARD_ROLE_MODBUS_TIMER TIM2)
# set(BOARD_ROLE_MODBUS_RS485_MODE manual)
# set(BOARD_ROLE_MODBUS_RS485_DE PA8)
# set(BOARD_ROLE_MODBUS_RS485_DE_ACTIVE_HIGH ON)
set(BOARD_ROLE_STATUS_LED PC13)
set(BOARD_ROLE_STATUS_LED_ACTIVE_LOW ON)
```

Framework code consumes that role through `hss_status_led_is_available()`, `hss_status_led_on()`, `hss_status_led_off()`, `hss_status_led_write()`, and `hss_status_led_toggle()`.

UART support is async-only by policy. `hss_uart_*` wraps normal HAL UART transmit/receive calls, and `hss_console_*` is the first role-backed UART helper. Synchronous USART mode is intentionally out of scope until a real application needs it.

When `BOARD_ROLE_CONSOLE_UART` is set, framework-owned `__io_putchar()` and `__io_getchar()` route CubeMX/newlib `printf()` and `scanf()` through the console UART. The default stdio timeout is controlled by `HSS_CONSOLE_STDIO_TIMEOUT_MS`.

RS485 is treated as an async UART transport variant. The generic `hss_uart_rs485_write()` helper supports manual DE GPIO control, but DE is optional. If no DE GPIO is provided, the helper falls back to plain UART transmit. Hardware DE should be configured in CubeMX and can use the normal UART write path.

`hss_modbus_uart_*` is a role-backed transport wrapper for FreeModbus. It uses manual RS485 DE when `BOARD_ROLE_MODBUS_RS485_DE` is configured; otherwise it uses plain UART transmit/receive. It also exposes the interrupt-driven serial hooks FreeModbus needs: RX callback registration, TX-empty callback registration, RX interrupt enable/disable, TX-empty interrupt enable/disable, and direct RX/TX data-register byte access.

FreeModbus is now ported into `framework_v3/protocols/freemodbus/` as an opt-in CMake target. Enable it with:

```sh
-DHSS_ENABLE_FREEMODBUS=ON
```

The target fetches upstream FreeModbus `1.6.0` with CMake `FetchContent`, builds the upstream RTU/ASCII/function sources, and provides the STM32/HSS port files under `protocols/freemodbus/src/`.

The HSS-facing Modbus API lives in `protocols/freemodbus/include/hss_modbus.h`. It wraps FreeModbus with `hss_result_t` return values, app-owned holding/input register banks, `hss_modbus_init()`, `hss_modbus_enable()`, `hss_modbus_disable()`, `hss_modbus_poll()`, and single-register get/set helpers. Coil and discrete callbacks currently return `MB_ENOREG`; add those APIs only when an application needs them.

Current FreeModbus focus is Blue Pill. The Blue Pill role setup maps `USART1` as Modbus UART and `TIM2` as Modbus timer. The Blue Pill minimal example enables FreeModbus by default in its local `CMakeLists.txt`. `examples/blue_pill_modbus_slave` is the first API-level Modbus slave example and exposes simple holding/input register arrays. Runtime validation on hardware is still needed.

Current CubeMX projects only enable `USART1`; both generated `board_config.h` files report `BOARD_UART2 0`. The desired split is `USART1` for Modbus and `USART2` for debug/console, but `BOARD_ROLE_DEBUG_UART USART2` should stay commented until CubeMX is regenerated with USART2 enabled.

Timer support is also CubeMX-owned. The generic `hss_timer_*` wrapper exposes start/stop interrupt mode, reset, period ticks, and basic properties. `hss_modbus_timer_*` is the role-backed wrapper for FreeModbus timing. Blue Pill currently maps `BOARD_ROLE_MODBUS_TIMER TIM2`; the G0 board does not yet have a generated timer and should keep the Modbus timer role commented until CubeMX enables one.

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

FreeModbus-enabled Blue Pill build was also configured and built successfully:

```sh
cmake -S framework_v3/examples/blue_pill_minimal \
  -B /tmp/hss-example-blue-freemodbus-on \
  -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=framework_v3/cmake/arm-gcc-toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Debug \
  -DHSS_ENABLE_FREEMODBUS=ON
cmake --build /tmp/hss-example-blue-freemodbus-on
```

This generated `.elf`, `.bin`, `.hex`, `.map`, and size output. The first configure requires network access to fetch the upstream FreeModbus repository unless the dependency is already cached.

The Blue Pill Modbus slave example was also configured and built successfully:

```sh
cmake -S framework_v3/examples/blue_pill_modbus_slave \
  -B /tmp/hss-example-blue-modbus-slave \
  -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=framework_v3/cmake/arm-gcc-toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Debug
cmake --build /tmp/hss-example-blue-modbus-slave
```

This generated `.elf`, `.bin`, `.hex`, `.map`, and size output.

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

1. Validate `examples/blue_pill_modbus_slave` RX/TX and TIM2 timeout behavior on hardware.
2. Decide whether the FreeModbus dependency should stay as `FetchContent` or become a vendored/submodule dependency for offline builds.
3. Add Modbus coil/discrete APIs when an application needs those register types.
4. Extend board role handling to SPI and other timer resources.
5. Add a small host-test scaffold so C framework logic can run on desktop.
6. Add VS Code task generation for configure/build/sync/flash/openocd.
7. Add memory/reporting improvements beyond `arm-none-eabi-size`.

Keep changes small and verify with both examples after changing CMake, sync tooling, or generated board files.
