#pragma once

#include "main.h"

#define HSS_BOARD_HAS_WATCHDOG 1
#define HSS_BOARD_WATCHDOG_KIND_IWDG 0
#define HSS_BOARD_WATCHDOG_KIND_WWDG 1
#define HSS_BOARD_WATCHDOG_HANDLE hwwdg

extern WWDG_HandleTypeDef hwwdg;
