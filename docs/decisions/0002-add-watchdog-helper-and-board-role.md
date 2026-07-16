# 0002: Add watchdog helper and board role

- Status: Accepted
- Date: 2026-07-16
- Deciders: project maintainers

## Context

Applications need a small, repeatable way to refresh an STM32 watchdog without
duplicating HAL calls and board-specific handle selection. Watchdog timing and
startup configuration remain safety-critical CubeMX concerns and must not be
silently changed by framework policy.

## Decision

The framework provides thin IWDG and WWDG helpers that accept STM32 HAL handles
directly, plus an optional `BOARD_ROLE_WATCHDOG` role that selects one primary
application watchdog. The role accepts `IWDG` or `WWDG` and is rendered into the
generated board-role header.

CubeMX continues to own watchdog clocking, prescaler, reload, window,
initialization, and interrupt configuration. Applications explicitly choose
whether to call the framework watchdog helpers and control when refreshes occur.
`HSS_ENABLE_WATCHDOG` is an application-facing compile-time policy toggle; it
does not configure or start a watchdog by itself.

## Consequences

- Applications can use direct HAL-handle helpers or the selected board role.
- Boards without a watchdog role report `HSS_NOT_SUPPORTED` from the board-level
  helpers.
- Enabling a watchdog remains explicit, which avoids accidental IWDG startup.
- A board role must agree with the CubeMX-enabled watchdog peripheral and its
  generated handle.

## Alternatives considered

- A framework-owned watchdog configuration layer was rejected because it would
  duplicate CubeMX peripheral setup and obscure safety-critical timing choices.
- An automatic call from `hss_platform_init()` was rejected because watchdog
  startup and refresh policy are product decisions.
- A multi-client watchdog supervisor was deferred; it can be added later above
  this thin HAL and board-role layer.

## Supersedes

None.
