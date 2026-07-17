#include "hss_platform.h"
#include "hss_hal_result.h"
#include "main.h"

void board_clock_config(void);
void board_init(void);

hss_result_t hss_platform_init(void)
{
    hss_result_t init_result = hss_result_from_hal_status(HAL_Init());
    if (hss_result_is_error(init_result))
    {
        return init_result;
    }

    board_clock_config();
    board_init();

    return HSS_OK;
}
