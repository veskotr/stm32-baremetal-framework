//
// Created by vesko on 15.4.2026 г..
//

#include "hss_platform.h"
#include "main.h"
#include "hss_gpio.h"
#include "hss_uart.h"
#include "hss_dma.h"
#include "hss_timer.h"

void hss_platform_init()
{
    // Initialize all configured peripherals
    HAL_Init();
    clock_init();
    hss_gpio_init();
    hss_dma_init();
    hss_timer_init();
    hss_uart_init();
}

int main()
{
    hss_platform_init();
    return app_main();
}
