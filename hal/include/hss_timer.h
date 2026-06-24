#pragma once

#include <stdint.h>

#include "hss_result.h"
#include "main.h"

#ifndef HAL_TIM_MODULE_ENABLED
typedef struct __TIM_HandleTypeDef TIM_HandleTypeDef;
#endif

typedef void (*hss_timer_callback_t)(void *context);

typedef struct
{
    uint32_t prescaler;
    uint32_t period;
    uint32_t counter;
} hss_timer_properties_t;

hss_result_t hss_timer_start_it(TIM_HandleTypeDef *timer);
hss_result_t hss_timer_stop_it(TIM_HandleTypeDef *timer);
hss_result_t hss_timer_reset(TIM_HandleTypeDef *timer);
hss_result_t hss_timer_set_period_ticks(TIM_HandleTypeDef *timer, uint32_t period_ticks);
hss_result_t hss_timer_get_properties(TIM_HandleTypeDef *timer, hss_timer_properties_t *properties);
