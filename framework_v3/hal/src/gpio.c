#include "hss_gpio.h"
#include <stddef.h>

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
