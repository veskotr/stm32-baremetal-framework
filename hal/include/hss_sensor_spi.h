#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "hss_result.h"
#include "hss_spi.h"

bool hss_sensor_spi_is_available(void);
hss_result_t hss_sensor_spi_get_device(hss_spi_device_t *device);
hss_result_t hss_sensor_spi_write(const uint8_t *data, uint16_t length, uint32_t timeout_ms);
hss_result_t hss_sensor_spi_read(uint8_t *data, uint16_t length, uint32_t timeout_ms);
hss_result_t hss_sensor_spi_transfer(const uint8_t *tx_data,
                                     uint8_t *rx_data,
                                     uint16_t length,
                                     uint32_t timeout_ms);
