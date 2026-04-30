#pragma once

#include "hss_types.h"

void hss_gpio_init();

void hss_gpio_set_pin(hss_gpio_handle_t gpio_handle);

void hss_gpio_reset_pin(hss_gpio_handle_t gpio_handle) ;

void hss_gpio_toggle_pin(hss_gpio_handle_t gpio_handle);