#include "hss_modbus_uart.h"

#include "hss_board_roles.h"
#include "hss_hal_result.h"
#include "hss_uart.h"

#if HSS_BOARD_HAS_MODBUS_UART
#include "usart.h"
#endif

static hss_modbus_uart_callback_t g_modbus_uart_rx_callback;
static void *g_modbus_uart_rx_context;
static hss_modbus_uart_callback_t g_modbus_uart_tx_empty_callback;
static void *g_modbus_uart_tx_empty_context;

#if HSS_BOARD_HAS_MODBUS_UART
static uint8_t g_modbus_uart_rx_byte;
static bool g_modbus_uart_rx_byte_available;
static bool g_modbus_uart_rx_irq_enabled;

static void hss_modbus_uart_write_data_register(UART_HandleTypeDef *uart, uint8_t byte)
{
#if defined(USART_TDR_TDR)
    uart->Instance->TDR = byte;
#else
    uart->Instance->DR = byte;
#endif
}

static uint8_t hss_modbus_uart_read_data_register(UART_HandleTypeDef *uart)
{
#if defined(USART_RDR_RDR)
    return (uint8_t)(uart->Instance->RDR & 0xFFU);
#else
    return (uint8_t)(uart->Instance->DR & 0xFFU);
#endif
}

static hss_result_t hss_modbus_uart_restart_rx(void)
{
    return hss_result_from_hal_status(HAL_UART_Receive_IT(&HSS_BOARD_MODBUS_UART_HANDLE,
                                                          &g_modbus_uart_rx_byte,
                                                          1U));
}
#endif

bool hss_modbus_uart_is_available(void)
{
#if HSS_BOARD_HAS_MODBUS_UART
    return true;
#else
    return false;
#endif
}

bool hss_modbus_uart_uses_manual_rs485_de(void)
{
#if HSS_BOARD_HAS_MODBUS_UART && HSS_BOARD_MODBUS_RS485_MANUAL_DE
    return true;
#else
    return false;
#endif
}

bool hss_modbus_uart_uses_hardware_rs485_de(void)
{
#if HSS_BOARD_HAS_MODBUS_UART && HSS_BOARD_MODBUS_RS485_HARDWARE_DE
    return true;
#else
    return false;
#endif
}

hss_result_t hss_modbus_uart_write(const uint8_t *data, uint16_t length, uint32_t timeout_ms)
{
#if HSS_BOARD_HAS_MODBUS_UART && HSS_BOARD_MODBUS_RS485_MANUAL_DE
    const hss_uart_rs485_config_t config = {
        .uart = &HSS_BOARD_MODBUS_UART_HANDLE,
        .de_port = HSS_BOARD_MODBUS_RS485_DE_PORT,
        .de_pin = HSS_BOARD_MODBUS_RS485_DE_PIN,
        .de_active_high = HSS_BOARD_MODBUS_RS485_DE_ACTIVE_HIGH != 0,
        .pre_tx_delay_ms = 0U,
        .post_tx_delay_ms = 0U,
    };

    return hss_uart_rs485_write(&config, data, length, timeout_ms);
#elif HSS_BOARD_HAS_MODBUS_UART
    return hss_uart_write(&HSS_BOARD_MODBUS_UART_HANDLE, data, length, timeout_ms);
#else
    (void)data;
    (void)length;
    (void)timeout_ms;
    return HSS_NOT_SUPPORTED;
#endif
}

hss_result_t hss_modbus_uart_read(uint8_t *data, uint16_t length, uint32_t timeout_ms)
{
#if HSS_BOARD_HAS_MODBUS_UART
    return hss_uart_read(&HSS_BOARD_MODBUS_UART_HANDLE, data, length, timeout_ms);
#else
    (void)data;
    (void)length;
    (void)timeout_ms;
    return HSS_NOT_SUPPORTED;
#endif
}

hss_result_t hss_modbus_uart_write_byte(uint8_t byte, uint32_t timeout_ms)
{
    return hss_modbus_uart_write(&byte, 1U, timeout_ms);
}

hss_result_t hss_modbus_uart_read_byte(uint8_t *byte, uint32_t timeout_ms)
{
    return hss_modbus_uart_read(byte, 1U, timeout_ms);
}

hss_result_t hss_modbus_uart_set_rx_callback(hss_modbus_uart_callback_t callback, void *context)
{
#if HSS_BOARD_HAS_MODBUS_UART
    g_modbus_uart_rx_callback = callback;
    g_modbus_uart_rx_context = context;
    return HSS_OK;
#else
    (void)callback;
    (void)context;
    return HSS_NOT_SUPPORTED;
#endif
}

hss_result_t hss_modbus_uart_set_tx_empty_callback(hss_modbus_uart_callback_t callback, void *context)
{
#if HSS_BOARD_HAS_MODBUS_UART
    g_modbus_uart_tx_empty_callback = callback;
    g_modbus_uart_tx_empty_context = context;
    return HSS_OK;
#else
    (void)callback;
    (void)context;
    return HSS_NOT_SUPPORTED;
#endif
}

hss_result_t hss_modbus_uart_enable_rx_irq(void)
{
#if HSS_BOARD_HAS_MODBUS_UART
    g_modbus_uart_rx_irq_enabled = true;
    g_modbus_uart_rx_byte_available = false;
    (void)HAL_UART_AbortReceive(&HSS_BOARD_MODBUS_UART_HANDLE);
    return hss_modbus_uart_restart_rx();
#else
    return HSS_NOT_SUPPORTED;
#endif
}

hss_result_t hss_modbus_uart_disable_rx_irq(void)
{
#if HSS_BOARD_HAS_MODBUS_UART
    g_modbus_uart_rx_irq_enabled = false;
    g_modbus_uart_rx_byte_available = false;
    return hss_result_from_hal_status(HAL_UART_AbortReceive(&HSS_BOARD_MODBUS_UART_HANDLE));
#else
    return HSS_NOT_SUPPORTED;
#endif
}

hss_result_t hss_modbus_uart_enable_tx_empty_irq(void)
{
#if HSS_BOARD_HAS_MODBUS_UART
    __HAL_UART_ENABLE_IT(&HSS_BOARD_MODBUS_UART_HANDLE, UART_IT_TXE);
    return HSS_OK;
#else
    return HSS_NOT_SUPPORTED;
#endif
}

hss_result_t hss_modbus_uart_disable_tx_empty_irq(void)
{
#if HSS_BOARD_HAS_MODBUS_UART
    __HAL_UART_DISABLE_IT(&HSS_BOARD_MODBUS_UART_HANDLE, UART_IT_TXE);
    return HSS_OK;
#else
    return HSS_NOT_SUPPORTED;
#endif
}

hss_result_t hss_modbus_uart_write_tx_register(uint8_t byte)
{
#if HSS_BOARD_HAS_MODBUS_UART
    hss_modbus_uart_write_data_register(&HSS_BOARD_MODBUS_UART_HANDLE, byte);
    return HSS_OK;
#else
    (void)byte;
    return HSS_NOT_SUPPORTED;
#endif
}

hss_result_t hss_modbus_uart_read_rx_register(uint8_t *byte)
{
    if (byte == 0)
    {
        return HSS_INVALID_ARGUMENT;
    }

#if HSS_BOARD_HAS_MODBUS_UART
    if (!g_modbus_uart_rx_byte_available)
    {
        return HSS_NOT_READY;
    }

    *byte = g_modbus_uart_rx_byte;
    g_modbus_uart_rx_byte_available = false;
    return HSS_OK;
#else
    return HSS_NOT_SUPPORTED;
#endif
}

#if HSS_BOARD_HAS_MODBUS_UART
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *uart)
{
    if (uart == &HSS_BOARD_MODBUS_UART_HANDLE)
    {
        g_modbus_uart_rx_byte_available = true;

        if (g_modbus_uart_rx_callback != 0)
        {
            g_modbus_uart_rx_callback(g_modbus_uart_rx_context);
        }

        if (g_modbus_uart_rx_irq_enabled)
        {
            (void)hss_modbus_uart_restart_rx();
        }
    }
}

void uart1_txe_handler(void)
{
    if (g_modbus_uart_tx_empty_callback != 0)
    {
        g_modbus_uart_tx_empty_callback(g_modbus_uart_tx_empty_context);
    }
}
#endif
