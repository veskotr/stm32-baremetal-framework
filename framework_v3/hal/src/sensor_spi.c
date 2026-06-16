#include "hss_sensor_spi.h"

#include <stddef.h>

#include "hss_board_roles.h"

bool hss_sensor_spi_is_available(void)
{
#if HSS_BOARD_HAS_SENSOR_SPI
    return true;
#else
    return false;
#endif
}

hss_result_t hss_sensor_spi_get_device(hss_spi_device_t *device)
{
    if (device == NULL)
    {
        return HSS_INVALID_ARGUMENT;
    }

#if HSS_BOARD_HAS_SENSOR_SPI
    device->spi = &HSS_BOARD_SENSOR_SPI_HANDLE;
#if HSS_BOARD_HAS_SENSOR_CS
    device->cs_port = HSS_BOARD_SENSOR_CS_PORT;
    device->cs_pin = HSS_BOARD_SENSOR_CS_PIN;
    device->cs_active_high = !HSS_BOARD_SENSOR_CS_ACTIVE_LOW;
#else
    device->cs_port = NULL;
    device->cs_pin = 0U;
    device->cs_active_high = false;
#endif
    return HSS_OK;
#else
    device->spi = NULL;
    device->cs_port = NULL;
    device->cs_pin = 0U;
    device->cs_active_high = false;
    return HSS_NOT_SUPPORTED;
#endif
}

hss_result_t hss_sensor_spi_write(const uint8_t *data, uint16_t length, uint32_t timeout_ms)
{
    hss_spi_device_t device;
    hss_result_t result = hss_sensor_spi_get_device(&device);
    if (result != HSS_OK)
    {
        return result;
    }

    return hss_spi_device_write(&device, data, length, timeout_ms);
}

hss_result_t hss_sensor_spi_read(uint8_t *data, uint16_t length, uint32_t timeout_ms)
{
    hss_spi_device_t device;
    hss_result_t result = hss_sensor_spi_get_device(&device);
    if (result != HSS_OK)
    {
        return result;
    }

    return hss_spi_device_read(&device, data, length, timeout_ms);
}

hss_result_t hss_sensor_spi_transfer(const uint8_t *tx_data,
                                     uint8_t *rx_data,
                                     uint16_t length,
                                     uint32_t timeout_ms)
{
    hss_spi_device_t device;
    hss_result_t result = hss_sensor_spi_get_device(&device);
    if (result != HSS_OK)
    {
        return result;
    }

    return hss_spi_device_transfer(&device, tx_data, rx_data, length, timeout_ms);
}
