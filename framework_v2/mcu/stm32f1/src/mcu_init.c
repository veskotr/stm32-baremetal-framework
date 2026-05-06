#include "mcu.h"
#include "stm32f1xx_hal.h"

extern void board_clock_config(void);

void mcu_init(void)
{
    HAL_Init();
    board_clock_config();
    SystemCoreClockUpdate();
}

void SysTick_Handler(void)
{
    HAL_IncTick();
}
