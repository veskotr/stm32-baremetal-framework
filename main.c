#include "board_gpio.h"
#include "hss_platform.h"
#include "hss_gpio.h"
#include "hss_time.h"
#include "hss_modbus.h"

int app_main()
{
    const hss_modbus_config_t modbus_cfg = {
        .uart = HSS_UART1,
        .slave_id = 0x01,
        .baudrate = 9800,
        .timer = HSS_TIMER2
    };

    hss_modbus_init(&modbus_cfg);

    hss_modbus_set_holding_register_value(1, 0);
    hss_modbus_set_holding_register_value(2, 7);
    hss_modbus_set_holding_register_value(3, 0xBEEF);

    while (1)
    {
        uint16_t holding_val = 0;
        hss_modbus_get_holding_register_value(2, &holding_val);

        if (0xDEAD == holding_val)
        {
            hss_gpio_reset_pin(LED_GPIO);
        }
        else
        {
            hss_gpio_set_pin(LED_GPIO);
        }

        hss_modbus_poll();
    }
    return 0;
}
