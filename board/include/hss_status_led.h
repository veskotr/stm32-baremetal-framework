#pragma once

#include <stdbool.h>

#include "hss_result.h"

bool hss_status_led_is_available(void);
hss_result_t hss_status_led_write(bool on);
hss_result_t hss_status_led_on(void);
hss_result_t hss_status_led_off(void);
hss_result_t hss_status_led_toggle(void);
