# framework_v3 Roadmap

## Purpose

This document tracks the current state of `framework_v3`, the decisions already made, and the next implementation steps.

It exists to make handoff between coding sessions easy and to help future contributors understand:
- what has already been decided
- what should be built next
- what constraints should not be broken

## Current Status

`framework_v3` has moved from architecture-only notes into the first working scaffold.

The current implementation can sync CubeMX board metadata, generate board glue, configure external-style firmware targets, and build the current examples.

Current completed work:
- created `framework_v3/`
- added top-level framework description
- documented architecture
- documented development and maintenance rules
- documented that the framework owns the build system
- documented the expected role of CubeMX
- added the first CubeMX sync script for generated board metadata and framework glue
- added the first CMake scaffold for external app consumption
- added initial example firmware projects
- added testing strategy notes
- verified both current examples build on Linux

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

`framework_v3` owns:
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

## Planned Folder Structure

This is the intended high-level structure:

- `framework_v3/boards`
- `framework_v3/cmake`
- `framework_v3/hal`
- `framework_v3/protocols`
- `framework_v3/drivers`
- `framework_v3/tools`
- `framework_v3/docs`
- `framework_v3/examples`

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
- add top-level `framework_v3/CMakeLists.txt`: done
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
- define config file format
- add config generator script
- generate `app_config.h`
- optionally generate CMake config values
- document config usage

Status:
- not started

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
- not started in `framework_v3`
- existing v1 helper code can be used as reference, but should be ported deliberately

### Phase 5: reusable integrations

Goal:
- add reusable protocol and driver support built on the new board model

Priority order:
1. FreeModbus STM32 port
2. Analog Devices no-OS STM32 port

Status:
- not started in `framework_v3`

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
- add a VS Code tool package under `framework_v3/tools`
- add `tasks.json` templates or generated tasks
- add `launch.json` templates or generated launch configs
- document Cortex-Debug usage
- document OpenOCD workflow

Status:
- not started
- `framework_v3/tools/` has been structured so VS Code generators can be added cleanly

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
- host/desktop test implementation is not started

## Current Verification Commands

Run these after changing CMake, board sync generation, or board files:

```sh
python3 framework_v3/tools/sync_board.py framework_v3/boards/blue_pill_temp_transmitter
python3 framework_v3/tools/sync_board.py framework_v3/boards/stm32g071kb_temp_transmitter
```

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

Expected result:
- both examples build
- `.elf`, `.bin`, `.hex`, and `.map` are generated
- firmware size is printed

## Likely First Board

The first real board integration should be based on the current STM32G0 temperature transmitter project already present in the repository.

This is the best first target because:
- it already exists in the repo
- it includes UART usage relevant to Modbus
- it includes peripheral setup relevant to future integrations
- it provides a realistic base for the architecture

## Future CubeMX Setup Guide

A future document should be added for new board/project creation, for example:

- `framework_v3/docs/cubemx_setup.md`

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

The best next task is:

1. scaffold the `framework_v3` implementation folders
2. add the first framework-owned CMake entry points
3. define the board representation for the first STM32G0 board

That would move the project from design-only into a concrete implementation baseline.
