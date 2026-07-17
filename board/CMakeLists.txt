add_library(hss_board STATIC
        src/console.c
        src/console_stdio.c
        src/eeprom.c
        src/modbus_timer.c
        src/modbus_uart.c
        src/platform.c
        src/sensor_spi.c
        src/status_led.c
        src/watchdog.c
)
set_target_properties(hss_board PROPERTIES EXCLUDE_FROM_ALL TRUE)

target_include_directories(hss_board PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(hss_board PUBLIC
        hss_hal
        hss_framework_board
)

if(HSS_ENABLE_MODBUS_DEBUG)
    target_compile_definitions(hss_board PUBLIC HSS_ENABLE_MODBUS_DEBUG=1)
endif()
