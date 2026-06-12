#include "main.h"

void board_clock_config(void);
void board_init(void);

int main(void)
{
    HAL_Init();
    board_clock_config();
    board_init();

    while (1) {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
        HAL_Delay(500);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
        HAL_Delay(500);
    }
}
