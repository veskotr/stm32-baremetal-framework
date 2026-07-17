#include "hss_hal.h"
#include "hss_board.h"
#include "hss_modbus.h"
#include "main.h"

#define MODBUS_SLAVE_ID 1U
#define MODBUS_BAUDRATE 115200U
#define MODBUS_REGISTER_COUNT 16U

static uint16_t holding_registers[MODBUS_REGISTER_COUNT];
static uint16_t persisted_holding_registers[MODBUS_REGISTER_COUNT];
static uint16_t input_registers[MODBUS_REGISTER_COUNT];

static void app_init_persistence(void)
{
    if (hss_eeprom_init() != HSS_OK)
    {
        Error_Handler();
    }

    if (hss_eeprom_load_u16_array(0U, holding_registers, MODBUS_REGISTER_COUNT) != HSS_OK)
    {
        Error_Handler();
    }

    for (uint16_t index = 0U; index < MODBUS_REGISTER_COUNT; ++index)
    {
        persisted_holding_registers[index] = holding_registers[index];
    }
}

static void app_persist_changed_holding_registers(void)
{
    for (uint16_t index = 0U; index < MODBUS_REGISTER_COUNT; ++index)
    {
        if (holding_registers[index] == persisted_holding_registers[index])
        {
            continue;
        }

        if (hss_eeprom_write_u16(index, holding_registers[index]) != HSS_OK)
        {
            Error_Handler();
        }

        persisted_holding_registers[index] = holding_registers[index];
    }
}

static void app_init_modbus(void)
{
    hss_modbus_config_t config = hss_modbus_default_config(MODBUS_SLAVE_ID, MODBUS_BAUDRATE);
    config.holding_registers.start_address = 0U;
    config.holding_registers.count = MODBUS_REGISTER_COUNT;
    config.holding_registers.values = holding_registers;
    config.input_registers.start_address = 0U;
    config.input_registers.count = MODBUS_REGISTER_COUNT;
    config.input_registers.values = input_registers;

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

    app_init_persistence();
    app_init_modbus();

    uint32_t last_status_tick = HAL_GetTick();

    while (1)
    {
        if (hss_modbus_poll() != HSS_OK)
        {
            Error_Handler();
        }
        app_persist_changed_holding_registers();

        const uint32_t now = HAL_GetTick();
        if ((now - last_status_tick) >= 500U)
        {
            last_status_tick = now;
            input_registers[0]++;
            (void)hss_status_led_toggle();
        }
    }
}
