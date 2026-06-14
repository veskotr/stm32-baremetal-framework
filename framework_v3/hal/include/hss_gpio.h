#pragma once

#include <stdint.h>
#include "main.h"

void hss_gpio_write(GPIO_TypeDef *port, uint16_t pin, GPIO_PinState state);
void hss_gpio_toggle(GPIO_TypeDef *port, uint16_t pin);

