# framework_v3

`framework_v3` is the next iteration of this STM32 framework.

Current version: `0.1.0`

Its goal is to provide a simple, stable, STM32 HAL-focused foundation that sits between:
- STM32CubeMX-generated board code
- reusable framework helpers
- protocol and driver integrations
- the application layer

This version is intentionally not a generic hardware abstraction framework.
It targets STM32 MCUs that use the HAL, accepts some HAL leakage where it keeps things simpler, and focuses on reuse, board portability inside the STM32 family, and a clean developer workflow.

Start with the docs in [`docs/`](./docs/):
- `architecture.md`
- `development_rules.md`
- `cubemx_workflow.md`
- `external_project.md`
- `testing.md`
- `handoff.md`

## Version 0.1.0 Snapshot

This is the first usable scaffold version. It includes:

- CubeMX-owned board projects under `boards/`
- board sync tooling under `tools/board_sync/`
- CMake helpers for external-style firmware projects
- common `hss_result_t` error/result type
- HAL glue for platform init, GPIO, status LED, UART, console stdio, RS485, timers, and Modbus roles
- opt-in FreeModbus integration under `protocols/freemodbus/`
- first HSS Modbus API with app-owned holding/input register banks
- Blue Pill and STM32G0 minimal examples
- Blue Pill Modbus slave example

Known `0.1.0` limits:

- Blue Pill Modbus firmware builds, but real hardware validation is still pending.
- FreeModbus is fetched with CMake `FetchContent`, so first configure needs network access unless cached.
- CubeMX still needs to regenerate USART2 before debug/console can split away from USART1 on the current boards.
- Modbus coils and discrete inputs are intentionally stubbed until an application needs them.
- Host tests and VS Code task generation are planned but not started.

Example firmware projects live in [`examples/`](./examples/).
