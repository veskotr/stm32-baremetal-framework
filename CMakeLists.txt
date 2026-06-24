cmake_minimum_required(VERSION 3.20)

file(READ "${CMAKE_CURRENT_LIST_DIR}/version.txt" HSS_FRAMEWORK_VERSION_FROM_FILE)
string(STRIP "${HSS_FRAMEWORK_VERSION_FROM_FILE}" HSS_FRAMEWORK_VERSION_FROM_FILE)
if(NOT HSS_FRAMEWORK_VERSION_FROM_FILE MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+(\\.[0-9]+)?$")
    message(FATAL_ERROR "version.txt must contain a numeric CMake project version such as 0.3.0")
endif()

project(hss_stm32_framework
        VERSION "${HSS_FRAMEWORK_VERSION_FROM_FILE}"
        LANGUAGES C ASM)

set(HSS_FRAMEWORK_ROOT "${CMAKE_CURRENT_LIST_DIR}")
set(HSS_FRAMEWORK_VERSION "${PROJECT_VERSION}")
set(HSS_FRAMEWORK_ROOT "${HSS_FRAMEWORK_ROOT}" CACHE INTERNAL "HSS framework root" FORCE)
set(HSS_FRAMEWORK_VERSION "${HSS_FRAMEWORK_VERSION}" CACHE INTERNAL "HSS framework version" FORCE)

option(HSS_ENABLE_MODBUS_DEBUG "Build Modbus debug counters" OFF)

include("${HSS_FRAMEWORK_ROOT}/cmake/hss_framework.cmake")

add_library(hss_framework_board INTERFACE)

add_subdirectory(common)
add_subdirectory(hal)
add_subdirectory(drivers)
add_subdirectory(protocols)

add_library(hss_framework INTERFACE)
target_compile_features(hss_framework INTERFACE c_std_11)
target_link_libraries(hss_framework INTERFACE
        hss_common
        hss_hal
        hss_drivers
        hss_protocols
)
