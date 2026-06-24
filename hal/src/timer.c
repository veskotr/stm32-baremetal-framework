#include "hss_timer.h"

#include <stddef.h>

#include "hss_hal_result.h"

hss_result_t hss_timer_start_it(TIM_HandleTypeDef *timer)
{
    if (timer == NULL)
    {
        return HSS_INVALID_ARGUMENT;
    }

#ifdef HAL_TIM_MODULE_ENABLED
    return hss_result_from_hal_status(HAL_TIM_Base_Start_IT(timer));
#else
    return HSS_NOT_SUPPORTED;
#endif
}

hss_result_t hss_timer_stop_it(TIM_HandleTypeDef *timer)
{
    if (timer == NULL)
    {
        return HSS_INVALID_ARGUMENT;
    }

#ifdef HAL_TIM_MODULE_ENABLED
    return hss_result_from_hal_status(HAL_TIM_Base_Stop_IT(timer));
#else
    return HSS_NOT_SUPPORTED;
#endif
}

hss_result_t hss_timer_reset(TIM_HandleTypeDef *timer)
{
    if (timer == NULL)
    {
        return HSS_INVALID_ARGUMENT;
    }

#ifdef HAL_TIM_MODULE_ENABLED
    __HAL_TIM_SET_COUNTER(timer, 0U);
    return HSS_OK;
#else
    return HSS_NOT_SUPPORTED;
#endif
}

hss_result_t hss_timer_set_period_ticks(TIM_HandleTypeDef *timer, uint32_t period_ticks)
{
    if (timer == NULL || period_ticks == 0U)
    {
        return HSS_INVALID_ARGUMENT;
    }

#ifdef HAL_TIM_MODULE_ENABLED
    __HAL_TIM_SET_AUTORELOAD(timer, period_ticks - 1U);
    return HSS_OK;
#else
    return HSS_NOT_SUPPORTED;
#endif
}

hss_result_t hss_timer_get_properties(TIM_HandleTypeDef *timer, hss_timer_properties_t *properties)
{
    if (timer == NULL || properties == NULL)
    {
        return HSS_INVALID_ARGUMENT;
    }

#ifdef HAL_TIM_MODULE_ENABLED
    properties->prescaler = timer->Init.Prescaler;
    properties->period = timer->Init.Period;
    properties->counter = __HAL_TIM_GET_COUNTER(timer);
    return HSS_OK;
#else
    (void)properties;
    return HSS_NOT_SUPPORTED;
#endif
}
