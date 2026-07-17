# Handoff Notes

This document captures the current state of `HSS STM32 framework` for future coding sessions.

## Current State

`HSS STM32 framework` reads its current version from `version.txt`.

The current state is v0.7.0 work:

- external/app-owned CubeMX board projects discovered through `HSS_BOARD_PATHS`
- temporary reference board projects in `examples/boards/`
- board sync tooling under `tools/board_sync/`
- generated board metadata and glue for both current boards
- CMake framework entrypoint at `CMakeLists.txt`
- reusable CMake functions in `cmake/hss_framework.cmake`
- ARM GCC toolchain file in `cmake/arm-gcc-toolchain.cmake`
- VS Code workflow generator under `tools/vscode/`
- first common type module under `common/`
- first framework-owned HAL glue module under `hal/`
- first role-backed helper: status LED API generated from `board_roles.cmake`
- SPI helpers, sensor SPI roles, GPIO EXTI callback dispatch, and IRQ save/restore helpers
- first framework-owned sensor driver under `drivers/max31865/`
- first opt-in protocol integration under `protocols/freemodbus/`
- example firmware projects in `examples/`
- docs for CubeMX sync, external project usage, examples, and testing direction
- patch validation for CubeMX user-code hooks and generated `MX_*_Init()` calls, including the TIM2 Modbus RTU timer fix
- hardware-validated Blue Pill USART1 FreeModbus RTU slave communication with an ESP32 master
- profile-aware HSS config system with generated C/CMake/meta outputs and config-driven FreeModbus/MAX31865 feature selection
- flash-backed EEPROM emulation with config-driven linker reservation and persistence helpers
- FetchContent-first external project documentation
- native host test helpers and framework self-tests enabled by `HSS_ENABLE_TESTS=ON`
- host fake STM32/HAL support for GPIO/SPI tests
- GitHub Actions workflow for Python tests, host C tests, and firmware build checks
- explicit-handle `hss_hal` and role-backed `hss_board` targets, with optional
  driver and protocol targets linked only when a firmware configuration enables them

The framework is C-first. C++ should not be required by the framework core.

CMake reads `version.txt`, passes it to `project(... VERSION ...)`, and exposes it internally as `HSS_FRAMEWORK_VERSION`. The common module also generates `hss_version.h` from the same value.

The common module currently defines `hss_result_t`, simple result predicates, string conversion, and a HAL status mapping helper in `hal/`. Framework APIs should return `hss_result_t` when the caller can handle a recoverable failure; fire-and-forget calls such as delay and IRQ enable/disable can stay `void`. IRQ critical sections that need to preserve the previous interrupt mask should use `hss_irq_save()` and `hss_irq_restore()`.

The config system has a first implementation. Firmware targets can pass `CONFIG hss.conf` to `hss_add_firmware()`, select ordered overlays through `PROFILES` or `HSS_CONFIG_PROFILES`, mark missing overlays optional through `OPTIONAL_PROFILES` or `HSS_CONFIG_OPTIONAL_PROFILES`, add explicit overlay files through `PROFILE_FILES` or `HSS_CONFIG_PROFILE_FILES`, and consume generated `hss_config.h`, `hss_config.cmake`, and `hss_config.meta` files under the build directory. Known `HSS_` keys are validated. Only C-facing custom keys are emitted to `hss_config.h`: `APP_` keys become `CONFIG_APP_*`, and existing `CONFIG_` keys pass through. Other custom keys remain available to CMake/meta only. Unparsable non-empty lines are logged as warnings and ignored. The Blue Pill Modbus slave example uses `hss.conf`, `hss-dev.conf`, `hss-release.conf`, and `hss-hw_rev_b.conf`.

The first host-test scaffold is implemented. `HSS_ENABLE_TESTS=ON` loads `cmake/hss_testing.cmake`, enables CTest, and provides:

- `hss_add_host_test(<name> SOURCES ... [LIBRARIES ...] [INCLUDE_DIRS ...] [DEFINITIONS ...])`
- `hss_add_host_integration_test(<name> SOURCES ... [LIBRARIES ...] [INCLUDE_DIRS ...] [DEFINITIONS ...])`
- `hss_test_unity`: bundled lightweight Unity-style runner under `tests/support/unity/`
- `hss_host_stm32_fakes`: fake `main.h`, GPIO, SPI, and HAL status behavior under `tests/support/host_stm32_fakes/`
- `hss_hal_host`: host-buildable subset of HAL helpers, currently GPIO, SPI, and EXTI

When HSS is the top-level project, `HSS_BUILD_FRAMEWORK_TESTS` defaults to `ON` and adds framework self-tests under `tests/`. When consumed by an app through FetchContent, `HSS_BUILD_FRAMEWORK_TESTS` defaults to `OFF`; apps still get the helper functions and support targets when `HSS_ENABLE_TESTS=ON`.

Current framework self-tests cover:

- `hss_result_t` predicates and string conversion
- GPIO invalid arguments, fake read/write/toggle state, EXTI callback registration/dispatch
- SPI invalid arguments, HAL status mapping, fake transfer state, software chip-select active-low/active-high behavior, deselect-after-error behavior

The test scaffold is intentionally a foundation, not full framework coverage. UART, timers, console, platform, IRQ, FreeModbus, MAX31865, CMake config integration tests, async SPI, and hardware-in-the-loop remain future work.

The next framework blockers from the temp-transmitter pilot are being handled as generic board capabilities, not product-specific roles. Instead of adding a hard-coded `sensor_drdy` role, the framework now supports named generic EXTI roles such as `BOARD_ROLE_EXTI_MAX31865_DRDY`; app firmware/config owns the meaning of that label. SPI support still has the older `sensor_spi` compatibility path, but it now has explicit role-configuration and deselect helpers for the temp-transmitter bring-up. Longer term, SPI should move toward one bus with multiple declared devices/chip-selects rather than one special sensor CS. Existing roles such as `BOARD_ROLE_STATUS_LED` can remain for compatibility, but new feature work should avoid expanding product-specific semantic roles until the generic role model is clearer.

Generic EXTI role support:

```cmake
set(BOARD_ROLE_EXTI_MAX31865_DRDY PA3)
set(BOARD_ROLE_EXTI_MAX31865_DRDY_ACTIVE_LOW ON)
set(BOARD_ROLE_EXTI_MAX31865_DRDY_TRIGGER falling)
```

This generates `HSS_BOARD_EXTI_MAX31865_DRDY_*` macros and `HSS_BOARD_EXTI_MAX31865_DRDY_INPUT`. It also generates a board-specific IRQ shim source under the build directory and adds it to the board object target. Verified mappings:

- STM32F1/Blue Pill PA3 -> `EXTI3_IRQHandler` / `EXTI3_IRQn`
- STM32G0 PA3 -> `EXTI2_3_IRQHandler` / `EXTI2_3_IRQn`

Generic EXTI helper API:

```c
#include "hss_board_roles.h"
#include "hss_exti.h"

hss_exti_input_t drdy = HSS_BOARD_EXTI_MAX31865_DRDY_INPUT;
hss_exti_input_register_callback(&drdy, app_sensor_drdy_callback, NULL);
hss_exti_input_enable(&drdy);
```

Sensor SPI compatibility helpers:

```c
hss_sensor_spi_deselect();
hss_sensor_spi_configure_for_role();
```

Optional role properties currently supported:

```cmake
set(BOARD_ROLE_SENSOR_CS_IDLE inactive)
set(BOARD_ROLE_SENSOR_SPI_MODE 3)
set(BOARD_ROLE_SENSOR_SPI_BAUD_PRESCALER 16)
set(BOARD_ROLE_SENSOR_SPI_NSS software)
set(BOARD_ROLE_SENSOR_SPI_FIRST_BIT msb)
```

## Working Boards

Current boards:

- `blue_pill_temp_transmitter`
- `stm32g071kb_temp_transmitter`

Both boards can be synced through CMake with:

```sh
cmake --build <build-dir> --target hss_sync_boards
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

Generic sensor SPI roles are also supported:

```cmake
set(BOARD_ROLE_SENSOR_SPI SPI1)
# set(BOARD_ROLE_SENSOR_CS PA4)
# set(BOARD_ROLE_SENSOR_CS_ACTIVE_LOW ON)
```

`BOARD_ROLE_SENSOR_SPI` generates `HSS_BOARD_HAS_SENSOR_SPI` and `HSS_BOARD_SENSOR_SPI_HANDLE`. Optional `BOARD_ROLE_SENSOR_CS` generates `HSS_BOARD_HAS_SENSOR_CS`, `HSS_BOARD_SENSOR_CS_PORT`, `HSS_BOARD_SENSOR_CS_PIN`, and `HSS_BOARD_SENSOR_CS_ACTIVE_LOW`. Both current boards map sensor SPI to `SPI1`; neither currently enables a sensor CS role because CubeMX has not configured a dedicated chip-select GPIO. Framework code can consume the role through `hss_sensor_spi_is_available()`, `hss_sensor_spi_get_device()`, `hss_sensor_spi_write()`, `hss_sensor_spi_read()`, and `hss_sensor_spi_transfer()`.

The first v3 sensor driver is `hss_max31865`. It is a native HSS driver shaped after the Analog Devices no-OS MAX31865 implementation used by the v1 framework, but it uses `hss_spi_device_t` and the `BOARD_ROLE_SENSOR_SPI` / optional `BOARD_ROLE_SENSOR_CS` roles instead of requiring the full no-OS platform layer. It supports config init, register read/write, fault clearing/status, bias, auto-convert, 50/60 Hz filtering, 2/4-wire vs 3-wire mode, thresholds, one-shot raw RTD reads, and raw-to-resistance conversion.

UART support is async-only by policy. `hss_uart_*` wraps normal HAL UART transmit/receive calls, and `hss_console_*` is the first role-backed UART helper. Synchronous USART mode is intentionally out of scope until a real application needs it.

When `BOARD_ROLE_CONSOLE_UART` is set, framework-owned `__io_putchar()` and `__io_getchar()` route CubeMX/newlib `printf()` and `scanf()` through the console UART. The default stdio timeout is controlled by `HSS_CONSOLE_STDIO_TIMEOUT_MS`.

RS485 is treated as an async UART transport variant. The generic `hss_uart_rs485_write()` helper supports manual DE GPIO control, but DE is optional. If no DE GPIO is provided, the helper falls back to plain UART transmit. Hardware DE should be configured in CubeMX and can use the normal UART write path.

`hss_modbus_uart_*` is a role-backed transport wrapper for FreeModbus. It uses manual RS485 DE when `BOARD_ROLE_MODBUS_RS485_DE` is configured; otherwise it uses plain UART transmit/receive. It also exposes the interrupt-driven serial hooks FreeModbus needs: RX callback registration, TX-empty callback registration, RX interrupt enable/disable, TX-empty interrupt enable/disable, and direct RX/TX data-register byte access.

FreeModbus is now ported into `protocols/freemodbus/` as an opt-in target. Enable it from target config:

```text
HSS_ENABLE_FREEMODBUS=y
```

or with the compatibility CMake option:

```sh
-DHSS_ENABLE_FREEMODBUS=ON
```

The target fetches upstream FreeModbus `1.6.0` with CMake `FetchContent`, builds the upstream RTU/ASCII/function sources, and provides the STM32/HSS port files under `protocols/freemodbus/src/`.

The HSS-facing Modbus API lives in `protocols/freemodbus/include/hss_modbus.h`. It wraps FreeModbus with `hss_result_t` return values, app-owned holding/input register banks, `hss_modbus_init()`, `hss_modbus_enable()`, `hss_modbus_disable()`, `hss_modbus_poll()`, and single-register get/set helpers. Coil and discrete callbacks currently return `MB_ENOREG`; add those APIs only when an application needs them.

Current FreeModbus focus is Blue Pill. The Blue Pill role setup maps `USART1` as Modbus UART and `TIM2` as Modbus timer. The Blue Pill minimal example enables FreeModbus by default in its local `CMakeLists.txt`. `examples/blue_pill_modbus_slave` is the first API-level Modbus slave example and exposes simple holding/input register arrays. Direct-UART RTU holding-register read/write has been validated on Blue Pill USART1 at 9600 8N1 with an ESP32 master.

Current CubeMX projects only enable `USART1`; both generated `board_config.h` files report `BOARD_UART2 0`. The desired split is `USART1` for Modbus and `USART2` for debug/console, but `BOARD_ROLE_DEBUG_UART USART2` should stay commented until CubeMX is regenerated with USART2 enabled.

Timer support is also CubeMX-owned. The generic `hss_timer_*` wrapper exposes start/stop interrupt mode, reset, period ticks, and basic properties. `hss_modbus_timer_*` is the role-backed wrapper for FreeModbus timing. Blue Pill currently maps `BOARD_ROLE_MODBUS_TIMER TIM2`; the G0 board does not yet have a generated timer and should keep the Modbus timer role commented until CubeMX enables one.

SPI support is a thin HAL helper in `hss_spi_*`. It provides blocking write/read/transfer calls and `hss_spi_device_t` helpers that optionally assert/deassert a software chip-select GPIO around a transaction. CubeMX still owns SPI mode, pinmux, DMA, NSS, and peripheral init.

GPIO support now includes read/write/toggle and a small EXTI callback registry. Applications can call `hss_gpio_register_interrupt(GPIO_PIN_x, callback, context)` and the framework dispatches from `HAL_GPIO_EXTI_Callback()`. CubeMX must still configure the pin as EXTI and generate/enable the correct NVIC handler; registration alone does not enable an interrupt line.

## Verified Builds

The host-test lane was configured, built, and run successfully:

```sh
cmake -S . -B build/host-tests -G Ninja -DHSS_ENABLE_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build/host-tests
ctest --test-dir build/host-tests --output-on-failure
```

CTest result:

- `hss_result_tests`: passed
- `hss_gpio_host_tests`: passed
- `hss_spi_host_tests`: passed

The Python config tests were also run successfully:

```sh
python3 -m unittest discover tools/config/tests
```

Result: 12 tests passed.

Firmware build CI is documented and added in `.github/workflows/ci.yml`. The ARM firmware build lane has now been run locally for the Blue Pill and STM32G0 minimal examples after adding the test scaffold. Hardware validation is still intentionally pending.

The following examples were configured and built successfully on Linux with system `arm-none-eabi-gcc`, Ninja, CMake, and system `openocd` available:

```sh
cmake -S examples/blue_pill_minimal \
  -B /tmp/hss-example-blue \
  -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=$PWD/cmake/arm-gcc-toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Debug
cmake --build /tmp/hss-example-blue
```

```sh
cmake -S examples/stm32g0_minimal \
  -B /tmp/hss-example-g0 \
  -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=$PWD/cmake/arm-gcc-toolchain.cmake \
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
cmake -S examples/blue_pill_minimal \
  -B /tmp/hss-example-blue-freemodbus-on \
  -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=$PWD/cmake/arm-gcc-toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Debug \
  -DHSS_ENABLE_FREEMODBUS=ON
cmake --build /tmp/hss-example-blue-freemodbus-on
```

This generated `.elf`, `.bin`, `.hex`, `.map`, and size output. The first configure requires network access to fetch the upstream FreeModbus repository unless the dependency is already cached.

The Blue Pill Modbus slave example was also configured and built successfully:

```sh
cmake -S examples/blue_pill_modbus_slave \
  -B /tmp/hss-example-blue-modbus-slave \
  -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=$PWD/cmake/arm-gcc-toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Debug
cmake --build /tmp/hss-example-blue-modbus-slave
```

This generated `.elf`, `.bin`, `.hex`, `.map`, and size output.

## Intended User Workflow

For a real application repository, the framework should preferably be added through CMake `FetchContent`. A submodule or copied dependency remains supported. The application owns its board directories.

Example:

```cmake
cmake_minimum_required(VERSION 3.20)

project(my_app C ASM)

include(FetchContent)

FetchContent_Declare(
        hss_framework
        GIT_REPOSITORY https://github.com/veskotr/stm32-baremetal-framework.git
        GIT_TAG v0.6.0
)
FetchContent_MakeAvailable(hss_framework)

set(HSS_BOARD_PATHS "${CMAKE_CURRENT_LIST_DIR}/boards" CACHE STRING "HSS board search paths")

hss_add_firmware(my_app
        BOARD my_board
        src/main.c
)
```

FetchContent firmware projects should keep a tiny app-local ARM GCC toolchain file or preset because CMake needs `CMAKE_TOOLCHAIN_FILE` before FetchContent downloads HSS. The future VS Code extension is expected to make this smoother by owning framework/toolchain setup.

The framework repo may contain examples and framework tests, but production applications should not be added to this repository.

The current CMake scaffold supports one selected board per build directory. Use separate build directories for different boards.

If `board_manifest.cmake` is missing for a selected board, `HSS_BOARD_AUTO_SYNC=ON` lets CMake run `tools/sync_board.py` during configure. The stable `hss_sync_boards` target and the generated `sync_board_<board>` target remain available for explicit refresh after CubeMX regeneration.

Each `hss_add_firmware(<target> ...)` call now creates:

- `<target>`
- `flash_<target>`
- `openocd_<target>`
- `vscode_<target>`

`hss_generate_vscode(<target>)` writes HSS-owned VS Code entries during CMake configure. `hss_add_firmware(... GENERATE_VSCODE ...)` is a shorthand for the same behavior. `vscode_<target>` remains available when generation should be triggered manually as a build target. The generator preserves unrelated entries and replaces entries generated for the same HSS target. The launch config expects the VS Code Cortex-Debug extension.

## Linux OpenOCD Policy

On Linux, use the system `openocd` found in `PATH`.

The bundled Windows OpenOCD tools from older framework versions are not used by `HSS STM32 framework` on Linux. Windows support can later add a platform-specific path or generated VS Code settings.

## Next Best Steps

Good next implementation tasks:

1. Validate the current firmware examples on hardware after the test-scaffold changes.
2. Stabilize the test scaffold as the baseline for new feature work in version control.
3. Migrate temp-transmitter to `BOARD_ROLE_EXTI_MAX31865_DRDY`, `hss_exti_input_*`, `hss_sensor_spi_deselect()`, and `hss_sensor_spi_configure_for_role()`.
4. Implement generic SPI bus/device metadata for multiple chip-selects and inactive CS handling.
5. Add board-sync validation/checklist diagnostics for EXTI mode, IRQ enablement, SPI mode, and CS idle level.
6. Add targeted tests for each generic capability as it lands, then return to broader framework test completion.
7. Validate `examples/blue_pill_modbus_slave` RX/TX and TIM2 timeout behavior on hardware.
8. Add host coverage for UART, timers, console, IRQ/platform helpers, MAX31865, and selected FreeModbus adapter behavior.
9. Start async SPI planning/implementation using the new fake SPI layer as the first test-driven feature.
10. Add Modbus coil/discrete APIs when an application needs those register types.
11. Validate the generated VS Code debug launch on hardware with Cortex-Debug and OpenOCD.
12. Add memory/reporting improvements beyond `arm-none-eabi-size`.

Keep changes small and verify with both examples after changing CMake, sync tooling, or generated board files.
