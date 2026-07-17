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
#if HSS_ENABLE_MODBUS_DEBUG
volatile uint32_t hss_modbus_uart_debug_rx_count;
volatile uint32_t hss_modbus_uart_debug_tx_count;
#endif

#if HSS_BOARD_HAS_MODBUS_UART
static bool g_modbus_uart_rx_irq_enabled;
static bool g_modbus_uart_tx_irq_enabled;

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

hss_result_t hss_modbus_uart_configure(uint32_t baudrate,
                                       uint8_t data_bits,
                                       hss_modbus_uart_parity_t parity,
                                       hss_modbus_uart_stop_bits_t stop_bits)
{
#if HSS_BOARD_HAS_MODBUS_UART
    if (baudrate == 0U || data_bits < 7U || data_bits > 9U ||
        (stop_bits != HSS_MODBUS_UART_STOP_BITS_1 &&
         stop_bits != HSS_MODBUS_UART_STOP_BITS_2))
    {
        return HSS_INVALID_ARGUMENT;
    }

    UART_HandleTypeDef *uart = &HSS_BOARD_MODBUS_UART_HANDLE;

    (void)HAL_UART_Abort(uart);
    if (HAL_UART_DeInit(uart) != HAL_OK)
    {
        return HSS_ERROR;
    }

    uart->Init.BaudRate = baudrate;
    uart->Init.WordLength = (parity == HSS_MODBUS_UART_PARITY_NONE) ? UART_WORDLENGTH_8B : UART_WORDLENGTH_9B;
    uart->Init.StopBits = (stop_bits == HSS_MODBUS_UART_STOP_BITS_2) ?
                              UART_STOPBITS_2 :
                              UART_STOPBITS_1;
    uart->Init.Mode = UART_MODE_TX_RX;
    uart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
    uart->Init.OverSampling = UART_OVERSAMPLING_16;

    switch (parity)
    {
    case HSS_MODBUS_UART_PARITY_ODD:
        uart->Init.Parity = UART_PARITY_ODD;
        break;
    case HSS_MODBUS_UART_PARITY_EVEN:
        uart->Init.Parity = UART_PARITY_EVEN;
        break;
    case HSS_MODBUS_UART_PARITY_NONE:
    default:
        uart->Init.Parity = UART_PARITY_NONE;
        break;
    }

    g_modbus_uart_rx_irq_enabled = false;
    g_modbus_uart_tx_irq_enabled = false;

    return hss_result_from_hal_status(HAL_UART_Init(uart));
#else
    (void)baudrate;
    (void)data_bits;
    (void)parity;
    (void)stop_bits;
    return HSS_NOT_SUPPORTED;
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
    __HAL_UART_ENABLE_IT(&HSS_BOARD_MODBUS_UART_HANDLE, UART_IT_RXNE);
    return HSS_OK;
#else
    return HSS_NOT_SUPPORTED;
#endif
}

hss_result_t hss_modbus_uart_disable_rx_irq(void)
{
#if HSS_BOARD_HAS_MODBUS_UART
    g_modbus_uart_rx_irq_enabled = false;
    __HAL_UART_DISABLE_IT(&HSS_BOARD_MODBUS_UART_HANDLE, UART_IT_RXNE);
    return HSS_OK;
#else
    return HSS_NOT_SUPPORTED;
#endif
}

hss_result_t hss_modbus_uart_enable_tx_empty_irq(void)
{
#if HSS_BOARD_HAS_MODBUS_UART
    g_modbus_uart_tx_irq_enabled = true;
    __HAL_UART_ENABLE_IT(&HSS_BOARD_MODBUS_UART_HANDLE, UART_IT_TXE);
    return HSS_OK;
#else
    return HSS_NOT_SUPPORTED;
#endif
}

hss_result_t hss_modbus_uart_disable_tx_empty_irq(void)
{
#if HSS_BOARD_HAS_MODBUS_UART
    g_modbus_uart_tx_irq_enabled = false;
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
#if HSS_ENABLE_MODBUS_DEBUG
    hss_modbus_uart_debug_tx_count++;
#endif
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
    *byte = hss_modbus_uart_read_data_register(&HSS_BOARD_MODBUS_UART_HANDLE);
    return HSS_OK;
#else
    return HSS_NOT_SUPPORTED;
#endif
}

#if HSS_BOARD_HAS_MODBUS_UART
int uart1_irq_handler(void)
{
    UART_HandleTypeDef *uart = &HSS_BOARD_MODBUS_UART_HANDLE;
    int handled = 0;

    if (__HAL_UART_GET_FLAG(uart, UART_FLAG_RXNE) &&
        __HAL_UART_GET_IT_SOURCE(uart, UART_IT_RXNE))
    {
#if HSS_ENABLE_MODBUS_DEBUG
        hss_modbus_uart_debug_rx_count++;
#endif

        if (g_modbus_uart_rx_callback != 0)
        {
            g_modbus_uart_rx_callback(g_modbus_uart_rx_context);
        }

        handled = 1;
    }

    if (__HAL_UART_GET_FLAG(uart, UART_FLAG_TXE) &&
        __HAL_UART_GET_IT_SOURCE(uart, UART_IT_TXE))
    {
        if (g_modbus_uart_tx_empty_callback != 0)
        {
            g_modbus_uart_tx_empty_callback(g_modbus_uart_tx_empty_context);
        }

        handled = 1;
    }

    if (handled)
    {
#if defined(UART_FLAG_ORE)
        if (__HAL_UART_GET_FLAG(uart, UART_FLAG_ORE))
        {
            __HAL_UART_CLEAR_OREFLAG(uart);
        }
#endif
        (void)g_modbus_uart_rx_irq_enabled;
        (void)g_modbus_uart_tx_irq_enabled;
    }

    return handled;
}
#endif
