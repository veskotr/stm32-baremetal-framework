cmake_minimum_required(VERSION 3.20)

project(hello_blink C ASM)

set(HSS_BOARD_PATHS "${CMAKE_CURRENT_LIST_DIR}/../boards" CACHE STRING "HSS board search paths" FORCE)

add_subdirectory("../.." hss_framework_build)

hss_add_firmware(hello_blink
        BOARD blue_pill_temp_transmitter
        src/main.c
)
