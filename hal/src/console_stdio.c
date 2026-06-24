#include "hss_console.h"

#ifndef HSS_CONSOLE_STDIO_TIMEOUT_MS
#define HSS_CONSOLE_STDIO_TIMEOUT_MS 1000U
#endif

int __io_putchar(int ch)
{
    const unsigned char byte = (unsigned char)ch;

    if (hss_console_write_bytes(&byte, 1U, HSS_CONSOLE_STDIO_TIMEOUT_MS) != HSS_OK)
    {
        return -1;
    }

    return ch;
}

int __io_getchar(void)
{
    uint8_t byte = 0U;

    if (hss_console_read_byte(&byte, HSS_CONSOLE_STDIO_TIMEOUT_MS) != HSS_OK)
    {
        return -1;
    }

    return (int)byte;
}
