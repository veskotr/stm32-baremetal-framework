#pragma once

#include <stdbool.h>

#include "hss_result.h"

bool hss_watchdog_is_available(void);
hss_result_t hss_watchdog_init(void);
hss_result_t hss_watchdog_refresh(void);
