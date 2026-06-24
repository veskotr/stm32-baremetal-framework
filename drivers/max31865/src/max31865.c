#include "hss_max31865.h"

#include <stddef.h>

#include "hss_sensor_spi.h"
#include "hss_time.h"

#define MAX31865_READ_MASK 0x7FU
#define MAX31865_WRITE_MASK 0x80U

#define MAX31865_CONFIG_REG 0x00U
#define MAX31865_RTDMSB_REG 0x01U
#define MAX31865_RTDLSB_REG 0x02U
#define MAX31865_HFAULTMSB_REG 0x03U
#define MAX31865_HFAULTLSB_REG 0x04U
#define MAX31865_LFAULTMSB_REG 0x05U
#define MAX31865_LFAULTLSB_REG 0x06U
#define MAX31865_FAULTSTAT_REG 0x07U

#define MAX31865_CONFIG_BIAS 0x80U
#define MAX31865_CONFIG_MODEAUTO 0x40U
#define MAX31865_CONFIG_1SHOT 0x20U
#define MAX31865_CONFIG_3WIRE 0x10U
#define MAX31865_CONFIG_FAULT_DETECT_MASK 0x0CU
#define MAX31865_CONFIG_CLRFAULT 0x02U
#define MAX31865_CONFIG_FILT50HZ 0x01U

#define MAX31865_RTD_RAW_SHIFT 1U
#define MAX31865_RTD_FULL_SCALE 32768.0f

static bool hss_max31865_is_readable_register(uint8_t reg_addr)
{
    reg_addr &= MAX31865_READ_MASK;
    return reg_addr >= MAX31865_CONFIG_REG && reg_addr <= MAX31865_FAULTSTAT_REG;
}

static bool hss_max31865_is_writable_register(uint8_t reg_addr)
{
    reg_addr &= MAX31865_READ_MASK;
    return reg_addr == MAX31865_CONFIG_REG ||
           (reg_addr >= MAX31865_HFAULTMSB_REG && reg_addr <= MAX31865_LFAULTLSB_REG);
}

static hss_result_t hss_max31865_validate(hss_max31865_t *device)
{
    if (device == NULL)
    {
        return HSS_INVALID_ARGUMENT;
    }

    if (!device->initialized)
    {
        return HSS_NOT_READY;
    }

    return HSS_OK;
}

static hss_result_t hss_max31865_update_register(hss_max31865_t *device,
                                                 uint8_t reg_addr,
                                                 uint8_t set_mask,
                                                 uint8_t clear_mask)
{
    uint8_t value;
    hss_result_t result = hss_max31865_read_register(device, reg_addr, &value);
    if (result != HSS_OK)
    {
        return result;
    }

    value &= (uint8_t)~clear_mask;
    value |= set_mask;
    return hss_max31865_write_register(device, reg_addr, value);
}

static uint32_t hss_max31865_conversion_delay_ms(const hss_max31865_t *device)
{
    const uint32_t conversion_us = device->filter_50hz ? 62500U : 52000U;
    const uint32_t delay_us = conversion_us + device->rtd_rc_delay_us;
    return (delay_us + 999U) / 1000U;
}

hss_max31865_config_t hss_max31865_default_config(void)
{
    hss_max31865_config_t config = {
        .three_wire = false,
        .filter_50hz = false,
        .timeout_ms = HSS_MAX31865_DEFAULT_TIMEOUT_MS,
        .rtd_rc_delay_us = HSS_MAX31865_DEFAULT_RTD_RC_DELAY_US,
    };

    return config;
}

hss_result_t hss_max31865_init(hss_max31865_t *device,
                               const hss_max31865_config_t *config)
{
    hss_spi_device_t spi_device;
    hss_result_t result = hss_sensor_spi_get_device(&spi_device);
    if (result != HSS_OK)
    {
        return result;
    }

    return hss_max31865_init_with_spi(device, &spi_device, config);
}

hss_result_t hss_max31865_init_with_spi(hss_max31865_t *device,
                                        const hss_spi_device_t *spi_device,
                                        const hss_max31865_config_t *config)
{
    if (device == NULL || spi_device == NULL || spi_device->spi == NULL)
    {
        return HSS_INVALID_ARGUMENT;
    }

    hss_max31865_config_t resolved_config = hss_max31865_default_config();
    if (config != NULL)
    {
        resolved_config = *config;
    }

    if (resolved_config.timeout_ms == 0U)
    {
        resolved_config.timeout_ms = HSS_MAX31865_DEFAULT_TIMEOUT_MS;
    }
    if (resolved_config.rtd_rc_delay_us == 0U)
    {
        resolved_config.rtd_rc_delay_us = HSS_MAX31865_DEFAULT_RTD_RC_DELAY_US;
    }

    device->spi_device = *spi_device;
    device->timeout_ms = resolved_config.timeout_ms;
    device->rtd_rc_delay_us = resolved_config.rtd_rc_delay_us;
    device->filter_50hz = resolved_config.filter_50hz;
    device->three_wire = resolved_config.three_wire;
    device->initialized = true;

    uint8_t config_value = 0U;
    if (device->three_wire)
    {
        config_value |= MAX31865_CONFIG_3WIRE;
    }
    if (device->filter_50hz)
    {
        config_value |= MAX31865_CONFIG_FILT50HZ;
    }

    return hss_max31865_write_register(device, MAX31865_CONFIG_REG, config_value);
}

hss_result_t hss_max31865_read_register(hss_max31865_t *device,
                                        uint8_t reg_addr,
                                        uint8_t *reg_data)
{
    hss_result_t result = hss_max31865_validate(device);
    if (result != HSS_OK)
    {
        return result;
    }
    if (reg_data == NULL || !hss_max31865_is_readable_register(reg_addr))
    {
        return HSS_INVALID_ARGUMENT;
    }

    uint8_t tx_data[2] = { (uint8_t)(reg_addr & MAX31865_READ_MASK), 0U };
    uint8_t rx_data[2] = { 0U, 0U };
    result = hss_spi_device_transfer(&device->spi_device,
                                     tx_data,
                                     rx_data,
                                     (uint16_t)sizeof(tx_data),
                                     device->timeout_ms);
    if (result != HSS_OK)
    {
        return result;
    }

    *reg_data = rx_data[1];
    return HSS_OK;
}

hss_result_t hss_max31865_write_register(hss_max31865_t *device,
                                         uint8_t reg_addr,
                                         uint8_t reg_data)
{
    hss_result_t result = hss_max31865_validate(device);
    if (result != HSS_OK)
    {
        return result;
    }
    if (!hss_max31865_is_writable_register(reg_addr))
    {
        return HSS_INVALID_ARGUMENT;
    }

    uint8_t tx_data[2] = {
        (uint8_t)((reg_addr & MAX31865_READ_MASK) | MAX31865_WRITE_MASK),
        reg_data,
    };

    return hss_spi_device_write(&device->spi_device,
                                tx_data,
                                (uint16_t)sizeof(tx_data),
                                device->timeout_ms);
}

hss_result_t hss_max31865_clear_faults(hss_max31865_t *device)
{
    return hss_max31865_update_register(device,
                                        MAX31865_CONFIG_REG,
                                        MAX31865_CONFIG_CLRFAULT,
                                        (uint8_t)(MAX31865_CONFIG_FAULT_DETECT_MASK |
                                                  MAX31865_CONFIG_1SHOT));
}

hss_result_t hss_max31865_set_bias(hss_max31865_t *device, bool enabled)
{
    return hss_max31865_update_register(device,
                                        MAX31865_CONFIG_REG,
                                        enabled ? MAX31865_CONFIG_BIAS : 0U,
                                        enabled ? 0U : MAX31865_CONFIG_BIAS);
}

hss_result_t hss_max31865_set_auto_convert(hss_max31865_t *device, bool enabled)
{
    return hss_max31865_update_register(device,
                                        MAX31865_CONFIG_REG,
                                        enabled ? MAX31865_CONFIG_MODEAUTO : 0U,
                                        enabled ? 0U : MAX31865_CONFIG_MODEAUTO);
}

hss_result_t hss_max31865_set_filter_50hz(hss_max31865_t *device, bool enabled)
{
    hss_result_t result = hss_max31865_update_register(device,
                                                       MAX31865_CONFIG_REG,
                                                       enabled ? MAX31865_CONFIG_FILT50HZ : 0U,
                                                       enabled ? 0U : MAX31865_CONFIG_FILT50HZ);
    if (result == HSS_OK)
    {
        device->filter_50hz = enabled;
    }

    return result;
}

hss_result_t hss_max31865_set_three_wire(hss_max31865_t *device, bool enabled)
{
    hss_result_t result = hss_max31865_update_register(device,
                                                       MAX31865_CONFIG_REG,
                                                       enabled ? MAX31865_CONFIG_3WIRE : 0U,
                                                       enabled ? 0U : MAX31865_CONFIG_3WIRE);
    if (result == HSS_OK)
    {
        device->three_wire = enabled;
    }

    return result;
}

hss_result_t hss_max31865_set_thresholds(hss_max31865_t *device,
                                         uint16_t low_threshold,
                                         uint16_t high_threshold)
{
    hss_result_t result = hss_max31865_write_register(device,
                                                      MAX31865_HFAULTMSB_REG,
                                                      (uint8_t)(high_threshold >> 8));
    if (result != HSS_OK)
    {
        return result;
    }

    result = hss_max31865_write_register(device,
                                         MAX31865_HFAULTLSB_REG,
                                         (uint8_t)(high_threshold & 0xFFU));
    if (result != HSS_OK)
    {
        return result;
    }

    result = hss_max31865_write_register(device,
                                         MAX31865_LFAULTMSB_REG,
                                         (uint8_t)(low_threshold >> 8));
    if (result != HSS_OK)
    {
        return result;
    }

    return hss_max31865_write_register(device,
                                       MAX31865_LFAULTLSB_REG,
                                       (uint8_t)(low_threshold & 0xFFU));
}

hss_result_t hss_max31865_get_thresholds(hss_max31865_t *device,
                                         uint16_t *low_threshold,
                                         uint16_t *high_threshold)
{
    if (low_threshold == NULL || high_threshold == NULL)
    {
        return HSS_INVALID_ARGUMENT;
    }

    uint8_t high_msb;
    uint8_t high_lsb;
    uint8_t low_msb;
    uint8_t low_lsb;

    hss_result_t result = hss_max31865_read_register(device, MAX31865_HFAULTMSB_REG, &high_msb);
    if (result != HSS_OK)
    {
        return result;
    }
    result = hss_max31865_read_register(device, MAX31865_HFAULTLSB_REG, &high_lsb);
    if (result != HSS_OK)
    {
        return result;
    }
    result = hss_max31865_read_register(device, MAX31865_LFAULTMSB_REG, &low_msb);
    if (result != HSS_OK)
    {
        return result;
    }
    result = hss_max31865_read_register(device, MAX31865_LFAULTLSB_REG, &low_lsb);
    if (result != HSS_OK)
    {
        return result;
    }

    *high_threshold = (uint16_t)(((uint16_t)high_msb << 8) | high_lsb);
    *low_threshold = (uint16_t)(((uint16_t)low_msb << 8) | low_lsb);
    return HSS_OK;
}

hss_result_t hss_max31865_read_fault_status(hss_max31865_t *device,
                                            uint8_t *fault_status)
{
    return hss_max31865_read_register(device, MAX31865_FAULTSTAT_REG, fault_status);
}

hss_result_t hss_max31865_read_rtd_raw(hss_max31865_t *device,
                                       uint16_t *rtd_raw)
{
    if (rtd_raw == NULL)
    {
        return HSS_INVALID_ARGUMENT;
    }

    hss_result_t result = hss_max31865_clear_faults(device);
    if (result != HSS_OK)
    {
        return result;
    }

    result = hss_max31865_set_bias(device, true);
    if (result != HSS_OK)
    {
        return result;
    }

    result = hss_max31865_update_register(device,
                                          MAX31865_CONFIG_REG,
                                          MAX31865_CONFIG_1SHOT,
                                          0U);
    if (result != HSS_OK)
    {
        (void)hss_max31865_set_bias(device, false);
        return result;
    }

    hss_delay_ms(hss_max31865_conversion_delay_ms(device));

    uint8_t msb;
    uint8_t lsb;
    result = hss_max31865_read_register(device, MAX31865_RTDMSB_REG, &msb);
    if (result != HSS_OK)
    {
        (void)hss_max31865_set_bias(device, false);
        return result;
    }

    result = hss_max31865_read_register(device, MAX31865_RTDLSB_REG, &lsb);
    if (result != HSS_OK)
    {
        (void)hss_max31865_set_bias(device, false);
        return result;
    }

    *rtd_raw = (uint16_t)((((uint16_t)msb << 8) | lsb) >> MAX31865_RTD_RAW_SHIFT);
    return hss_max31865_set_bias(device, false);
}

hss_result_t hss_max31865_raw_to_resistance(uint16_t rtd_raw,
                                            float reference_resistor_ohms,
                                            float *resistance_ohms)
{
    if (reference_resistor_ohms <= 0.0f || resistance_ohms == NULL)
    {
        return HSS_INVALID_ARGUMENT;
    }

    *resistance_ohms = ((float)rtd_raw * reference_resistor_ohms) / MAX31865_RTD_FULL_SCALE;
    return HSS_OK;
}
