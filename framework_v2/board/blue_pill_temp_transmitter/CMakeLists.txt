cmake_minimum_required(VERSION 3.20)

add_library(hss_board_blue_pill_temp_transmitter INTERFACE)

target_link_libraries(hss_board_blue_pill_temp_transmitter INTERFACE
        hss_platform
)

set_target_properties(hss_board_blue_pill_temp_transmitter PROPERTIES
        HSS_MCU stm32f103xb
)
