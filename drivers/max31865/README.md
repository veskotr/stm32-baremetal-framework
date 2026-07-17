# MAX31865 Driver

`hss_max31865` provides a small v3-native driver for the MAX31865 RTD converter.

It follows the register behavior of the Analog Devices no-OS MAX31865 driver used by the v1 framework, but uses HSS HAL helpers directly:

- `hss_spi_device_t`
- `hss_delay_ms()`
- `hss_result_t`

The driver requires an explicit SPI device descriptor. A product that uses the
generic sensor-SPI board role can obtain that descriptor through `hss_board`;
other products can construct it from their own SPI handle and chip-select GPIO.

Minimal use:

```c
#include "hss_board.h"
#include "hss_max31865.h"

hss_max31865_t rtd;
hss_max31865_config_t config = hss_max31865_default_config();
hss_spi_device_t spi_device;
config.three_wire = true;
config.filter_50hz = true;

if (hss_sensor_spi_get_device(&spi_device) == HSS_OK &&
    hss_max31865_init_with_spi(&rtd, &spi_device, &config) == HSS_OK) {
    uint16_t raw;
    float resistance;

    if (hss_max31865_read_rtd_raw(&rtd, &raw) == HSS_OK &&
        hss_max31865_raw_to_resistance(raw, 430.0f, &resistance) == HSS_OK) {
        /* Use resistance. */
    }
}
```

Temperature conversion is intentionally left to application code for now because the RTD nominal resistance and coefficient model are product-specific.
