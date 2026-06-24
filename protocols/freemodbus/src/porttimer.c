#include <stdint.h>

#include "hss_modbus_timer.h"
#include "hss_modbus.h"
#include "mb.h"
#include "mbport.h"

static volatile uint32_t g_downcounter;
static uint32_t g_timeout_reload;
#if HSS_ENABLE_MODBUS_DEBUG
volatile uint32_t hss_modbus_debug_timer_expired_count;
#endif

static void hss_freemodbus_timer_tick(void *context)
{
    (void)context;

    if (g_downcounter > 0U)
    {
        g_downcounter--;
        if (g_downcounter == 0U)
        {
#if HSS_ENABLE_MODBUS_DEBUG
            hss_modbus_debug_timer_expired_count++;
#endif
            (void)pxMBPortCBTimerExpired();
        }
    }
}

BOOL xMBPortTimersInit(USHORT timeout_50us)
{
    if (!hss_modbus_timer_is_available())
    {
        return FALSE;
    }

    g_timeout_reload = timeout_50us;
    return hss_modbus_timer_set_callback(hss_freemodbus_timer_tick, 0) == HSS_OK ? TRUE : FALSE;
}

void vMBPortTimersEnable(void)
{
    g_downcounter = g_timeout_reload;
    (void)hss_modbus_timer_reset();
    (void)hss_modbus_timer_start_it();
}

void vMBPortTimersDisable(void)
{
    g_downcounter = 0U;
    (void)hss_modbus_timer_stop_it();
}
