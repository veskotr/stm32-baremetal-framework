---
name: hss-add-hal-feature
description: Add or change a reusable STM32 HAL-facing feature in HSS, including public headers, thin HAL implementation, CMake exposure, host fakes, tests, examples, and documentation. Use for work in `hal/` or for a new framework helper around GPIO, EXTI, SPI, UART, timers, watchdogs, IRQs, platform services, or another STM32 peripheral. Do not use for application-only behavior or a standalone device driver.
---

# Add an HSS HAL feature

1. Read `AGENTS.md`, `docs/development_rules.md`, `docs/architecture.md`, and any focused design or accepted decision for the feature.
2. Inspect the relevant public header, implementation, `hal/CMakeLists.txt`, host target in `cmake/hss_testing.cmake`, fakes, and neighboring tests before designing the API.
3. Keep the helper thin. Expose HAL handles or types when that is clearer; avoid a cross-vendor abstraction. Use `hss_result_t` for recoverable or validated operations, not mechanically for every function.
4. Separate three layers when applicable:
   - low-level helper operating on explicit HAL handles or small descriptors;
   - optional board-role convenience API using generated `hss_board_roles.h` macros;
   - application policy, which stays outside the framework.
5. Guard family- or module-specific symbols so host builds and boards without the peripheral still compile. Do not silently claim support for an untested STM32 family.
6. Extend `tests/support/host_stm32_fakes/` with the minimum deterministic state and failure controls needed to test behavior. Reset fake state between tests.
7. Add focused host tests for invalid arguments, HAL status mapping, state transitions, cleanup after errors, availability behavior, and both sides of compile-time role selection when relevant.
8. Update affected usage, architecture, config, or board-role documentation. Add an ADR only if the work changes a durable boundary or establishes a new cross-module pattern.
9. Invoke `$hss-verify-change`. Firmware-facing or shared CMake changes require both reference-family ARM builds unless the toolchain is unavailable.

Do not edit CubeMX-owned `Core/` or `Drivers/` files. Do not treat a successful host test or ARM build as hardware validation.
