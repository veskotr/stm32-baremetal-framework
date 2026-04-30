#include "hss_gpio.h"
#include "gpio.h"

//TODO make configurable based on board
static GPIO_TypeDef* gpio_ports[] = {GPIOA, GPIOB, GPIOC, GPIOD, GPIOE};

void hss_gpio_init()
{
    MX_GPIO_Init();
}

void hss_gpio_set_pin(hss_gpio_handle_t gpio_handle)
{
    HAL_GPIO_WritePin(gpio_ports[gpio_handle.port], gpio_handle.pin, GPIO_PIN_SET);
}

void hss_gpio_reset_pin(hss_gpio_handle_t gpio_handle)
{
    HAL_GPIO_WritePin(gpio_ports[gpio_handle.port], gpio_handle.pin,  GPIO_PIN_RESET);
}

void hss_gpio_toggle_pin(hss_gpio_handle_t gpio_handle)
{
    HAL_GPIO_TogglePin(gpio_ports[gpio_handle.port], gpio_handle.pin);
}
