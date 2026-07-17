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
option(HSS_ENABLE_TESTS "Enable HSS host test helpers and support targets" OFF)
option(HSS_BUILD_FRAMEWORK_TESTS "Build HSS framework self-tests when tests are enabled" ${PROJECT_IS_TOP_LEVEL})

include("${HSS_FRAMEWORK_ROOT}/cmake/hss_framework.cmake")

add_library(hss_framework_board INTERFACE)
add_library(hss_framework_stm32 INTERFACE)

add_subdirectory(common)
add_subdirectory(hal)
add_subdirectory(board)
add_subdirectory(drivers)
add_subdirectory(protocols)

if(HSS_ENABLE_TESTS)
    include("${HSS_FRAMEWORK_ROOT}/cmake/hss_testing.cmake")
    if(HSS_BUILD_FRAMEWORK_TESTS)
        add_subdirectory(tests)
    endif()
endif()
