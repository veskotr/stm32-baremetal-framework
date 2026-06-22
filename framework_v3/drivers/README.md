# Drivers

Reusable driver integrations live here.

The aggregate `hss_drivers` CMake target is linked by `hss_framework`, so applications that use `hss_add_firmware()` can include driver headers without extra target wiring.

Current drivers:

- `hss_max31865`: MAX31865 RTD converter helper backed by `hss_spi_device_t` and the `BOARD_ROLE_SENSOR_SPI` / optional `BOARD_ROLE_SENSOR_CS` roles.

The MAX31865 implementation is shaped after the Analog Devices no-OS driver used by the v1 framework, but it is native to v3 and does not require the full no-OS platform layer.
