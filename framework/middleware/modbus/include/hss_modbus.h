#pragma once
#include <stdint.h>

#include "hss_timer.h"
#include "hss_uart.h"

typedef struct
{
    hss_uart_id_t uart;
    uint8_t slave_id;
    uint32_t baudrate;
    hss_timer_id_t timer;
} hss_modbus_config_t;

void hss_modbus_init(const hss_modbus_config_t *cfg);
void hss_modbus_poll(void);

void hss_modbus_get_holding_register_value(uint16_t reg_addr, uint16_t *value);
void hss_modbus_get_input_register_value(uint16_t reg_addr, uint16_t *value);

void hss_modbus_set_holding_register_value(uint16_t reg_addr, uint16_t value);
void hss_modbus_set_input_register_value(uint16_t reg_addr, uint16_t value);