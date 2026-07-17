# 0004: Split explicit HAL and board conveniences

- Status: Accepted
- Date: 2026-07-17

## Context

The framework's former aggregate targets and mixed `hal/` layer made a
consumer's dependencies unclear. Explicit STM32 peripheral access and
board-role-backed conveniences were exposed together, while optional drivers
and protocols could be enabled indirectly. That made a small reusable
component harder to reason about and gave an application more framework than
it requested.

## Decision

The public framework boundary is split as follows:

- `hss_common` remains the dependency-light shared base.
- `hss_hal` provides explicit-handle STM32 HAL helpers only.
- `hss_board` provides selected board and semantic-role conveniences and links
  `hss_hal` plus the registered board integration.
- Drivers and protocols are separate opt-in targets. A firmware configuration
  links only integrations it enables.

The aggregate `hss_framework`, `hss_drivers`, and `hss_protocols` targets are
removed. Applications include `hss_hal.h` and/or `hss_board.h` according to
the API they use. Reusable components link the narrowest targets they need.

## Consequences

- The public dependency graph is visible in each target's CMake definition.
- Board-specific semantic roles no longer appear in the explicit HAL layer.
- Existing consumers that link `hss_framework` or use role-backed APIs through
  `hss_hal.h` must migrate; this is a breaking release.
- Optional integrations are absent from a baseline firmware unless its
  configuration requests them.

## Alternatives considered

- Keeping a convenience aggregate target was rejected because it hides
  dependencies and makes optional integrations less predictable.
- Moving all board helpers into each application was rejected because common
  board-role behavior would be copied between products.
- Hiding STM32 HAL handles behind another abstraction was rejected because it
  adds indirection without helping the framework's HAL-first use cases.

## Supersedes

This supersedes the aggregate-target boundary described in 0003. The
framework/component/product ownership decision in 0003 remains accepted.
