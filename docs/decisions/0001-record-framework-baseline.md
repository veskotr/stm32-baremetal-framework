# 0001: Record the existing framework baseline

- Status: Accepted
- Date: 2026-07-16
- Deciders: project maintainers
- Note: this record documents decisions that predate the ADR log; it does not claim they were first made on this date.

## Context

The framework already has an established direction spread across architecture, development, roadmap, and handoff documents. That direction needs one stable decision baseline so future work and automated contributors can distinguish constraints from tentative roadmap items.

## Decision

The framework will remain:

- STM32-only and based on STM32 HAL; cross-vendor portability is not a goal.
- C-first, with small direct APIs and HAL types visible when that is simpler.
- CubeMX-owned for `.ioc`, clocks, pinmux, peripheral initialization, startup, linker scripts, HAL configuration, and generated interrupt/MSP code.
- framework-owned for CMake, configuration, board synchronization, build, flash, debug, and reporting workflows.
- structured around generated board capabilities and small handwritten role mappings, with generic capabilities preferred over product-specific semantics.
- verified through native host tests plus ARM builds for representative STM32 families, with hardware validation reported as a distinct lane.

Generated CubeMX source must not be patched as part of normal framework integration. Reusable hooks and glue belong in framework-owned or generated integration files outside CubeMX-owned source.

## Consequences

- HAL-specific code is acceptable and should not be hidden behind a cross-vendor abstraction without a concrete need.
- Build features must flow through framework CMake rather than CubeMX-generated build projects.
- New board features should extend metadata, role generation, and reusable helpers instead of adding application-specific conditionals.
- Changes to shared HAL or build behavior need host coverage where practical and representative ARM builds.
- A future change to any of these boundaries requires an explicit superseding decision.

## Alternatives considered

- A vendor-neutral HAL was rejected because it adds abstraction without serving the framework's STM32 scope.
- CubeMX-generated build files as the primary workflow were rejected because they weaken reuse and framework-owned automation.
- Product-specific board roles as the default were rejected because they couple a reusable framework to one application's vocabulary.

## Supersedes

None.
