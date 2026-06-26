# Testing Strategy

Testing is a first-class goal for the framework.

The framework should eventually support:

- framework unit tests on the desktop
- firmware logic tests on the desktop
- user application unit tests
- user application integration tests
- hardware-in-the-loop smoke tests
- later factory automated tests

## Current Shape

The framework has two normal test lanes:

- native host tests for fast C logic and fake peripheral behavior
- ARM firmware build checks for real board/example integration

Hardware-in-the-loop tests are a later optional lane.

Host tests are enabled with:

```sh
cmake -S . -B build/host-tests -G Ninja -DHSS_ENABLE_TESTS=ON
cmake --build build/host-tests
ctest --test-dir build/host-tests --output-on-failure
```

The framework exposes:

```cmake
hss_add_host_test(my_unit_tests
        SOURCES tests/test_state_machine.c src/state_machine.c
)

hss_add_host_integration_test(my_integration_tests
        SOURCES tests/test_spi_flow.c src/sensor_logic.c
        LIBRARIES hss_hal_host
)
```

`hss_add_host_test()` registers a CTest test labeled `unit`.
`hss_add_host_integration_test()` registers a CTest test labeled `integration`.

Host tests can link:

- `hss_test_unity`: bundled lightweight Unity-style test runner
- `hss_host_stm32_fakes`: host fake STM32/HAL types and fake GPIO/SPI hooks
- `hss_hal_host`: host-buildable GPIO/SPI/EXTI HAL helper subset

## Direction

The firmware should be structured so useful logic can run without an STM32 target.

The intended split is:

- hardware-facing code remains HAL/CubeMX based
- reusable logic is kept behind small interfaces where practical
- app logic can be compiled for a host test runner
- target builds and host test builds share as much source as possible

## Desktop Firmware Runs

The framework should gain a host build mode that can run firmware-like code on a desktop.

This does not mean simulating every STM32 peripheral. The first useful version should provide:

- host stubs for time, GPIO, UART, SPI, and timers
- deterministic fake peripherals for tests
- a way to run app main-loop steps without flashing hardware
- a way to inspect fake peripheral state from tests

## User Application Tests

Application repositories should have an easy way to add tests without modifying the framework repo.

FetchContent is the preferred dependency path:

```cmake
include(FetchContent)

FetchContent_Declare(
        hss_framework
        GIT_REPOSITORY https://github.com/veskotr/stm32-baremetal-framework.git
        GIT_TAG v0.5.0
)
FetchContent_MakeAvailable(hss_framework)
```

Test shape:

```cmake
hss_add_firmware(my_app BOARD my_board src/main.c)
hss_add_host_test(my_app_unit_tests SOURCES tests/test_state_machine.c src/state_machine.c)
hss_add_host_integration_test(my_app_integration SOURCES tests/test_modbus_flow.c)
```

User CI should normally use two build directories:

- `build/host-tests`: native compiler, `-DHSS_ENABLE_TESTS=ON`, then CTest
- `build/firmware`: ARM compiler/toolchain, firmware targets only

Until a future VS Code extension owns toolchain setup, FetchContent firmware projects should keep a tiny app-local ARM toolchain file or CMake preset. HSS still owns board flags, linker script, board metadata, and post-build artifacts.

## Release Workflow

The repository has a manual GitHub Actions release workflow:

- open GitHub Actions
- select `Release`
- click `Run workflow`

The workflow reads `version.txt`, creates an annotated tag named `v<version>`, pushes the tag, and creates a GitHub Release for that tag. The release notes contain the latest commit message from the selected branch/ref. The workflow fails if the tag already exists, so bump `version.txt` before running it for a new release.

## Factory Tests

Factory automated tests are a later phase.

Likely needs:

- scripted flash
- scripted reset
- serial log capture
- pass/fail command protocol
- fixture configuration
- per-board factory test metadata
- CI-friendly result output

Factory tests should build on the same flash/debug/reporting tools used during development rather than becoming a separate workflow.
