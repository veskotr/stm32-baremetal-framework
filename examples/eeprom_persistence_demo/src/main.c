#include "hss_config.h"
#include "hss_hal.h"
#include "main.h"

static uint16_t app_load_boot_count(void)
{
    uint16_t boot_count = 0U;
    const hss_result_t result = hss_eeprom_read_u16((uint16_t)CONFIG_APP_BOOT_COUNT_KEY, &boot_count);

    if (result == HSS_NOT_READY)
    {
        return 0U;
    }
    if (hss_result_is_error(result))
    {
        Error_Handler();
    }

    return boot_count;
}

int main(void)
{
    if (hss_platform_init() != HSS_OK)
    {
        Error_Handler();
    }

    if (hss_eeprom_init() != HSS_OK)
    {
        Error_Handler();
    }

    uint16_t boot_count = app_load_boot_count();
    boot_count++;

    if (hss_eeprom_write_u16((uint16_t)CONFIG_APP_BOOT_COUNT_KEY, boot_count) != HSS_OK)
    {
        Error_Handler();
    }

    while (1)
    {
        uint16_t blink_count = (uint16_t)(boot_count % 8U);
        if (blink_count == 0U)
        {
            blink_count = 8U;
        }

        for (uint16_t index = 0U; index < blink_count; ++index)
        {
            if (hss_status_led_on() != HSS_OK)
            {
                Error_Handler();
            }
            hss_delay_ms((uint32_t)CONFIG_APP_BLINK_ON_MS);

            if (hss_status_led_off() != HSS_OK)
            {
                Error_Handler();
            }
            hss_delay_ms((uint32_t)CONFIG_APP_BLINK_OFF_MS);
        }

        hss_delay_ms((uint32_t)CONFIG_APP_BLINK_GROUP_GAP_MS);
    }
}
