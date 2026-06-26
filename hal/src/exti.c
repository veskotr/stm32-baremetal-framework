#include "hss_exti.h"

#include <stddef.h>

bool hss_exti_input_is_available(const hss_exti_input_t *input)
{
    return input != NULL && input->pin != 0U;
}

hss_result_t hss_exti_input_enable(const hss_exti_input_t *input)
{
    if (!hss_exti_input_is_available(input))
    {
        return HSS_INVALID_ARGUMENT;
    }

    HAL_NVIC_SetPriority(input->irqn, input->priority, input->subpriority);
    HAL_NVIC_EnableIRQ(input->irqn);
    return HSS_OK;
}

hss_result_t hss_exti_input_disable(const hss_exti_input_t *input)
{
    if (!hss_exti_input_is_available(input))
    {
        return HSS_INVALID_ARGUMENT;
    }

    HAL_NVIC_DisableIRQ(input->irqn);
    return HSS_OK;
}

hss_result_t hss_exti_input_register_callback(const hss_exti_input_t *input,
                                              hss_gpio_interrupt_callback_t callback,
                                              void *context)
{
    if (!hss_exti_input_is_available(input))
    {
        return HSS_INVALID_ARGUMENT;
    }

    return hss_gpio_register_interrupt(input->pin, callback, context);
}
