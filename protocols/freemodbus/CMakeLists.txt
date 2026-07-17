include(FetchContent)

set(HSS_FREEMODBUS_GIT_REPOSITORY "https://github.com/cwalter-at/freemodbus"
        CACHE STRING "FreeModbus upstream repository")
set(HSS_FREEMODBUS_GIT_TAG "1.6.0"
        CACHE STRING "FreeModbus upstream git tag")

FetchContent_Declare(
        hss_freemodbus_upstream
        GIT_REPOSITORY ${HSS_FREEMODBUS_GIT_REPOSITORY}
        GIT_TAG ${HSS_FREEMODBUS_GIT_TAG}
)

FetchContent_MakeAvailable(hss_freemodbus_upstream)

add_library(hss_freemodbus STATIC
        src/modbus.c
        src/modbus_codec.c
        src/portcritical.c
        src/portevent.c
        src/portserial.c
        src/porttimer.c
)
set_target_properties(hss_freemodbus PROPERTIES EXCLUDE_FROM_ALL TRUE)

target_sources(hss_freemodbus PRIVATE
        ${hss_freemodbus_upstream_SOURCE_DIR}/modbus/mb.c
        ${hss_freemodbus_upstream_SOURCE_DIR}/modbus/ascii/mbascii.c
        ${hss_freemodbus_upstream_SOURCE_DIR}/modbus/rtu/mbcrc.c
        ${hss_freemodbus_upstream_SOURCE_DIR}/modbus/rtu/mbrtu.c
        ${hss_freemodbus_upstream_SOURCE_DIR}/modbus/functions/mbfunccoils.c
        ${hss_freemodbus_upstream_SOURCE_DIR}/modbus/functions/mbfuncdisc.c
        ${hss_freemodbus_upstream_SOURCE_DIR}/modbus/functions/mbfuncholding.c
        ${hss_freemodbus_upstream_SOURCE_DIR}/modbus/functions/mbfuncinput.c
        ${hss_freemodbus_upstream_SOURCE_DIR}/modbus/functions/mbfuncother.c
        ${hss_freemodbus_upstream_SOURCE_DIR}/modbus/functions/mbutils.c
)

target_include_directories(hss_freemodbus PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${hss_freemodbus_upstream_SOURCE_DIR}/modbus/include
        ${hss_freemodbus_upstream_SOURCE_DIR}/modbus/rtu
        ${hss_freemodbus_upstream_SOURCE_DIR}/modbus/ascii
)

target_link_libraries(hss_freemodbus PUBLIC
        hss_common
        hss_board
)

if(HSS_ENABLE_MODBUS_DEBUG)
    target_compile_definitions(hss_freemodbus PUBLIC HSS_ENABLE_MODBUS_DEBUG=1)
endif()
