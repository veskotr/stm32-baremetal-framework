cmake_minimum_required(VERSION 3.20)

project(eeprom_persistence_demo C ASM)

set(HSS_BOARD_PATHS "${CMAKE_CURRENT_LIST_DIR}/../boards" CACHE STRING "HSS board search paths" FORCE)

add_subdirectory("../.." hss_framework_build)

hss_add_firmware(eeprom_persistence_demo
        BOARD blue_pill_temp_transmitter
        CONFIG hss.conf
        src/main.c
)
