#pragma once

#include <stddef.h>
#include <stdint.h>

typedef enum
{
    HSS_UART1 = 0,
    HSS_UART2,
    HSS_UART3
} hss_uart_id_t;

void hss_uart_init();

typedef void (*hss_uart_rx_cb_t)(hss_uart_id_t uart);
typedef void (*hss_uart_tx_cb_t)(hss_uart_id_t uart);
typedef void (*hss_uart_txe_cb_t)(hss_uart_id_t uart);

void hss_uart_register_rx_callback(hss_uart_id_t uart, hss_uart_rx_cb_t cb);
void hss_uart_register_txe_callback(hss_uart_id_t uart, hss_uart_txe_cb_t cb);
void hss_uart_register_tx_callback(hss_uart_id_t uart, hss_uart_tx_cb_t cb);

void hss_uart_enable_rx_irq(hss_uart_id_t uart);
void hss_uart_disable_rx_irq(hss_uart_id_t uart);

void hss_uart_enable_tx_irq(hss_uart_id_t uart);
void hss_uart_disable_tx_irq(hss_uart_id_t uart);

bool hss_uart_is_busy(const hss_uart_id_t uart);
size_t hss_uart_read(hss_uart_id_t uart, uint8_t* out, size_t max_len);
void hss_uart_write(const hss_uart_id_t uart, const uint8_t* data, const size_t len);
void hss_uart_write_it(const hss_uart_id_t uart, const uint8_t* data, const size_t len);
void hss_uart_write_tx_register(const hss_uart_id_t uart, const uint8_t byte);
