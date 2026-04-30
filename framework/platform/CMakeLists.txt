cmake_minimum_required(VERSION 3.20)

add_library(hss_platform STATIC
        src/uart.c
        src/gpio.c
        src/platform.c
        src/dma.c
        src/irq.c
        src/time.c
        src/timer.c
)

target_include_directories(hss_platform PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(hss_platform PUBLIC
        board
        hss_core
)