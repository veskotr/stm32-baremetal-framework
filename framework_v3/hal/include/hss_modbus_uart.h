#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "hss_result.h"

#ifndef HSS_ENABLE_MODBUS_DEBUG
#define HSS_ENABLE_MODBUS_DEBUG 0
#endif

typedef void (*hss_modbus_uart_callback_t)(void *context);

typedef enum
{
    HSS_MODBUS_UART_PARITY_NONE = 0,
    HSS_MODBUS_UART_PARITY_ODD = 1,
    HSS_MODBUS_UART_PARITY_EVEN = 2,
} hss_modbus_uart_parity_t;

#if HSS_ENABLE_MODBUS_DEBUG
extern volatile uint32_t hss_modbus_uart_debug_rx_count;
extern volatile uint32_t hss_modbus_uart_debug_tx_count;
#endif

bool hss_modbus_uart_is_available(void);
bool hss_modbus_uart_uses_manual_rs485_de(void);
bool hss_modbus_uart_uses_hardware_rs485_de(void);
hss_result_t hss_modbus_uart_configure(uint32_t baudrate,
                                       uint8_t data_bits,
                                       hss_modbus_uart_parity_t parity);
hss_result_t hss_modbus_uart_write(const uint8_t *data, uint16_t length, uint32_t timeout_ms);
hss_result_t hss_modbus_uart_read(uint8_t *data, uint16_t length, uint32_t timeout_ms);
hss_result_t hss_modbus_uart_write_byte(uint8_t byte, uint32_t timeout_ms);
hss_result_t hss_modbus_uart_read_byte(uint8_t *byte, uint32_t timeout_ms);
hss_result_t hss_modbus_uart_set_rx_callback(hss_modbus_uart_callback_t callback, void *context);
hss_result_t hss_modbus_uart_set_tx_empty_callback(hss_modbus_uart_callback_t callback, void *context);
hss_result_t hss_modbus_uart_enable_rx_irq(void);
hss_result_t hss_modbus_uart_disable_rx_irq(void);
hss_result_t hss_modbus_uart_enable_tx_empty_irq(void);
hss_result_t hss_modbus_uart_disable_tx_empty_irq(void);
hss_result_t hss_modbus_uart_write_tx_register(uint8_t byte);
hss_result_t hss_modbus_uart_read_rx_register(uint8_t *byte);
