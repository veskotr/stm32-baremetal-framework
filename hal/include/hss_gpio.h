#pragma once

#include <stdint.h>
#include "hss_result.h"
#include "main.h"

typedef void (*hss_gpio_interrupt_callback_t)(uint16_t pin, void *context);

hss_result_t hss_gpio_read(GPIO_TypeDef *port, uint16_t pin, GPIO_PinState *state);
hss_result_t hss_gpio_write(GPIO_TypeDef *port, uint16_t pin, GPIO_PinState state);
hss_result_t hss_gpio_toggle(GPIO_TypeDef *port, uint16_t pin);
hss_result_t hss_gpio_register_interrupt(uint16_t pin,
                                         hss_gpio_interrupt_callback_t callback,
                                         void *context);
hss_result_t hss_gpio_unregister_interrupt(uint16_t pin);
hss_result_t hss_gpio_dispatch_interrupt(uint16_t pin);
