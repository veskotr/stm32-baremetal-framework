# HSS STM32 framework Roadmap

## Purpose

This document tracks the current state of `HSS STM32 framework`, the decisions already made, and the next implementation steps.

It exists to make handoff between coding sessions easy and to help future contributors understand:
- what has already been decided
- what should be built next
- what constraints should not be broken

## Current Status

`HSS STM32 framework` has moved from architecture-only notes into the first working scaffold.

Current version: see `version.txt`.

The current implementation can sync CubeMX board metadata, generate and validate board glue, configure external-style firmware targets, build the current examples, provide SPI/GPIO helper coverage, generate VS Code task/debug workflows, run the Blue Pill Modbus RTU slave against an ESP32 master, configure firmware targets through profile-aware HSS config files, and run the first host C framework tests through CTest.

Current completed work:
- promoted the current framework implementation to the repository root
- added top-level framework description
- documented architecture
- documented development and maintenance rules
- documented that the framework owns the build system
- documented the expected role of CubeMX
- added the first CubeMX sync script for generated board metadata and framework glue
- added the first CMake scaffold for external app consumption
- added the first common result/error type module
- added the first framework-owned `hal/` C glue module
- added placeholder `drivers` and `protocols` module targets
- added initial example firmware projects
- added first opt-in FreeModbus protocol port
- added first HSS Modbus API and Blue Pill Modbus slave example
- added SPI helpers, sensor SPI roles, and GPIO EXTI callback dispatch
- added VS Code tasks and Cortex-Debug launch generation
- added testing strategy notes
- verified both current examples build on Linux
- fixed CubeMX generated-glue discovery so missing `MX_*_Init()` calls such as `MX_TIM2_Init()` are generated and validated
- validated Blue Pill USART1 FreeModbus RTU holding-register read/write on hardware
- moved temporary Modbus debug counters behind `HSS_ENABLE_MODBUS_DEBUG`
- added the v0.4.0 project config system with base config files, profile overlays, optional profiles, explicit profile files, generated C/CMake/meta outputs, C-facing custom macros, schema help, parser tests, and config-driven FreeModbus/MAX31865 feature selection
- added the v0.6.0 test/CI, generic board-capability, and flash-backed persistence work: `HSS_ENABLE_TESTS`, `hss_add_host_test`, `hss_add_host_integration_test`, a bundled lightweight Unity-style runner, host STM32 GPIO/SPI/EXTI fakes, `hss_hal_host`, initial result/GPIO/SPI/EXTI tests, generic EXTI roles, sensor SPI compatibility helpers, EEPROM emulation, linker reservation generation, and the persistence demo example
- added the v0.6.0 flash-backed persistence work: `HSS_ENABLE_EEPROM_EMULATION`, EEPROM geometry config, linker reservation generation, STM32F1/G0 backend support, config validation, and the new persistence demo example
- added FetchContent-first external-project documentation and GitHub Actions CI lanes for Python tests, host C tests, and firmware build checks

`0.1.0` should be treated as the first scaffold baseline. `0.2.0` is the HAL-helper and developer-workflow milestone. `0.2.1` is the Modbus/CubeMX generated-glue bugfix patch after first hardware validation. `0.3.0` is the repository cleanup release that promotes the framework to the root and moves the temp transmitter pilot into its own project. `0.4.0` is the project config system release. `0.5.0` is the test/CI scaffold and generic board-capability release. `0.6.0` is the EEPROM emulation and flash-backed persistence release. `0.7.0` is the breaking target-boundary release: explicit `hss_hal`, board conveniences in `hss_board`, and opt-in drivers and protocols.

## Confirmed Design Decisions

The following decisions are already agreed and should be treated as the working baseline unless there is a strong reason to change them.

### 1. Scope

- The framework targets STM32 MCUs only.
- The framework is HAL-based.
- Portability across STM32 families is a goal.
- Cross-vendor portability is not a goal.

### 2. HAL usage

- HAL leakage is acceptable.
- The framework should stay simple rather than hiding HAL behind heavy abstraction.
- HAL handles and STM32 HAL types may be used in framework code when helpful.
- The framework core should stay C-first. C++ may be used by external applications later, but it is not required by the framework.

### 3. CubeMX ownership

CubeMX owns:
- board `.ioc`
- generated `Core/`
- generated `Drivers/`
- startup files
- linker scripts
- peripheral init code
- HAL configuration

The framework should not fight CubeMX.

### 4. Build ownership

`HSS STM32 framework` owns:
- CMake files
- build flow
- flash flow
- debug flow
- post-build reports
- config generation
- board sync and generation tooling

CubeMX-generated build files are not the main workflow and should be ignored by default.

### 5. Developer workflow

The desired workflow is:
- configure
- build
- flash
- debug
- inspect memory and firmware stats

### 6. Automation

The framework should automate repeated setup work where possible.

Important future automation areas:
- board capability extraction from CubeMX output
- semantic role mapping
- generated board metadata
- framework patching/sync steps for generated projects
- project config header generation
- post-build reporting

### 7. Documentation policy

- Docs are part of the framework.
- If a framework feature changes usage or architecture, docs must be updated in the same change.

### 8. Generic roles vs application meaning

- The framework should prefer generic board capabilities over product-specific names.
- Firmware/application config should assign product meaning to those capabilities.
- Avoid adding narrowly named framework roles such as `sensor_drdy` when a generic external interrupt input can serve the same need.
- SPI support should model one bus with multiple logical devices/chip-selects, not a single baked-in sensor CS.
- Existing semantic conveniences such as `BOARD_ROLE_STATUS_LED` can remain for compatibility, but new work should not expand that pattern blindly. Product-specific aliases should move toward app config, generated metadata, or thin app-side wrappers.

## Planned Folder Structure

This is the intended high-level structure:

- app-owned board directories discovered through `HSS_BOARD_PATHS`
- temporary reference boards under `examples/boards`
- `cmake`
- `common`
- `hal`
- `protocols`
- `drivers`
- `tools`
- `docs`
- `examples`

## Recommended Implementation Order

The next implementation work should happen in this order unless a good reason appears to change it.

### Phase 1: skeleton and workflow

Goal:
- establish the base framework structure
- make CubeMX sync and build/flash/debug ownership real

Tasks:
- create initial folder scaffold: done
- add CubeMX board sync script: done
- generate initial board metadata from the current CubeMX projects: done
- add top-level `CMakeLists.txt`: done
- add reusable CMake helper functions: done
- define board registration flow: done
- define firmware target creation flow: done
- add OpenOCD flash target support: done for Linux/system OpenOCD
- add debug target generation: initial `openocd_<target>` target exists

### Phase 2: board model

Goal:
- define how a board is represented inside the framework

Tasks:
- define board metadata format: first generated `board_manifest.cmake` exists
- define board semantic roles format: first editable `board_roles.cmake` exists
- generate board role header from selected CMake roles: started with status LED
- add first STM32G0 board entry based on the current project: done
- document what is generated vs handwritten: started in `cubemx_workflow.md`

Possible files:
- board manifest
- board roles file
- generated capability headers
- generated board resource headers

### Phase 3: config system

Goal:
- provide a lightweight project config flow similar in spirit to ESP-IDF

Tasks:
- define config file format and profile overlay model: done in `docs/config_system.md`
- add config generator script: done with `tools/generate_config.py` and `tools/config/`
- generate `hss_config.h`: done
- generate CMake config values: done with `hss_config.cmake`
- document config usage: started in `docs/config_system.md` and `docs/external_project.md`

Status:
- first implementation exists with base config, ordered profiles, optional profiles, explicit profile files, custom `CONFIG_` macros, generated metadata, CMake integration, FreeModbus/MAX31865 feature selection, example configs, and parser/generator tests

### Phase 3.5: common types

Goal:
- provide small framework-wide types that apps and framework modules can share

Status:
- started with `hss_result_t` and simple helpers
- current convention: use `hss_result_t` for recoverable or validateable operations, not mechanically for every function
- keep this module small and dependency-light

### Phase 4: HAL helper layer

Goal:
- add only the thin reusable helpers that remove real duplication

Expected focus:
- UART helpers
- SPI helpers
- GPIO helpers
- timer helpers
- delay/time helpers
- critical section helpers

Status:
- started with platform init, delay, IRQ, IRQ save/restore, and basic GPIO wrappers
- first semantic helper added: status LED API backed by `BOARD_ROLE_STATUS_LED`
- async UART wrapper added for blocking TX/RX and byte writes
- console UART role added with `hss_console_*`
- `printf()` and `scanf()` are mapped through `BOARD_ROLE_CONSOLE_UART` via CubeMX `__io_putchar()`/`__io_getchar()`
- manual-DE RS485 transmit helper added; hardware DE remains CubeMX-owned
- RS485 DE is optional; missing DE falls back to plain UART transmit
- Modbus UART role wrapper added as FreeModbus transport preparation
- Modbus UART interrupt/data-register helpers added for the FreeModbus serial port
- timer wrapper added for start/stop interrupt mode, reset, period ticks, and basic properties
- Modbus timer role wrapper added; Blue Pill maps TIM2, G0 still needs a CubeMX timer
- SPI wrapper added for blocking write/read/transfer operations, plus optional software chip-select handling through `hss_spi_device_t`
- GPIO wrapper now supports read/write/toggle and callback registration for CubeMX/HAL EXTI callbacks
- IRQ helper now supports save/restore of the interrupt mask for critical sections; the FreeModbus port uses this with local nesting
- first generic sensor SPI role macros are generated from `BOARD_ROLE_SENSOR_SPI` and optional `BOARD_ROLE_SENSOR_CS`; `hss_sensor_spi_*` provides the role-backed helper
- planned next SPI helper: async SPI transfers driven by DMA and/or interrupts, with completion/error callbacks and CubeMX-owned DMA/NVIC setup
- synchronous USART mode is intentionally out of scope
- existing v1 helper code can be used as reference, but should be ported deliberately

### Phase 5: reusable integrations

Goal:
- add reusable protocol and driver support built on the new board model

Priority order:
1. FreeModbus STM32 port
2. MAX31865 sensor driver
3. Analog Devices no-OS STM32 port

Status:
- FreeModbus has a first v3 port under `protocols/freemodbus/`
- FreeModbus is opt-in with `HSS_ENABLE_FREEMODBUS=ON` or target config `HSS_ENABLE_FREEMODBUS=y`
- upstream FreeModbus `1.6.0` is fetched with CMake `FetchContent`
- port files are backed by `hss_modbus_uart_*`, `hss_modbus_timer_*`, and `hss_irq_*`
- HSS-facing Modbus API added with `hss_modbus_*`, app-owned holding/input register banks, and `hss_result_t` returns
- Blue Pill FreeModbus-enabled firmware configures and builds
- Blue Pill minimal example enables FreeModbus by default
- Blue Pill Modbus slave example added and builds
- Blue Pill USART1 RTU holding-register read/write has been validated on hardware with an ESP32 master
- MAX31865 has a first v3 driver under `drivers/max31865/`, based on the ADI no-OS driver behavior but using HSS sensor SPI roles directly
- broader Analog Devices no-OS platform integration is not started

### Phase 6: automation and reporting

Goal:
- reduce manual board bring-up work
- improve visibility after build

Tasks:
- board sync/generation scripts
- CubeMX patch/sync helpers
- memory usage report
- artifact summary
- optional object/symbol size reporting

Status:
- basic `.elf`, `.bin`, `.hex`, `.map`, and size output exist
- deeper reporting is not started

### Phase 7: VS Code developer experience

Goal:
- make the default firmware workflow smooth on a fresh setup

Tasks:
- add a VS Code tool package under `tools`
- add `tasks.json` templates or generated tasks
- add `launch.json` templates or generated launch configs
- document Cortex-Debug usage
- document OpenOCD workflow

Status:
- started with `tools/vscode/`
- `hss_generate_vscode(<target>)`, `hss_add_firmware(... GENERATE_VSCODE ...)`, and `vscode_<firmware_target>` generate `.vscode/tasks.json` and `.vscode/launch.json`
- generated launch configs use Cortex-Debug and board OpenOCD metadata
- `tools/` has been structured so VS Code generators can be added cleanly

### Phase 8: examples and testing

Goal:
- make the framework easy to learn and easy to test

Tasks:
- add examples for common board and peripheral workflows
- add framework unit tests that run on the desktop
- add a host build mode for firmware-like logic
- add app-level unit test helpers
- add app-level integration test helpers
- define hardware-in-the-loop smoke test conventions
- define later factory automated test conventions

Status:
- initial examples exist and build
- testing strategy is documented in `testing.md`
- first host/desktop test implementation exists and passes locally
- `HSS_ENABLE_TESTS=ON` exposes CTest helpers and host support targets
- framework self-tests currently cover `hss_result_t`, GPIO fakes/EXTI dispatch, and SPI fakes/status/device CS behavior
- GitHub Actions workflow exists for Python tests, host C tests, and ARM firmware example build checks
- local ARM firmware build checks pass for `examples/blue_pill_minimal` and `examples/stm32g0_minimal`
- hardware validation is pending after the host-test scaffold change

### Phase 8.5: test stabilization and generic board capability work

Goal:
- use the new test scaffold as guardrails while unblocking real firmware needs
- avoid freezing product-specific roles into the framework API

Ordering:
1. stabilize and commit the current host-test/CI scaffold
2. add narrowly targeted tests beside each new board-capability feature
3. implement the missing generic features needed by the temp transmitter pilot
4. return to broader framework test completion after those blockers are removed

Planned capability improvements:
- generic external interrupt input roles were started with `BOARD_ROLE_EXTI_<name>`
- role metadata is generated for EXTI pin, GPIO port, active level, trigger edge, IRQ handler, IRQn, priority, and subpriority
- framework-owned IRQ shim sources are generated for selected EXTI roles so applications do not define `EXTI*_IRQHandler()` themselves
- generic helper APIs for external interrupt roles are available through `hss_exti_input_*`
- evolve sensor SPI into generic SPI bus/device metadata that supports multiple chip-selects on one bus
- expose chip-select idle/inactive state and provide a helper to put declared SPI devices into their inactive state before use; compatibility support exists for `BOARD_ROLE_SENSOR_CS_IDLE` and `hss_sensor_spi_deselect()`
- add explicit SPI role/device configuration helpers for mode, prescaler, NSS, first-bit, and related settings when CubeMX output needs verification or correction; compatibility support exists through `hss_sensor_spi_configure_for_role()`
- add board-sync validation/checklist diagnostics after the metadata exists, rather than making validation the first implementation step

Testing expectations for this phase:
- unit-test EXTI IRQ mapping for STM32F1/F4-style and STM32G0-style IRQ groups
- host-test generic EXTI helper behavior against the GPIO interrupt registry and fake NVIC hooks
- CMake/config tests for generated role metadata where practical
- preserve existing builds when no external interrupt or extra chip-select roles are declared

Non-goals for this phase:
- finishing all framework test coverage
- adding hardware-in-the-loop automation
- making the framework own product-specific names such as `sensor_drdy`
- expanding status LED-style semantic roles before the generic role model is clearer

### Phase 9: dependency and component readiness

Goal:
- keep HSS small while making reusable application components practical in a
  separate repository.

The accepted boundary is recorded in
[`0003-separate-framework-components-and-products.md`](decisions/0003-separate-framework-components-and-products.md):
HSS provides generic STM32 capabilities and tooling; a component repository
provides reusable behavior; products own boards, configuration, and composition.

Technical-debt priority:

1. Define and document stable granular CMake dependencies for `hss_common`,
   `hss_hal`, `hss_board`, each driver, and each protocol. Done: aggregate
   framework, driver, and protocol targets were removed.
2. Make optional drivers and protocols opt-in by default unless a dependency is
   fundamental to every HSS application. Done for MAX31865 and FreeModbus.
3. Create a small external component-repository proof project that consumes HSS
   through `FetchContent`, links only narrow targets, and runs host tests.
4. Prefer component APIs that accept explicit handles, descriptors, or callbacks
   over board-role globals. Products adapt board roles to those APIs.
5. Add dependency-focused CMake coverage so an optional integration cannot be
   pulled into a component unexpectedly.

Non-goals:
- moving application components into HSS;
- replacing STM32 HAL types with a cross-vendor abstraction;
- making the framework own product policy or application `main()` functions.

## Current Verification Commands

Run the host tests after changing framework C helpers, host fakes, or test CMake:

```sh
cmake -S . -B build/host-tests -G Ninja -DHSS_ENABLE_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build/host-tests
ctest --test-dir build/host-tests --output-on-failure
```

Run the Python config tests after changing `tools/config/`:

```sh
python3 -m unittest discover tools/config/tests
```

Run these after changing CMake, board sync generation, or board files:

```sh
cmake --build <build-dir> --target hss_sync_boards
```

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

Expected result:
- both examples build
- `.elf`, `.bin`, `.hex`, and `.map` are generated
- firmware size is printed

## Pilot Application

The temp transmitter pilot now lives outside this repository, under `/home/vesko/projects/temp-transmitter` in the local workspace. It owns its board folders and consumes this framework as a sibling dependency.

## Future CubeMX Setup Guide

A future document should be added for new board/project creation, for example:

- `docs/cubemx_setup.md`

That document should eventually include:
- required CubeMX project settings
- generation flags
- naming conventions
- expected output structure
- regeneration workflow
- required post-generation sync command

## Open Questions

These items are not fully decided yet and should be revisited during implementation.

### 1. Board metadata source split

Need to finalize how much is inferred automatically from CubeMX output versus how much is declared explicitly in a board roles file.

Current preferred direction:
- capabilities inferred where practical
- semantic roles declared explicitly

### 2. Generated board files

Need to finalize the exact generated file set for board capabilities and resource mappings.

### 3. Config format

Need to choose the simplest stable config input format.

Current likely options:
- simple `KEY=VALUE`
- YAML
- JSON

Preferred direction right now:
- simple `KEY=VALUE` first

### 4. Reporting depth

Need to decide how much post-build reporting is included in the first version.

Minimum expected:
- flash usage
- RAM usage
- section sizes
- artifact paths

## Constraints To Preserve

Future work should preserve these constraints:

- keep the framework simple
- keep it easy to understand
- do not over-abstract HAL
- do not move build ownership back to CubeMX
- do not require long manual setup guides for common tasks
- prefer automation over repeated manual edits
- keep docs in sync with implementation

## Suggested Next Task

The best next task is to continue validating the temp transmitter pilot against real hardware, then promote only the reusable pieces back into the framework.
