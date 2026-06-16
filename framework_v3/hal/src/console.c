#include "hss_console.h"

#include <stddef.h>
#include <string.h>

#include "hss_board_roles.h"
#include "hss_uart.h"

#if HSS_BOARD_HAS_CONSOLE_UART
#include "usart.h"
#endif

bool hss_console_is_available(void)
{
#if HSS_BOARD_HAS_CONSOLE_UART
    return true;
#else
    return false;
#endif
}

hss_result_t hss_console_write_bytes(const uint8_t *data, uint16_t length, uint32_t timeout_ms)
{
#if HSS_BOARD_HAS_CONSOLE_UART
    return hss_uart_write(&HSS_BOARD_CONSOLE_UART_HANDLE, data, length, timeout_ms);
#else
    (void)data;
    (void)length;
    (void)timeout_ms;
    return HSS_NOT_SUPPORTED;
#endif
}

hss_result_t hss_console_write(const char *text, uint32_t timeout_ms)
{
    if (text == NULL)
    {
        return HSS_INVALID_ARGUMENT;
    }

    const size_t length = strlen(text);
    if (length > UINT16_MAX)
    {
        return HSS_INVALID_ARGUMENT;
    }

    if (length == 0U)
    {
        return HSS_OK;
    }

    return hss_console_write_bytes((const uint8_t *)text, (uint16_t)length, timeout_ms);
}

hss_result_t hss_console_write_line(const char *text, uint32_t timeout_ms)
{
    hss_result_t result = hss_console_write(text, timeout_ms);
    if (result != HSS_OK)
    {
        return result;
    }

    return hss_console_write("\r\n", timeout_ms);
}

hss_result_t hss_console_read_bytes(uint8_t *data, uint16_t length, uint32_t timeout_ms)
{
#if HSS_BOARD_HAS_CONSOLE_UART
    return hss_uart_read(&HSS_BOARD_CONSOLE_UART_HANDLE, data, length, timeout_ms);
#else
    (void)data;
    (void)length;
    (void)timeout_ms;
    return HSS_NOT_SUPPORTED;
#endif
}

hss_result_t hss_console_read_byte(uint8_t *byte, uint32_t timeout_ms)
{
    return hss_console_read_bytes(byte, 1U, timeout_ms);
}
