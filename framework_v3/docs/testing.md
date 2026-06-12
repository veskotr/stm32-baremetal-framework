# Testing Strategy

Testing is a first-class goal for the framework.

The framework should eventually support:

- framework unit tests on the desktop
- firmware logic tests on the desktop
- user application unit tests
- user application integration tests
- hardware-in-the-loop smoke tests
- later factory automated tests

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

Desired future shape:

```cmake
add_subdirectory(third_party/framework_v3)

hss_add_firmware(my_app BOARD my_board src/main.c)
hss_add_host_test(my_app_unit_tests tests/test_state_machine.c src/state_machine.c)
hss_add_host_integration_test(my_app_integration tests/test_modbus_flow.c)
```

The exact APIs are not final, but the goal is clear: users should be able to test their own applications with small CMake calls.

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
