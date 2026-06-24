#include "hss_modbus_timer.h"

#include "hss_board_roles.h"

#if HSS_BOARD_HAS_MODBUS_TIMER
#include "tim.h"
#endif

static hss_timer_callback_t g_modbus_timer_callback;
static void *g_modbus_timer_context;

bool hss_modbus_timer_is_available(void)
{
#if HSS_BOARD_HAS_MODBUS_TIMER
    return true;
#else
    return false;
#endif
}

hss_result_t hss_modbus_timer_set_callback(hss_timer_callback_t callback, void *context)
{
#if HSS_BOARD_HAS_MODBUS_TIMER
    g_modbus_timer_callback = callback;
    g_modbus_timer_context = context;
    return HSS_OK;
#else
    (void)callback;
    (void)context;
    return HSS_NOT_SUPPORTED;
#endif
}

hss_result_t hss_modbus_timer_start_it(void)
{
#if HSS_BOARD_HAS_MODBUS_TIMER
    return hss_timer_start_it(&HSS_BOARD_MODBUS_TIMER_HANDLE);
#else
    return HSS_NOT_SUPPORTED;
#endif
}

hss_result_t hss_modbus_timer_stop_it(void)
{
#if HSS_BOARD_HAS_MODBUS_TIMER
    return hss_timer_stop_it(&HSS_BOARD_MODBUS_TIMER_HANDLE);
#else
    return HSS_NOT_SUPPORTED;
#endif
}

hss_result_t hss_modbus_timer_reset(void)
{
#if HSS_BOARD_HAS_MODBUS_TIMER
    return hss_timer_reset(&HSS_BOARD_MODBUS_TIMER_HANDLE);
#else
    return HSS_NOT_SUPPORTED;
#endif
}

hss_result_t hss_modbus_timer_set_period_ticks(uint32_t period_ticks)
{
#if HSS_BOARD_HAS_MODBUS_TIMER
    return hss_timer_set_period_ticks(&HSS_BOARD_MODBUS_TIMER_HANDLE, period_ticks);
#else
    (void)period_ticks;
    return HSS_NOT_SUPPORTED;
#endif
}

hss_result_t hss_modbus_timer_get_properties(hss_timer_properties_t *properties)
{
#if HSS_BOARD_HAS_MODBUS_TIMER
    return hss_timer_get_properties(&HSS_BOARD_MODBUS_TIMER_HANDLE, properties);
#else
    (void)properties;
    return HSS_NOT_SUPPORTED;
#endif
}

#if HSS_BOARD_HAS_MODBUS_TIMER
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *timer)
{
    if (timer == &HSS_BOARD_MODBUS_TIMER_HANDLE && g_modbus_timer_callback != 0)
    {
        g_modbus_timer_callback(g_modbus_timer_context);
    }
}
#endif
