cmake_minimum_required(VERSION 3.20)

project(freemodbus C)

include(FetchContent)

FetchContent_Declare(
        free_modbus
        GIT_REPOSITORY https://github.com/cwalter-at/freemodbus
        GIT_TAG 1.6.0
)

FetchContent_MakeAvailable(free_modbus)

add_library(${PROJECT_NAME} STATIC
        src/portserial.c
        src/porttimer.c
        src/portevent.c
        src/portcritical.c)

target_sources(${PROJECT_NAME} PRIVATE
        ${free_modbus_SOURCE_DIR}/modbus/mb.c
        ${free_modbus_SOURCE_DIR}/modbus/ascii/mbascii.c
        ${free_modbus_SOURCE_DIR}/modbus/rtu/mbcrc.c
        ${free_modbus_SOURCE_DIR}/modbus/rtu/mbrtu.c
        ${free_modbus_SOURCE_DIR}/modbus/functions/mbfunccoils.c
        ${free_modbus_SOURCE_DIR}/modbus/functions/mbfuncdisc.c
        ${free_modbus_SOURCE_DIR}/modbus/functions/mbfuncholding.c
        ${free_modbus_SOURCE_DIR}/modbus/functions/mbfuncinput.c
        ${free_modbus_SOURCE_DIR}/modbus/functions/mbfuncother.c
        ${free_modbus_SOURCE_DIR}/modbus/functions/mbutils.c
)

target_include_directories(${PROJECT_NAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${free_modbus_SOURCE_DIR}/modbus/include
        ${free_modbus_SOURCE_DIR}/modbus/rtu
        ${free_modbus_SOURCE_DIR}/modbus/ascii
)

target_link_libraries(${PROJECT_NAME} PUBLIC
        hss_platform
)