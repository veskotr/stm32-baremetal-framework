# HSS STM32 framework Development Rules

## Purpose

This document defines the rules for adding code, integrations, tooling, and boards to `HSS STM32 framework`.

The goal is to keep the framework:
- simple
- stable
- easy to extend
- easy to understand
- easy to reuse across future STM32 projects

## General Rules

1. Prefer simple over clever.

If two designs both work, use the one that a future developer will understand faster.

2. Keep STM32 HAL visible when it helps.

Do not create unnecessary wrappers just to hide HAL names or types.

3. Reuse must come from structure and tooling.

Do not depend on developers remembering long manual setup sequences.

4. Generated code and framework code must have clear ownership.

CubeMX-generated files are generated code. Framework scripts may synchronize required additions, but handwritten framework logic should live outside generated code whenever possible.

5. New features must improve the common workflow.

Do not add framework features that make rare cases elegant but make normal use harder.

6. The framework owns build behavior.

Do not let CubeMX-generated build files become the main project workflow. Build logic should stay in framework-owned CMake files.

## Rules For Writing Framework Code

- Keep APIs small and direct.
- Prefer named resources and semantic roles over generic numeric IDs when possible.
- Avoid deep inheritance, complicated templates, or overly generic utility layers.
- Keep board-specific logic in board-related files.
- Keep reusable logic in shared framework folders.
- Prefer explicit data structures over hidden global behavior.
- Make error conditions visible and easy to diagnose.

## Rules For Board Support

- Each board should define clear semantic roles for the peripherals the framework cares about.
- Board metadata should be generated or derived where practical.
- Board-specific handwritten files should stay small.
- New board support should not require copy-pasting large amounts of code.
- Board support should be buildable through framework-owned CMake, not through CubeMX-generated build files.

Examples of roles:
- Modbus UART
- Modbus timer
- sensor SPI
- sensor chip select
- status LED

When a role names a peripheral or pin, use the same names CubeMX exposes (`SPI1`, `USART1`, `TIM2`, `PA4`, `PC13`) so generated role headers can remain simple and predictable.

## Rules For Integrations

When adding a new protocol or driver integration:

- separate reusable integration logic from app logic
- document what board metadata it needs
- document what generated or configured resources it expects
- avoid forcing unrelated projects to include heavy dependencies

Each integration should clearly answer:
- what problem it solves
- what HAL resources it needs
- what board roles it expects
- what public API the app uses

## Rules For Tooling

Tooling should aim to remove repeated manual work.

Good tooling candidates:
- board capability generation
- config header generation
- post-build reporting
- VS Code task generation
- OpenOCD helper generation
- CubeMX sync helpers

Tooling should be:
- idempotent
- easy to rerun
- clear when it fails
- explicit about what files it owns

Tooling may later include:
- CubeMX project generation guidance
- board sync commands
- board capability extraction
- framework patching of generated user sections

## Rules For Generated Files

Generated files should:
- have a clear ownership comment
- be reproducible from source inputs
- avoid manual editing when possible
- live in predictable locations

If a file is generated, the docs should say:
- what generates it
- what inputs it uses
- whether users are allowed to edit it manually

## Rules For Build / Flash / Debug

`HSS STM32 framework` must always support an easy firmware workflow.

At minimum, each supported project should have a clear path for:
- build
- flash
- debug

The framework should also aim to provide:
- artifact generation
- memory usage reporting
- OpenOCD integration
- VS Code task and launch support

Build system ownership rules:
- framework CMake is authoritative
- CubeMX-generated build files are optional and ignored by default
- board CMake files must be maintained as framework code
- changes to board layout that affect build inputs should be reflected in framework-owned CMake or generation tooling

## Rules For Config

Configuration should be centralized and easy to inspect.

The config system should:
- stay lightweight
- be easy to version control
- generate headers or build values from a simple input format
- avoid hidden magic

Configuration should be used for things like:
- protocol settings
- feature flags
- timing constants
- product options

## Documentation Rules

Documentation is part of the framework, not an afterthought.

When adding or changing a feature, update the relevant docs in the same change if it affects:
- architecture
- usage flow
- project layout
- config flow
- board setup
- generated files
- developer expectations

### Required documentation updates

If a new feature is added, update whichever of these are affected:
- `README.md`
- `docs/architecture.md`
- `docs/development_rules.md`
- future feature-specific docs

If the expected CubeMX project generation settings change, the future CubeMX setup guide must also be updated in the same change.

### Rule of thumb

If a future coding session would benefit from knowing why a piece exists, how it is meant to be used, or what constraints it has, document it.

## Future Expansion Rule

This framework is expected to grow across multiple projects.

New code should be added in a way that makes later reuse easier:
- prefer conventions over one-off hacks
- prefer role-based board descriptions over scattered defines
- prefer generated metadata over manual duplicated declarations
- prefer framework-owned tooling over setup checklists

The framework should accumulate reusable structure, not reusable confusion.
