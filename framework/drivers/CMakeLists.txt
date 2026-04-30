cmake_minimum_required(VERSION 3.20)

# Build individual drivers
add_subdirectory(analog_devices_no_os)
add_subdirectory(max31865)

# Create umbrella target for all drivers
add_library(hss_drivers INTERFACE)

target_link_libraries(hss_drivers INTERFACE
        hss_adi_max31865
        hss_max31865_driver
)
