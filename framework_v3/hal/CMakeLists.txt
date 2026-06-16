add_library(hss_hal STATIC
        src/gpio.c
        src/console.c
        src/console_stdio.c
        src/irq.c
        src/modbus_timer.c
        src/modbus_uart.c
        src/platform.c
        src/sensor_spi.c
        src/spi.c
        src/status_led.c
        src/timer.c
        src/time.c
        src/uart.c
)

target_include_directories(hss_hal PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(hss_hal PUBLIC
        hss_common
        hss_framework_board
)
