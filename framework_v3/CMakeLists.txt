cmake_minimum_required(VERSION 3.20)

project(hss_framework_v3 LANGUAGES C ASM)

set(HSS_FRAMEWORK_ROOT "${CMAKE_CURRENT_LIST_DIR}" CACHE INTERNAL "HSS framework root")

include("${HSS_FRAMEWORK_ROOT}/cmake/hss_framework.cmake")

add_library(hss_framework_board INTERFACE)

add_subdirectory(hal)

add_library(hss_framework INTERFACE)
target_compile_features(hss_framework INTERFACE c_std_11)
target_link_libraries(hss_framework INTERFACE
        hss_hal
)
