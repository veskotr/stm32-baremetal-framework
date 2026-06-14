#include "hss_platform.h"
#include "hss_time.h"
#include "main.h"

int main(void)
{
    hss_platform_init();

    while (1) {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
        hss_delay_ms(500);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
        hss_delay_ms(500);
    }
}
