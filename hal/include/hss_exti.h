#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "hss_gpio.h"
#include "hss_result.h"
#include "main.h"

typedef struct
{
    uint16_t pin;
    IRQn_Type irqn;
    uint32_t priority;
    uint32_t subpriority;
} hss_exti_input_t;

bool hss_exti_input_is_available(const hss_exti_input_t *input);
hss_result_t hss_exti_input_enable(const hss_exti_input_t *input);
hss_result_t hss_exti_input_disable(const hss_exti_input_t *input);
hss_result_t hss_exti_input_register_callback(const hss_exti_input_t *input,
                                              hss_gpio_interrupt_callback_t callback,
                                              void *context);
