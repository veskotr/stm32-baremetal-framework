# Drivers

Reusable driver integrations live here. Each driver is an opt-in, named CMake
target; applications link it directly or enable it with the corresponding
`HSS_ENABLE_*` config value.

Current drivers:

- `hss_max31865`: MAX31865 RTD converter helper backed by an explicit
  `hss_spi_device_t`.

The MAX31865 implementation is shaped after the Analog Devices no-OS driver used by the v1 framework, but it is native to v3 and does not require the full no-OS platform layer.
