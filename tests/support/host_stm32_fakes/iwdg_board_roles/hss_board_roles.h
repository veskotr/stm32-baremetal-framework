#pragma once

#include "main.h"

#define HSS_BOARD_HAS_WATCHDOG 1
#define HSS_BOARD_WATCHDOG_KIND_IWDG 1
#define HSS_BOARD_WATCHDOG_KIND_WWDG 0
#define HSS_BOARD_WATCHDOG_HANDLE hiwdg

extern IWDG_HandleTypeDef hiwdg;
