cmake_minimum_required(VERSION 3.20)

add_library(hss_mcu_stm32f103xb INTERFACE)

target_link_libraries(hss_mcu_stm32f103xb INTERFACE
        hss_mcu_family
)

set_target_properties(hss_mcu_stm32f103xb PROPERTIES
        HSS_MCU_FAMILY stm32f1
)
