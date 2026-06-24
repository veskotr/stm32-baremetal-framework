#include "hss_spi.h"

#include <stddef.h>

#include "hss_gpio.h"
#include "hss_hal_result.h"

hss_result_t hss_spi_write(SPI_HandleTypeDef *spi,
                           const uint8_t *data,
                           uint16_t length,
                           uint32_t timeout_ms)
{
    if (spi == NULL || data == NULL || length == 0U)
    {
        return HSS_INVALID_ARGUMENT;
    }

#ifdef HAL_SPI_MODULE_ENABLED
    return hss_result_from_hal_status(HAL_SPI_Transmit(spi, (uint8_t *)data, length, timeout_ms));
#else
    (void)timeout_ms;
    return HSS_NOT_SUPPORTED;
#endif
}

hss_result_t hss_spi_read(SPI_HandleTypeDef *spi,
                          uint8_t *data,
                          uint16_t length,
                          uint32_t timeout_ms)
{
    if (spi == NULL || data == NULL || length == 0U)
    {
        return HSS_INVALID_ARGUMENT;
    }

#ifdef HAL_SPI_MODULE_ENABLED
    return hss_result_from_hal_status(HAL_SPI_Receive(spi, data, length, timeout_ms));
#else
    (void)timeout_ms;
    return HSS_NOT_SUPPORTED;
#endif
}

hss_result_t hss_spi_transfer(SPI_HandleTypeDef *spi,
                              const uint8_t *tx_data,
                              uint8_t *rx_data,
                              uint16_t length,
                              uint32_t timeout_ms)
{
    if (spi == NULL || tx_data == NULL || rx_data == NULL || length == 0U)
    {
        return HSS_INVALID_ARGUMENT;
    }

#ifdef HAL_SPI_MODULE_ENABLED
    return hss_result_from_hal_status(HAL_SPI_TransmitReceive(spi,
                                                              (uint8_t *)tx_data,
                                                              rx_data,
                                                              length,
                                                              timeout_ms));
#else
    (void)timeout_ms;
    return HSS_NOT_SUPPORTED;
#endif
}

hss_result_t hss_spi_device_select(const hss_spi_device_t *device)
{
    if (device == NULL || device->spi == NULL)
    {
        return HSS_INVALID_ARGUMENT;
    }

    if (device->cs_port == NULL || device->cs_pin == 0U)
    {
        return HSS_OK;
    }

    const GPIO_PinState state = device->cs_active_high ? GPIO_PIN_SET : GPIO_PIN_RESET;
    return hss_gpio_write(device->cs_port, device->cs_pin, state);
}

hss_result_t hss_spi_device_deselect(const hss_spi_device_t *device)
{
    if (device == NULL || device->spi == NULL)
    {
        return HSS_INVALID_ARGUMENT;
    }

    if (device->cs_port == NULL || device->cs_pin == 0U)
    {
        return HSS_OK;
    }

    const GPIO_PinState state = device->cs_active_high ? GPIO_PIN_RESET : GPIO_PIN_SET;
    return hss_gpio_write(device->cs_port, device->cs_pin, state);
}

hss_result_t hss_spi_device_write(const hss_spi_device_t *device,
                                  const uint8_t *data,
                                  uint16_t length,
                                  uint32_t timeout_ms)
{
    if (device == NULL)
    {
        return HSS_INVALID_ARGUMENT;
    }

    hss_result_t result = hss_spi_device_select(device);
    if (result != HSS_OK)
    {
        return result;
    }

    result = hss_spi_write(device->spi, data, length, timeout_ms);
    const hss_result_t deselect_result = hss_spi_device_deselect(device);
    return result == HSS_OK ? deselect_result : result;
}

hss_result_t hss_spi_device_read(const hss_spi_device_t *device,
                                 uint8_t *data,
                                 uint16_t length,
                                 uint32_t timeout_ms)
{
    if (device == NULL)
    {
        return HSS_INVALID_ARGUMENT;
    }

    hss_result_t result = hss_spi_device_select(device);
    if (result != HSS_OK)
    {
        return result;
    }

    result = hss_spi_read(device->spi, data, length, timeout_ms);
    const hss_result_t deselect_result = hss_spi_device_deselect(device);
    return result == HSS_OK ? deselect_result : result;
}

hss_result_t hss_spi_device_transfer(const hss_spi_device_t *device,
                                     const uint8_t *tx_data,
                                     uint8_t *rx_data,
                                     uint16_t length,
                                     uint32_t timeout_ms)
{
    if (device == NULL)
    {
        return HSS_INVALID_ARGUMENT;
    }

    hss_result_t result = hss_spi_device_select(device);
    if (result != HSS_OK)
    {
        return result;
    }

    result = hss_spi_transfer(device->spi, tx_data, rx_data, length, timeout_ms);
    const hss_result_t deselect_result = hss_spi_device_deselect(device);
    return result == HSS_OK ? deselect_result : result;
}
