cmake_minimum_required(VERSION 3.20)

project(stm32g0_minimal C ASM)

add_subdirectory("../.." framework_v3_build)

hss_add_firmware(stm32g0_minimal
        BOARD stm32g071kb_temp_transmitter
        src/main.c
)
