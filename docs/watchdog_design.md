# Watchdog Support Design

This note captures the intended first implementation of watchdog support for the
HSS STM32 framework. It is written as coding context for the implementation
model.

## Goal

Add watchdog support in the same style as the existing HAL helper layer:

- STM32 HAL-first, not a cross-vendor abstraction.
- CubeMX remains responsible for watchdog peripheral setup.
- Framework code provides consistent helper functions, board-role glue, config
  toggles, and tests.
- The first version should be small and reliable.

## Non-Goals For V1

- Do not compute watchdog prescaler/reload/window values in framework code.
- Do not replace CubeMX watchdog configuration.
- Do not automatically start a watchdog from `hss_platform_init()`.
- Do not add a full multi-subsystem watchdog supervisor yet.

The independent watchdog cannot be stopped once started, so enabling it must
remain an explicit application/product decision.

## Public API

Add:

```text
hal/include/hss_watchdog.h
hal/src/watchdog.c
board/include/hss_board_watchdog.h
board/src/watchdog.c
```

Proposed header:

```c
#pragma once

#include <stdbool.h>

#include "hss_result.h"
#include "main.h"

#ifndef HAL_IWDG_MODULE_ENABLED
typedef struct __IWDG_HandleTypeDef IWDG_HandleTypeDef;
#endif

#ifndef HAL_WWDG_MODULE_ENABLED
typedef struct __WWDG_HandleTypeDef WWDG_HandleTypeDef;
#endif

hss_result_t hss_iwdg_init(IWDG_HandleTypeDef *watchdog);
hss_result_t hss_iwdg_refresh(IWDG_HandleTypeDef *watchdog);

hss_result_t hss_wwdg_init(WWDG_HandleTypeDef *watchdog);
hss_result_t hss_wwdg_refresh(WWDG_HandleTypeDef *watchdog);
```

Low-level functions accept HAL handles directly, matching helpers such as
`hss_timer_*()` and `hss_spi_*()`.

The board-level declarations live in `hss_board_watchdog.h`, which is included
by the `hss_board.h` umbrella rather than `hss_hal.h`.

```c
bool hss_watchdog_is_available(void);
hss_result_t hss_watchdog_init(void);
hss_result_t hss_watchdog_refresh(void);
```

Board-level functions use generated board role macros and represent the primary
application watchdog for the firmware target.

## Low-Level Behavior

IWDG:

- `hss_iwdg_init(NULL)` returns `HSS_INVALID_ARGUMENT`.
- `hss_iwdg_refresh(NULL)` returns `HSS_INVALID_ARGUMENT`.
- If `HAL_IWDG_MODULE_ENABLED` is not defined, both functions return
  `HSS_NOT_SUPPORTED`.
- `hss_iwdg_init()` wraps `HAL_IWDG_Init()`.
- `hss_iwdg_refresh()` wraps `HAL_IWDG_Refresh()`.
- HAL statuses are converted with `hss_result_from_hal_status()`.

WWDG:

- `hss_wwdg_init(NULL)` returns `HSS_INVALID_ARGUMENT`.
- `hss_wwdg_refresh(NULL)` returns `HSS_INVALID_ARGUMENT`.
- If `HAL_WWDG_MODULE_ENABLED` is not defined, both functions return
  `HSS_NOT_SUPPORTED`.
- `hss_wwdg_init()` wraps `HAL_WWDG_Init()`.
- `hss_wwdg_refresh()` wraps `HAL_WWDG_Refresh()`.
- HAL statuses are converted with `hss_result_from_hal_status()`.

## Board Role

Add a new optional semantic role in board role files:

```cmake
set(BOARD_ROLE_WATCHDOG IWDG)
```

or:

```cmake
set(BOARD_ROLE_WATCHDOG WWDG)
```

`hss_write_board_roles_header()` should emit default macros when no role exists:

```c
#define HSS_BOARD_HAS_WATCHDOG 0
#define HSS_BOARD_WATCHDOG_KIND_IWDG 0
#define HSS_BOARD_WATCHDOG_KIND_WWDG 0
```

For IWDG:

```c
#define HSS_BOARD_HAS_WATCHDOG 1
#define HSS_BOARD_WATCHDOG_KIND_IWDG 1
#define HSS_BOARD_WATCHDOG_KIND_WWDG 0
#define HSS_BOARD_WATCHDOG_HANDLE hiwdg
```

For WWDG:

```c
#define HSS_BOARD_HAS_WATCHDOG 1
#define HSS_BOARD_WATCHDOG_KIND_IWDG 0
#define HSS_BOARD_WATCHDOG_KIND_WWDG 1
#define HSS_BOARD_WATCHDOG_HANDLE hwwdg
```

The generated role header should include `iwdg.h` for IWDG and `wwdg.h` for
WWDG when those headers exist for the board.

Invalid values for `BOARD_ROLE_WATCHDOG` should fail CMake configure with a
clear error. Accepted values are `IWDG` and `WWDG`, case-insensitive.

## Board-Level Behavior

`hss_watchdog_is_available()`:

- Returns `true` when `HSS_BOARD_HAS_WATCHDOG` is true.
- Returns `false` otherwise.

`hss_watchdog_init()`:

- For an IWDG board role, calls `hss_iwdg_init(&HSS_BOARD_WATCHDOG_HANDLE)`.
- For a WWDG board role, calls `hss_wwdg_init(&HSS_BOARD_WATCHDOG_HANDLE)`.
- Without a board role, returns `HSS_NOT_SUPPORTED`.

`hss_watchdog_refresh()`:

- For an IWDG board role, calls `hss_iwdg_refresh(&HSS_BOARD_WATCHDOG_HANDLE)`.
- For a WWDG board role, calls `hss_wwdg_refresh(&HSS_BOARD_WATCHDOG_HANDLE)`.
- Without a board role, returns `HSS_NOT_SUPPORTED`.

## Config

Add this framework config key:

```text
HSS_ENABLE_WATCHDOG=y
```

Default: `false`.

Purpose:

- Allows applications and examples to compile watchdog usage conditionally.
- Provides a visible product policy toggle.

It should not configure watchdog timing. CubeMX owns prescaler, reload, window,
clock source, generated init functions, and interrupt setup.

Update:

- `tools/config/model.py`
- `tools/config/render.py` framework compile definition list
- config docs/schema help if applicable

## CMake Integration

Update `hal/CMakeLists.txt`:

- Add `src/watchdog.c`.

Update `board/CMakeLists.txt`:

- Add `src/watchdog.c`.

Update board role generation:

- Add `WATCHDOG_DEFINES`.
- Append watchdog-specific include to `ROLE_EXTRA_INCLUDES` when needed.
- Include the watchdog defines in generated `hss_board_roles.h`.

Update board sync role template:

- Add commented examples for `BOARD_ROLE_WATCHDOG`.

## Suggested Implementation

Low-level implementation shape:

```c
hss_result_t hss_iwdg_init(IWDG_HandleTypeDef *watchdog)
{
    if (watchdog == NULL)
    {
        return HSS_INVALID_ARGUMENT;
    }

#ifdef HAL_IWDG_MODULE_ENABLED
    return hss_result_from_hal_status(HAL_IWDG_Init(watchdog));
#else
    return HSS_NOT_SUPPORTED;
#endif
}
```

Board-level implementation shape:

```c
hss_result_t hss_watchdog_refresh(void)
{
#if HSS_BOARD_HAS_WATCHDOG && HSS_BOARD_WATCHDOG_KIND_IWDG
    return hss_iwdg_refresh(&HSS_BOARD_WATCHDOG_HANDLE);
#elif HSS_BOARD_HAS_WATCHDOG && HSS_BOARD_WATCHDOG_KIND_WWDG
    return hss_wwdg_refresh(&HSS_BOARD_WATCHDOG_HANDLE);
#else
    return HSS_NOT_SUPPORTED;
#endif
}
```

Use `(void)watchdog;` where needed in unsupported compile paths if warnings
require it.

## Application Usage

Example:

```c
#include "hss_board.h"

int main(void)
{
    hss_platform_init();

#if HSS_ENABLE_WATCHDOG
    hss_watchdog_init();
#endif

    while (1)
    {
        app_poll();

#if HSS_ENABLE_WATCHDOG
        hss_watchdog_refresh();
#endif
    }
}
```

## Tests

Extend host fakes:

- Add `HAL_IWDG_MODULE_ENABLED`.
- Add `HAL_WWDG_MODULE_ENABLED`.
- Add fake `IWDG_HandleTypeDef` and `WWDG_HandleTypeDef`.
- Add `HAL_IWDG_Init()`.
- Add `HAL_IWDG_Refresh()`.
- Add `HAL_WWDG_Init()`.
- Add `HAL_WWDG_Refresh()`.
- Track last operation, last handle, and next HAL status.

Add:

```text
tests/integration/test_hss_watchdog_host.c
```

Test cases:

- Null IWDG handle returns `HSS_INVALID_ARGUMENT`.
- Null WWDG handle returns `HSS_INVALID_ARGUMENT`.
- IWDG init maps HAL statuses.
- IWDG refresh maps HAL statuses and records the handle.
- WWDG init maps HAL statuses.
- WWDG refresh maps HAL statuses and records the handle.
- Board-level wrapper returns `HSS_NOT_SUPPORTED` when no watchdog role exists.

Register the test in `tests/CMakeLists.txt`.

## Example Board Notes

The current example board HAL configs have IWDG and WWDG modules commented out.
That is fine for the framework implementation. Do not force-enable watchdogs in
existing example boards unless creating a dedicated watchdog example.

For a future dedicated example:

1. Enable IWDG in CubeMX.
2. Regenerate board files.
3. Add `set(BOARD_ROLE_WATCHDOG IWDG)` to the board role file.
4. Add `HSS_ENABLE_WATCHDOG=y` to the app config.
5. Call `hss_watchdog_init()` once after `hss_platform_init()`.
6. Call `hss_watchdog_refresh()` only after the application has completed one
   healthy loop iteration.

## Phase 2 Idea

After the helper layer works, add a watchdog supervisor. The supervisor should
refresh the hardware watchdog only after all registered subsystems check in.

Possible future API:

```c
hss_watchdog_supervisor_register(client_id, timeout_ms);
hss_watchdog_supervisor_check_in(client_id);
hss_watchdog_supervisor_poll(now_ms);
```

That is the safer long-term product behavior, but it should stay out of the
first implementation.
