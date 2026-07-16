# 0003: Separate framework, components, and products

- Status: Accepted
- Date: 2026-07-16
- Deciders: project maintainers

## Context

HSS is intended to remain a small STM32 HAL-first framework while future
projects will need reusable application behavior such as telemetry, device
health, persistence policy, and product workflows. Keeping that behavior in HSS
would turn board capabilities and generic helpers into a product-specific
collection. Conversely, repeatedly copying it into products would lose reuse.

## Decision

Use three explicit layers:

1. HSS provides STM32 HAL helpers, generic board capabilities, integrations,
   build tooling, and host-test support.
2. A separate reusable component repository provides application behavior. Its
   components declare narrow dependencies on specific HSS targets and expose
   small C APIs with caller-supplied configuration, handles, or callbacks.
3. Product applications own CubeMX boards, product configuration, component
   composition, and `main()`.

Components must not own CubeMX configuration, generated board files, product
board-role names, or framework-wide policy. They may consume a generic HSS
capability when the product explicitly supplies it.

`hss_framework` remains a convenience umbrella target for simple applications.
Reusable components should link the narrowest public HSS targets they require
rather than depending on the umbrella target.

## Consequences

- HSS can stay small even as reusable product behavior grows.
- Components can be consumed through CMake `FetchContent`, a submodule, or a
  copied source dependency without making HSS depend on them.
- Component APIs need host-testable seams rather than hidden board globals.
- HSS needs clear, stable granular CMake targets and explicit optional
  integrations before component adoption becomes routine.

## Alternatives considered

- Putting components under this repository was rejected because application
  policy would expand the framework's scope and release surface.
- Putting all reusable behavior directly in each product was rejected because
  it recreates copy-paste maintenance.
- Requiring components to use only the `hss_framework` umbrella target was
  rejected because it hides dependencies and needlessly enables integrations.

## Supersedes

None.
