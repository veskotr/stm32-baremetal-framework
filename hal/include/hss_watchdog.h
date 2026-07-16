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

bool hss_watchdog_is_available(void);
hss_result_t hss_watchdog_init(void);
hss_result_t hss_watchdog_refresh(void);
