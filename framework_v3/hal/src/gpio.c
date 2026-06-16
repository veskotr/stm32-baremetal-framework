#include "hss_gpio.h"

#include <stddef.h>

typedef struct
{
    hss_gpio_interrupt_callback_t callback;
    void *context;
} hss_gpio_interrupt_slot_t;

static hss_gpio_interrupt_slot_t gpio_interrupt_slots[16];

static int hss_gpio_pin_index(uint16_t pin)
{
    if (pin == 0U || (pin & (uint16_t)(pin - 1U)) != 0U)
    {
        return -1;
    }

    for (int index = 0; index < 16; ++index)
    {
        if (pin == (uint16_t)(1U << index))
        {
            return index;
        }
    }

    return -1;
}

hss_result_t hss_gpio_read(GPIO_TypeDef *port, uint16_t pin, GPIO_PinState *state)
{
    if (port == NULL || pin == 0U || state == NULL)
    {
        return HSS_INVALID_ARGUMENT;
    }

    *state = HAL_GPIO_ReadPin(port, pin);
    return HSS_OK;
}

hss_result_t hss_gpio_write(GPIO_TypeDef *port, uint16_t pin, GPIO_PinState state)
{
    if (port == NULL || pin == 0U)
    {
        return HSS_INVALID_ARGUMENT;
    }

    HAL_GPIO_WritePin(port, pin, state);
    return HSS_OK;
}

hss_result_t hss_gpio_toggle(GPIO_TypeDef *port, uint16_t pin)
{
    if (port == NULL || pin == 0U)
    {
        return HSS_INVALID_ARGUMENT;
    }

    HAL_GPIO_TogglePin(port, pin);
    return HSS_OK;
}

hss_result_t hss_gpio_register_interrupt(uint16_t pin,
                                         hss_gpio_interrupt_callback_t callback,
                                         void *context)
{
    const int index = hss_gpio_pin_index(pin);
    if (index < 0 || callback == NULL)
    {
        return HSS_INVALID_ARGUMENT;
    }

    gpio_interrupt_slots[index].callback = callback;
    gpio_interrupt_slots[index].context = context;
    return HSS_OK;
}

hss_result_t hss_gpio_unregister_interrupt(uint16_t pin)
{
    const int index = hss_gpio_pin_index(pin);
    if (index < 0)
    {
        return HSS_INVALID_ARGUMENT;
    }

    gpio_interrupt_slots[index].callback = NULL;
    gpio_interrupt_slots[index].context = NULL;
    return HSS_OK;
}

hss_result_t hss_gpio_dispatch_interrupt(uint16_t pin)
{
    const int index = hss_gpio_pin_index(pin);
    if (index < 0)
    {
        return HSS_INVALID_ARGUMENT;
    }

    hss_gpio_interrupt_callback_t callback = gpio_interrupt_slots[index].callback;
    if (callback == NULL)
    {
        return HSS_NOT_READY;
    }

    callback(pin, gpio_interrupt_slots[index].context);
    return HSS_OK;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    (void)hss_gpio_dispatch_interrupt(GPIO_Pin);
}
