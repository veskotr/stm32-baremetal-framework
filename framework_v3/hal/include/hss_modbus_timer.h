#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "hss_result.h"
#include "hss_timer.h"

bool hss_modbus_timer_is_available(void);
hss_result_t hss_modbus_timer_set_callback(hss_timer_callback_t callback, void *context);
hss_result_t hss_modbus_timer_start_it(void);
hss_result_t hss_modbus_timer_stop_it(void);
hss_result_t hss_modbus_timer_reset(void);
hss_result_t hss_modbus_timer_set_period_ticks(uint32_t period_ticks);
hss_result_t hss_modbus_timer_get_properties(hss_timer_properties_t *properties);
