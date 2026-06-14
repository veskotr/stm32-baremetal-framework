add_library(hss_hal STATIC
        src/gpio.c
        src/irq.c
        src/platform.c
        src/time.c
)

target_include_directories(hss_hal PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(hss_hal PUBLIC
        hss_framework_board
)

