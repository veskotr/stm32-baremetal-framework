---
name: hss-change-board-support
description: Add, regenerate, or modify HSS CubeMX-backed board integration, board capabilities, semantic roles, generated role headers/glue, or board-sync tooling. Use for `.ioc`-derived metadata, `board_roles.cmake`, `tools/board_sync/`, board registration, EXTI/peripheral mapping, or adding a reference board. Do not use for application policy that merely consumes an existing role.
---

# Change HSS board support

1. Read `AGENTS.md`, `docs/cubemx_workflow.md`, `docs/architecture.md`, and accepted board-related decisions.
2. Classify every touched file before editing:
   - CubeMX-owned input/output: `.ioc`, `Core/`, `Drivers/`, startup, linker scripts;
   - generated HSS output: `board_manifest.cmake`, `generated/board_config.h`, `generated/board_glue.c`;
   - handwritten HSS input: `board_roles.cmake`, generators, CMake, and framework helpers.
3. Never patch CubeMX-owned C/H files to integrate HSS. Put reusable glue in framework-owned sources or generated HSS output.
4. Change generator logic or its source inputs instead of hand-editing generated outputs. Preserve content-stable, idempotent generation and clear ownership comments.
5. Prefer generic capability names. Keep product meaning in application config; add a semantic convenience only when it is broadly reusable and compatible with the accepted role model.
6. When parsing peripherals or pins, test both supported STM32 families and failure cases such as missing, malformed, disabled, or conflicting resources. Preserve CubeMX spellings such as `SPI1`, `USART1`, `TIM2`, and `PA4` at input boundaries.
7. Run board sync for affected reference boards and review generated diffs. If the source list changes, reconfigure CMake before building.
8. Update `docs/cubemx_workflow.md`, role/config documentation, and examples when inputs or generated outputs change. Record a decision for a new durable board-model convention.
9. Invoke `$hss-verify-change`; generator changes require Python tests plus both reference-family firmware builds.

Report which files are source inputs and which were regenerated. Never describe an ARM build as validation on physical hardware.
