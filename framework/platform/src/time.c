//
// Created by vesko on 17.4.2026 г..
//

#include "hss_time.h"
#include "main.h"

#define MAX_TICK_CALLBACKS 4

static hss_tick_cb_t tick_callbacks[MAX_TICK_CALLBACKS];
static size_t tick_callback_count = 0;

void hss_delay_ms(const uint32_t ms)
{
    HAL_Delay(ms);
}

void hss_time_init(void)
{
    // Nothing to do for now, HAL_Init() is called in platform init
}

void hss_time_register_tick_cb(hss_tick_cb_t cb)
{
    if (tick_callback_count < MAX_TICK_CALLBACKS)
    {
        tick_callbacks[tick_callback_count++] = cb;
    }
}

void board_systick_hook()
{
    for (uint8_t i = 0; i < MAX_TICK_CALLBACKS; i++)
    {
        if (i < tick_callback_count && tick_callbacks[i] != NULL)
        {
            tick_callbacks[i]();
        }
    }
}
