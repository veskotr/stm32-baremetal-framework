add_library(hss_hal STATIC
        src/gpio.c
        src/exti.c
        src/irq.c
        src/spi.c
        src/timer.c
        src/time.c
        src/watchdog.c
        src/uart.c
)
set_target_properties(hss_hal PROPERTIES EXCLUDE_FROM_ALL TRUE)

target_include_directories(hss_hal PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(hss_hal PUBLIC
        hss_common
        hss_framework_stm32
)
