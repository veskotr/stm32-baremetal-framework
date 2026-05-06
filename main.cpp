#include "stm32f1xx_hal.h"
#include "mcu.h"
#include "board.h"

int main()
{
    mcu_init();
    board_init();

    while (1)
    {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET); // ON
        HAL_Delay(500);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);   // OFF
        HAL_Delay(500);
    }
}