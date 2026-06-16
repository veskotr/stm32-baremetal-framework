#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "hss_result.h"
#include "main.h"

typedef struct
{
    UART_HandleTypeDef *uart;
    GPIO_TypeDef *de_port;
    uint16_t de_pin;
    bool de_active_high;
    uint32_t pre_tx_delay_ms;
    uint32_t post_tx_delay_ms;
} hss_uart_rs485_config_t;

hss_result_t hss_uart_write(UART_HandleTypeDef *uart,
                            const uint8_t *data,
                            uint16_t length,
                            uint32_t timeout_ms);
hss_result_t hss_uart_read(UART_HandleTypeDef *uart,
                           uint8_t *data,
                           uint16_t length,
                           uint32_t timeout_ms);
hss_result_t hss_uart_write_byte(UART_HandleTypeDef *uart,
                                 uint8_t byte,
                                 uint32_t timeout_ms);
hss_result_t hss_uart_rs485_write(const hss_uart_rs485_config_t *config,
                                  const uint8_t *data,
                                  uint16_t length,
                                  uint32_t timeout_ms);
