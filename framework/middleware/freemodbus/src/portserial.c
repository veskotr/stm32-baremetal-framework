//
// Created by vesko on 16.4.2026 г..
//
#include "port.h"
#include "hss_uart.h"
#include "mb.h"

hss_uart_id_t g_modbus_uart;

static void modbus_rx_cb(hss_uart_id_t uart)
{
    (void)uart;
    pxMBFrameCBByteReceived();
}

static void modbus_tx_cb(hss_uart_id_t uart)
{
    (void)uart;
    pxMBFrameCBTransmitterEmpty();
}

BOOL xMBPortSerialInit(UCHAR port, ULONG baud, UCHAR data_bits, eMBParity parity)
{
    (void)port;
    (void)baud;
    (void)data_bits;
    (void)parity;

    hss_uart_register_rx_callback(g_modbus_uart, modbus_rx_cb);
    hss_uart_register_txe_callback(g_modbus_uart, modbus_tx_cb);
    return TRUE;
}

void vMBPortSerialEnable(BOOL rxEnable, BOOL txEnable)
{
    if (rxEnable)
        hss_uart_enable_rx_irq(g_modbus_uart);
    else
        hss_uart_disable_rx_irq(g_modbus_uart);

    if (txEnable)
        hss_uart_enable_tx_irq(g_modbus_uart);
    else
        hss_uart_disable_tx_irq(g_modbus_uart);
}

BOOL xMBPortSerialPutByte(CHAR byte)
{
    hss_uart_write_tx_register(g_modbus_uart, (uint8_t)byte);
    return TRUE;
}

BOOL xMBPortSerialGetByte(CHAR* byte)
{
    uint8_t b;

    if (hss_uart_read(g_modbus_uart, &b, 1) == 1)
    {
        *byte = b;
        return TRUE;
    }

    return FALSE;
}
