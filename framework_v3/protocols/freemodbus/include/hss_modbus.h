#pragma once

#include <stdint.h>

#include "hss_result.h"

typedef enum
{
    HSS_MODBUS_MODE_RTU = 0,
    HSS_MODBUS_MODE_ASCII = 1,
} hss_modbus_mode_t;

typedef enum
{
    HSS_MODBUS_PARITY_NONE = 0,
    HSS_MODBUS_PARITY_ODD = 1,
    HSS_MODBUS_PARITY_EVEN = 2,
} hss_modbus_parity_t;

typedef struct
{
    uint16_t start_address;
    uint16_t count;
    uint16_t *values;
} hss_modbus_register_bank_t;

typedef struct
{
    uint8_t slave_id;
    uint32_t baudrate;
    hss_modbus_mode_t mode;
    hss_modbus_parity_t parity;
    hss_modbus_register_bank_t holding_registers;
    hss_modbus_register_bank_t input_registers;
} hss_modbus_config_t;

hss_modbus_config_t hss_modbus_default_config(uint8_t slave_id, uint32_t baudrate);

hss_result_t hss_modbus_init(const hss_modbus_config_t *config);
hss_result_t hss_modbus_enable(void);
hss_result_t hss_modbus_disable(void);
hss_result_t hss_modbus_poll(void);

hss_result_t hss_modbus_get_holding_register(uint16_t address, uint16_t *value);
hss_result_t hss_modbus_set_holding_register(uint16_t address, uint16_t value);
hss_result_t hss_modbus_get_input_register(uint16_t address, uint16_t *value);
hss_result_t hss_modbus_set_input_register(uint16_t address, uint16_t value);
