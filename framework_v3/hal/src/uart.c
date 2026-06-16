#include "hss_uart.h"

#include <stddef.h>

#include "hss_gpio.h"
#include "hss_hal_result.h"
#include "hss_time.h"

hss_result_t hss_uart_write(UART_HandleTypeDef *uart,
                            const uint8_t *data,
                            uint16_t length,
                            uint32_t timeout_ms)
{
    if (uart == NULL || data == NULL || length == 0U)
    {
        return HSS_INVALID_ARGUMENT;
    }

    return hss_result_from_hal_status(HAL_UART_Transmit(uart, (uint8_t *)data, length, timeout_ms));
}

hss_result_t hss_uart_read(UART_HandleTypeDef *uart,
                           uint8_t *data,
                           uint16_t length,
                           uint32_t timeout_ms)
{
    if (uart == NULL || data == NULL || length == 0U)
    {
        return HSS_INVALID_ARGUMENT;
    }

    return hss_result_from_hal_status(HAL_UART_Receive(uart, data, length, timeout_ms));
}

hss_result_t hss_uart_write_byte(UART_HandleTypeDef *uart,
                                 uint8_t byte,
                                 uint32_t timeout_ms)
{
    return hss_uart_write(uart, &byte, 1U, timeout_ms);
}

hss_result_t hss_uart_rs485_write(const hss_uart_rs485_config_t *config,
                                  const uint8_t *data,
                                  uint16_t length,
                                  uint32_t timeout_ms)
{
    if (config == NULL || config->uart == NULL || data == NULL || length == 0U)
    {
        return HSS_INVALID_ARGUMENT;
    }

    const bool has_de = config->de_port != NULL && config->de_pin != 0U;
    if (!has_de)
    {
        return hss_uart_write(config->uart, data, length, timeout_ms);
    }

    const GPIO_PinState active_state = config->de_active_high ? GPIO_PIN_SET : GPIO_PIN_RESET;
    const GPIO_PinState inactive_state = config->de_active_high ? GPIO_PIN_RESET : GPIO_PIN_SET;

    hss_result_t result = hss_gpio_write(config->de_port, config->de_pin, active_state);
    if (result != HSS_OK)
    {
        return result;
    }

    if (config->pre_tx_delay_ms > 0U)
    {
        hss_delay_ms(config->pre_tx_delay_ms);
    }

    result = hss_uart_write(config->uart, data, length, timeout_ms);

    if (config->post_tx_delay_ms > 0U)
    {
        hss_delay_ms(config->post_tx_delay_ms);
    }

    const hss_result_t de_result = hss_gpio_write(config->de_port, config->de_pin, inactive_state);
    if (result != HSS_OK)
    {
        return result;
    }

    return de_result;
}
