#include "port.h"

#include "hss_modbus_uart.h"
#include "mb.h"

static void hss_freemodbus_rx_callback(void *context)
{
    (void)context;
    (void)pxMBFrameCBByteReceived();
}

static void hss_freemodbus_tx_empty_callback(void *context)
{
    (void)context;
    (void)pxMBFrameCBTransmitterEmpty();
}

static hss_modbus_uart_parity_t hss_freemodbus_uart_parity(eMBParity parity)
{
    switch (parity)
    {
    case MB_PAR_ODD:
        return HSS_MODBUS_UART_PARITY_ODD;
    case MB_PAR_EVEN:
        return HSS_MODBUS_UART_PARITY_EVEN;
    case MB_PAR_NONE:
    default:
        return HSS_MODBUS_UART_PARITY_NONE;
    }
}

BOOL xMBPortSerialInit(UCHAR port, ULONG baud, UCHAR data_bits, eMBParity parity)
{
    (void)port;

    if (!hss_modbus_uart_is_available())
    {
        return FALSE;
    }

    if (hss_modbus_uart_configure((uint32_t)baud,
                                  (uint8_t)data_bits,
                                  hss_freemodbus_uart_parity(parity)) != HSS_OK)
    {
        return FALSE;
    }

    if (hss_modbus_uart_set_rx_callback(hss_freemodbus_rx_callback, 0) != HSS_OK)
    {
        return FALSE;
    }

    if (hss_modbus_uart_set_tx_empty_callback(hss_freemodbus_tx_empty_callback, 0) != HSS_OK)
    {
        return FALSE;
    }

    return TRUE;
}

void vMBPortSerialEnable(BOOL rx_enable, BOOL tx_enable)
{
    if (rx_enable)
    {
        (void)hss_modbus_uart_enable_rx_irq();
    }
    else
    {
        (void)hss_modbus_uart_disable_rx_irq();
    }

    if (tx_enable)
    {
        (void)hss_modbus_uart_enable_tx_empty_irq();
    }
    else
    {
        (void)hss_modbus_uart_disable_tx_empty_irq();
    }
}

BOOL xMBPortSerialPutByte(CHAR byte)
{
    return hss_modbus_uart_write_tx_register((uint8_t)byte) == HSS_OK ? TRUE : FALSE;
}

BOOL xMBPortSerialGetByte(CHAR *byte)
{
    uint8_t raw_byte = 0U;

    if (byte == 0)
    {
        return FALSE;
    }

    if (hss_modbus_uart_read_rx_register(&raw_byte) != HSS_OK)
    {
        return FALSE;
    }

    *byte = (CHAR)raw_byte;
    return TRUE;
}
