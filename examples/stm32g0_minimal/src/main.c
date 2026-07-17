#include "hss_hal.h"
#include "hss_board.h"
#include "main.h"
#include <stdio.h>

int main(void)
{
    if (hss_platform_init() != HSS_OK) {
        Error_Handler();
    }

    if (hss_console_is_available()) {
        printf("stm32g0_minimal boot\r\n");
    }

    while (1) {
    }
}
