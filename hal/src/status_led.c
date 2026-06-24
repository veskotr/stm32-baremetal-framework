#include "hss_status_led.h"

#include "hss_board_roles.h"
#include "hss_gpio.h"

bool hss_status_led_is_available(void)
{
#if HSS_BOARD_HAS_STATUS_LED
    return true;
#else
    return false;
#endif
}

hss_result_t hss_status_led_write(bool on)
{
#if HSS_BOARD_HAS_STATUS_LED
#if HSS_BOARD_STATUS_LED_ACTIVE_LOW
    const GPIO_PinState state = on ? GPIO_PIN_RESET : GPIO_PIN_SET;
#else
    const GPIO_PinState state = on ? GPIO_PIN_SET : GPIO_PIN_RESET;
#endif

    return hss_gpio_write(HSS_BOARD_STATUS_LED_PORT, HSS_BOARD_STATUS_LED_PIN, state);
#else
    (void)on;
    return HSS_NOT_SUPPORTED;
#endif
}

hss_result_t hss_status_led_on(void)
{
    return hss_status_led_write(true);
}

hss_result_t hss_status_led_off(void)
{
    return hss_status_led_write(false);
}

hss_result_t hss_status_led_toggle(void)
{
#if HSS_BOARD_HAS_STATUS_LED
    return hss_gpio_toggle(HSS_BOARD_STATUS_LED_PORT, HSS_BOARD_STATUS_LED_PIN);
#else
    return HSS_NOT_SUPPORTED;
#endif
}
