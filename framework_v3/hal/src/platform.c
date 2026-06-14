#include "hss_platform.h"
#include "main.h"

void board_clock_config(void);
void board_init(void);

void hss_platform_init(void)
{
    HAL_Init();
    board_clock_config();
    board_init();
}

