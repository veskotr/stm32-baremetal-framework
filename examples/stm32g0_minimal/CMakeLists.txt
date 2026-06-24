cmake_minimum_required(VERSION 3.20)

project(stm32g0_minimal C ASM)

set(HSS_BOARD_PATHS "${CMAKE_CURRENT_LIST_DIR}/../boards" CACHE STRING "HSS board search paths" FORCE)

add_subdirectory("../.." hss_framework_build)

hss_add_firmware(stm32g0_minimal
        BOARD stm32g071kb_temp_transmitter
        src/main.c
)
