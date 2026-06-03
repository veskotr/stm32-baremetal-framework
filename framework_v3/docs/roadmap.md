# framework_v3 Roadmap

## Purpose

This document tracks the current state of `framework_v3`, the decisions already made, and the next implementation steps.

It exists to make handoff between coding sessions easy and to help future contributors understand:
- what has already been decided
- what should be built next
- what constraints should not be broken

## Current Status

`framework_v3` is in the documentation and architecture definition stage.

The implementation has not started yet.

Current completed work:
- created `framework_v3/`
- added top-level framework description
- documented architecture
- documented development and maintenance rules
- documented that the framework owns the build system
- documented the expected role of CubeMX

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

## Recommended Implementation Order

The next implementation work should happen in this order unless a good reason appears to change it.

### Phase 1: skeleton and workflow

Goal:
- establish the base framework structure
- make build/flash/debug ownership real

Tasks:
- create initial folder scaffold
- add top-level `framework_v3/CMakeLists.txt`
- add reusable CMake helper functions
- define board registration flow
- define firmware target creation flow
- add OpenOCD flash target support
- add debug target generation

### Phase 2: board model

Goal:
- define how a board is represented inside the framework

Tasks:
- define board metadata format
- define board semantic roles format
- add first STM32G0 board entry based on the current project
- document what is generated vs handwritten

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

### Phase 5: reusable integrations

Goal:
- add reusable protocol and driver support built on the new board model

Priority order:
1. FreeModbus STM32 port
2. Analog Devices no-OS STM32 port

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

### Phase 7: VS Code developer experience

Goal:
- make the default firmware workflow smooth on a fresh setup

Tasks:
- add `tasks.json` templates or generated tasks
- add `launch.json` templates or generated launch configs
- document Cortex-Debug usage
- document OpenOCD workflow

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
