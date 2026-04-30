#include "hss_uart.h"
#include "usart.h"
#include "board_config.h"

#define HSS_UART_MAX_BUFFER_SIZE 128
#define HSS_UART_MAX_CALLBACKS 16
#define HSS_UART_MAX 3

static hss_uart_rx_cb_t uart_rx_callbacks[HSS_UART_MAX][HSS_UART_MAX_CALLBACKS];
static hss_uart_tx_cb_t uart_tx_callbacks[HSS_UART_MAX][HSS_UART_MAX_CALLBACKS];
static hss_uart_tx_cb_t uart_txe_callbacks[HSS_UART_MAX][HSS_UART_MAX_CALLBACKS];
static bool uart_rx_enabled[HSS_UART_MAX];
static bool uart_tx_enabled[HSS_UART_MAX];

#if BOARD_UART1
extern UART_HandleTypeDef huart1;
bool uart1_busy = false;
static uint8_t uart1_rx_byte;
static uint8_t uart1_rx_buf[HSS_UART_MAX_BUFFER_SIZE];
static volatile size_t uart1_rx_head = 0;
static volatile size_t uart1_rx_tail = 0;
#endif
#if BOARD_UART2
extern UART_HandleTypeDef huart2;
bool uart2_busy = false;
static uint8_t uart2_rx_byte;
static uint8_t uart2_rx_buf[HSS_UART_MAX_BUFFER_SIZE];
static volatile size_t uart2_rx_head = 0;
static volatile size_t uart2_rx_tail = 0;
#endif

void hss_uart_init()
{
#if BOARD_UART1
    MX_USART1_UART_Init();
    HAL_UART_Receive_IT(&huart1, &uart1_rx_byte, 1);
#endif
#if BOARD_UART2
    MX_USART2_UART_Init();
    HAL_UART_Receive_IT(&huart2, &uart2_rx_byte, 1);
#endif
#if BOARD_UART3
    MX_USART3_UART_Init();
#endif
}

void hss_uart_write_tx_register(const hss_uart_id_t uart, const uint8_t byte)
{
    switch (uart)
    {
#if BOARD_UART1
    case HSS_UART1:
        huart1.Instance->DR = byte;
        break;
#endif

#if BOARD_UART2
    case HSS_UART2:
        huart2.Instance->DR = byte;
        break;
#endif

    default:
        break;
    }
    }

void hss_uart_write(const hss_uart_id_t uart, const uint8_t* data, const size_t len)
{
    if (hss_uart_is_busy(uart))
    {
        return;
    }
    switch (uart)
    {
#if BOARD_UART1
    case HSS_UART1:
        HAL_UART_Transmit(&huart1, data, len, HAL_MAX_DELAY);
        break;
#endif
#if BOARD_UART2
    case HSS_UART2:
        HAL_UART_Transmit(&huart2, data, len, HAL_MAX_DELAY);
        break;
#endif

    default:
        break;
    }
}

void hss_uart_write_it(const hss_uart_id_t uart, const uint8_t* data, const size_t len)
{
    if (hss_uart_is_busy(uart))
    {
        return;
    }
    switch (uart)
    {
#if BOARD_UART1
    case HSS_UART1:
        uart1_busy = true;
        HAL_UART_Transmit_IT(&huart1, data, len);
        break;
#endif
#if BOARD_UART2
    case HSS_UART2:
        uart2_busy = true;
        HAL_UART_Transmit_IT(&huart2, data, len);
        break;
#endif

    default:
        break;
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef* huart)
{
#if BOARD_UART1
    if (huart == &huart1)
    {
        uart1_busy = false;

        if (uart_tx_enabled[HSS_UART1])
        {
            for (int i = 0; i < HSS_UART_MAX_CALLBACKS; i++)
            {
                if (uart_tx_callbacks[HSS_UART1][i])
                    uart_tx_callbacks[HSS_UART1][i](HSS_UART1);
            }
        }
    }
#endif

#if BOARD_UART2
    if (huart == &huart2)
    {
        uart2_busy = false;

        if (uart_tx_enabled[HSS_UART2])
        {
            for (int i = 0; i < HSS_UART_MAX_CALLBACKS; i++)
            {
                if (uart_tx_callbacks[HSS_UART2][i])
                    uart_tx_callbacks[HSS_UART2][i](HSS_UART2);
            }
        }
    }
#endif
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart)
{
#if BOARD_UART1
    if (huart == &huart1)
    {
        size_t next = (uart1_rx_head + 1) % HSS_UART_MAX_BUFFER_SIZE;

        if (next != uart1_rx_tail) // avoid overflow
        {
            uart1_rx_buf[uart1_rx_head] = uart1_rx_byte;
            uart1_rx_head = next;
        }
        if (uart_rx_enabled[HSS_UART1])
        {
            for (int i = 0; i < HSS_UART_MAX_CALLBACKS; i++)
            {
                if (uart_rx_callbacks[HSS_UART1][i])
                    uart_rx_callbacks[HSS_UART1][i](HSS_UART1);
            }
        }
        // restart reception
        if (uart_rx_enabled[HSS_UART1])
            HAL_UART_Receive_IT(&huart1, &uart1_rx_byte, 1);
    }
#endif

#if BOARD_UART2
    if (huart == &huart2)
    {
        size_t next = (uart2_rx_head + 1) % HSS_UART_MAX_BUFFER_SIZE;

        if (next != uart2_rx_tail)
        {
            uart2_rx_buf[uart2_rx_head] = uart2_rx_byte;
            uart2_rx_head = next;
        }
        if (uart_rx_enabled[HSS_UART2])
        {
            for (int i = 0; i < HSS_UART_MAX_CALLBACKS; i++)
            {
                if (uart_rx_callbacks[HSS_UART2][i])
                    uart_rx_callbacks[HSS_UART2][i](HSS_UART2);
            }
        }
        if (uart_rx_enabled[HSS_UART2])
            HAL_UART_Receive_IT(&huart2, &uart2_rx_byte, 1);
    }
#endif
}

size_t hss_uart_read(const hss_uart_id_t uart, uint8_t* out, const size_t max_len)
{
    size_t count = 0;

    switch (uart)
    {
#if BOARD_UART1
    case HSS_UART1:
        while ((uart1_rx_tail != uart1_rx_head) && (count < max_len))
        {
            out[count++] = uart1_rx_buf[uart1_rx_tail];
            uart1_rx_tail = (uart1_rx_tail + 1) % HSS_UART_MAX_BUFFER_SIZE;
        }
        break;
#endif

#if BOARD_UART2
    case HSS_UART2:
        while ((uart2_rx_tail != uart2_rx_head) && (count < max_len))
        {
            out[count++] = uart2_rx_buf[uart2_rx_tail];
            uart2_rx_tail = (uart2_rx_tail + 1) % HSS_UART_MAX_BUFFER_SIZE;
        }
        break;
#endif

    default:
        break;
    }

    return count;
}

bool hss_uart_is_busy(const hss_uart_id_t uart)
{
    switch (uart)
    {
#if BOARD_UART1
    case HSS_UART1:
        return uart1_busy;
#endif
#if BOARD_UART2
    case HSS_UART2:
        return uart2_busy;
#endif
    default:
        return false;
    }
}

void hss_uart_register_rx_callback(hss_uart_id_t uart, hss_uart_rx_cb_t cb)
{
    for (int i = 0; i < HSS_UART_MAX_CALLBACKS; i++)
    {
        if (uart_rx_callbacks[uart][i] == NULL)
        {
            uart_rx_callbacks[uart][i] = cb;
            return;
        }
    }
}

void hss_uart_register_tx_callback(hss_uart_id_t uart, hss_uart_tx_cb_t cb)
{
    for (int i = 0; i < HSS_UART_MAX_CALLBACKS; i++)
    {
        if (uart_tx_callbacks[uart][i] == NULL)
        {
            uart_tx_callbacks[uart][i] = cb;
            return;
        }
    }
}

void hss_uart_register_txe_callback(hss_uart_id_t uart, hss_uart_tx_cb_t cb)
{
    for (int i = 0; i < HSS_UART_MAX_CALLBACKS; i++)
    {
        if (uart_txe_callbacks[uart][i] == NULL)
        {
            uart_txe_callbacks[uart][i] = cb;
            return;
        }
    }
}

void uart1_txe_handler(void)
{
    for (int i = 0; i < HSS_UART_MAX_CALLBACKS; i++)
    {
        if (uart_txe_callbacks[HSS_UART1][i])
            uart_txe_callbacks[HSS_UART1][i](HSS_UART1);
    }
}

void hss_uart_enable_rx_irq(hss_uart_id_t uart)
{
    uart_rx_enabled[uart] = true;

    switch (uart)
    {
#if BOARD_UART1
    case HSS_UART1:
        // Restart RX state machine cleanly
        HAL_UART_AbortReceive(&huart1);

        HAL_UART_Receive_IT(&huart1, &uart1_rx_byte, 1);
        break;
#endif

#if BOARD_UART2
    case HSS_UART2:
        HAL_UART_AbortReceive(&huart2);

        HAL_UART_Receive_IT(&huart2, &uart2_rx_byte, 1);
        break;
#endif
    }
}

void hss_uart_disable_rx_irq(hss_uart_id_t uart)
{
    uart_rx_enabled[uart] = false;

    switch (uart)
    {
#if BOARD_UART1
    case HSS_UART1:
        HAL_UART_AbortReceive(&huart1); // 🔥 important
        break;
#endif

#if BOARD_UART2
    case HSS_UART2:
        HAL_UART_AbortReceive(&huart2);
        break;
#endif
    }
}

void hss_uart_enable_tx_irq(hss_uart_id_t uart)
{
    uart_tx_enabled[uart] = true;

    switch (uart)
    {
#if BOARD_UART1
    case HSS_UART1:
        __HAL_UART_ENABLE_IT(&huart1, UART_IT_TXE);
        break;
#endif
#if BOARD_UART2
    case HSS_UART2:
        __HAL_UART_ENABLE_IT(&huart2, UART_IT_TXE);
        break;
#endif
    }
}

void hss_uart_disable_tx_irq(hss_uart_id_t uart)
{
    uart_tx_enabled[uart] = false;

    switch (uart)
    {
#if BOARD_UART1
    case HSS_UART1:
        __HAL_UART_DISABLE_IT(&huart1, UART_IT_TXE);
        break;
#endif
#if BOARD_UART2
    case HSS_UART2:
        __HAL_UART_DISABLE_IT(&huart2, UART_IT_TXE);
        break;
#endif
    }
}
