//
// Created by vesko on 17.4.2026 г..
//

#include <stdint.h>

#include "mb.h"
#include "mbport.h"
#include "hss_timer.h"

static volatile uint32_t downcounter = 0;
static uint32_t timeout_reload = 0;

hss_timer_id_t g_modbus_timer;

static void modbus_timer_tick(void)
{
    if (downcounter > 0)
    {
        if (--downcounter == 0)
        {
            pxMBPortCBTimerExpired();
        }
    }
}

BOOL xMBPortTimersInit(USHORT timeout50us)
{
    timeout_reload = timeout50us;

    hss_timer_set_cb(g_modbus_timer, modbus_timer_tick);

    return TRUE;
}

void vMBPortTimersEnable(void)
{
    downcounter = timeout_reload;
    hss_timer_start_it(g_modbus_timer);
}

void vMBPortTimersDisable(void)
{
    downcounter = 0;
    hss_timer_stop_it(g_modbus_timer);
}

