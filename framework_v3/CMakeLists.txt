cmake_minimum_required(VERSION 3.20)

project(hss_framework_v3
        VERSION 0.2.0
        LANGUAGES C ASM)

set(HSS_FRAMEWORK_ROOT "${CMAKE_CURRENT_LIST_DIR}" CACHE INTERNAL "HSS framework root")
set(HSS_FRAMEWORK_VERSION "${PROJECT_VERSION}" CACHE INTERNAL "HSS framework version")

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
