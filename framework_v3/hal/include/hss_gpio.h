#pragma once

#include <stdint.h>
#include "hss_result.h"
#include "main.h"

hss_result_t hss_gpio_write(GPIO_TypeDef *port, uint16_t pin, GPIO_PinState state);
hss_result_t hss_gpio_toggle(GPIO_TypeDef *port, uint16_t pin);
