#include "main.h"

void board_clock_config(void);
void board_init(void);

int main(void)
{
    HAL_Init();
    board_clock_config();
    board_init();

    while (1) {
    }
}
