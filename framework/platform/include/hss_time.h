#pragma once
#include <stdint.h>

typedef void (*hss_tick_cb_t)(void);

void hss_time_init(void);
void hss_time_register_tick_cb(hss_tick_cb_t cb);
void hss_delay_ms(uint32_t ms);