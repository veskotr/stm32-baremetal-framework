#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "hss_result.h"
#include "main.h"

#ifndef HAL_SPI_MODULE_ENABLED
typedef struct __SPI_HandleTypeDef SPI_HandleTypeDef;
#endif

typedef struct
{
    SPI_HandleTypeDef *spi;
    GPIO_TypeDef *cs_port;
    uint16_t cs_pin;
    bool cs_active_high;
} hss_spi_device_t;

hss_result_t hss_spi_write(SPI_HandleTypeDef *spi,
                           const uint8_t *data,
                           uint16_t length,
                           uint32_t timeout_ms);
hss_result_t hss_spi_read(SPI_HandleTypeDef *spi,
                          uint8_t *data,
                          uint16_t length,
                          uint32_t timeout_ms);
hss_result_t hss_spi_transfer(SPI_HandleTypeDef *spi,
                              const uint8_t *tx_data,
                              uint8_t *rx_data,
                              uint16_t length,
                              uint32_t timeout_ms);
hss_result_t hss_spi_device_select(const hss_spi_device_t *device);
hss_result_t hss_spi_device_deselect(const hss_spi_device_t *device);
hss_result_t hss_spi_device_write(const hss_spi_device_t *device,
                                  const uint8_t *data,
                                  uint16_t length,
                                  uint32_t timeout_ms);
hss_result_t hss_spi_device_read(const hss_spi_device_t *device,
                                 uint8_t *data,
                                 uint16_t length,
                                 uint32_t timeout_ms);
hss_result_t hss_spi_device_transfer(const hss_spi_device_t *device,
                                     const uint8_t *tx_data,
                                     uint8_t *rx_data,
                                     uint16_t length,
                                     uint32_t timeout_ms);
