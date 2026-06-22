#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "hss_result.h"
#include "hss_spi.h"

#define HSS_MAX31865_DEFAULT_TIMEOUT_MS 100U
#define HSS_MAX31865_DEFAULT_RTD_RC_DELAY_US 2000U

typedef enum
{
    HSS_MAX31865_FAULT_HIGH_THRESHOLD = 0x80U,
    HSS_MAX31865_FAULT_LOW_THRESHOLD = 0x40U,
    HSS_MAX31865_FAULT_REFIN_LOW = 0x20U,
    HSS_MAX31865_FAULT_REFIN_HIGH = 0x10U,
    HSS_MAX31865_FAULT_RTDIN_LOW = 0x08U,
    HSS_MAX31865_FAULT_OVERVOLT_UNDERVOLT = 0x04U,
} hss_max31865_fault_t;

typedef struct
{
    bool three_wire;
    bool filter_50hz;
    uint32_t timeout_ms;
    uint32_t rtd_rc_delay_us;
} hss_max31865_config_t;

typedef struct
{
    hss_spi_device_t spi_device;
    uint32_t timeout_ms;
    uint32_t rtd_rc_delay_us;
    bool filter_50hz;
    bool three_wire;
    bool initialized;
} hss_max31865_t;

hss_max31865_config_t hss_max31865_default_config(void);
hss_result_t hss_max31865_init(hss_max31865_t *device,
                               const hss_max31865_config_t *config);
hss_result_t hss_max31865_init_with_spi(hss_max31865_t *device,
                                        const hss_spi_device_t *spi_device,
                                        const hss_max31865_config_t *config);
hss_result_t hss_max31865_read_register(hss_max31865_t *device,
                                        uint8_t reg_addr,
                                        uint8_t *reg_data);
hss_result_t hss_max31865_write_register(hss_max31865_t *device,
                                         uint8_t reg_addr,
                                         uint8_t reg_data);
hss_result_t hss_max31865_clear_faults(hss_max31865_t *device);
hss_result_t hss_max31865_set_bias(hss_max31865_t *device, bool enabled);
hss_result_t hss_max31865_set_auto_convert(hss_max31865_t *device, bool enabled);
hss_result_t hss_max31865_set_filter_50hz(hss_max31865_t *device, bool enabled);
hss_result_t hss_max31865_set_three_wire(hss_max31865_t *device, bool enabled);
hss_result_t hss_max31865_set_thresholds(hss_max31865_t *device,
                                         uint16_t low_threshold,
                                         uint16_t high_threshold);
hss_result_t hss_max31865_get_thresholds(hss_max31865_t *device,
                                         uint16_t *low_threshold,
                                         uint16_t *high_threshold);
hss_result_t hss_max31865_read_fault_status(hss_max31865_t *device,
                                            uint8_t *fault_status);
hss_result_t hss_max31865_read_rtd_raw(hss_max31865_t *device,
                                       uint16_t *rtd_raw);
hss_result_t hss_max31865_raw_to_resistance(uint16_t rtd_raw,
                                            float reference_resistor_ohms,
                                            float *resistance_ohms);
