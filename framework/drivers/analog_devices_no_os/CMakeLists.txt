cmake_minimum_required(VERSION 3.20)

include(FetchContent)

set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

FetchContent_Declare(
    analog_devices_no_os
    GIT_REPOSITORY https://github.com/analogdevicesinc/no-OS
    GIT_TAG Latest
)
FetchContent_MakeAvailable(analog_devices_no_os)

add_library(hss_adi_max31865 STATIC)

target_sources(hss_adi_max31865 PRIVATE
    ${analog_devices_no_os_SOURCE_DIR}/drivers/temperature/max31865pmb1/max31865.c
)

target_include_directories(hss_adi_max31865 PUBLIC
    ${analog_devices_no_os_SOURCE_DIR}/drivers/temperature/max31865pmb1
    ${analog_devices_no_os_SOURCE_DIR}/include
)

target_link_libraries(hss_adi_max31865 PUBLIC
    hss_platform
)

target_compile_options(hss_adi_max31865 PRIVATE
    -ffunction-sections
    -fdata-sections
)
