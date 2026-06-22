#include "hss_hal.h"
#include "hss_modbus.h"
#include "main.h"

#define MODBUS_SLAVE_ID 1U
#define MODBUS_BAUDRATE 9600U
#define MODBUS_REGISTER_COUNT 16U
#define MODBUS_TEST_REGISTER 2U
#define MODBUS_WRITE_LED_PULSE_MS 120U

static uint16_t holding_registers[MODBUS_REGISTER_COUNT];
static uint16_t input_registers[MODBUS_REGISTER_COUNT];

static void app_init_modbus(void)
{
    hss_modbus_config_t config = hss_modbus_default_config(MODBUS_SLAVE_ID, MODBUS_BAUDRATE);
    config.holding_registers.start_address = 0U;
    config.holding_registers.count = MODBUS_REGISTER_COUNT;
    config.holding_registers.values = holding_registers;
    config.input_registers.start_address = 0U;
    config.input_registers.count = MODBUS_REGISTER_COUNT;
    config.input_registers.values = input_registers;

    holding_registers[0] = 0x1234U;
    holding_registers[1] = 0x0001U;
    holding_registers[MODBUS_TEST_REGISTER] = 0U;
    input_registers[0] = 0U;
    input_registers[1] = 0xB105U;
    input_registers[2] = holding_registers[MODBUS_TEST_REGISTER];

    if (hss_modbus_init(&config) != HSS_OK)
    {
        Error_Handler();
    }

    if (hss_modbus_enable() != HSS_OK)
    {
        Error_Handler();
    }
}

int main(void)
{
    if (hss_platform_init() != HSS_OK)
    {
        Error_Handler();
    }

    app_init_modbus();
    (void)hss_status_led_off();

    uint16_t last_test_register = holding_registers[MODBUS_TEST_REGISTER];
    uint32_t led_off_tick = 0U;

    while (1)
    {
        if (hss_modbus_poll() != HSS_OK)
        {
            Error_Handler();
        }

        const uint32_t now = HAL_GetTick();
        const uint16_t test_register = holding_registers[MODBUS_TEST_REGISTER];
        if (test_register != last_test_register)
        {
            last_test_register = test_register;
            input_registers[0]++;
            input_registers[2] = test_register;
            led_off_tick = now + MODBUS_WRITE_LED_PULSE_MS;
            (void)hss_status_led_on();
        }

        if (led_off_tick != 0U && (int32_t)(now - led_off_tick) >= 0)
        {
            led_off_tick = 0U;
            (void)hss_status_led_off();
        }
    }
}
