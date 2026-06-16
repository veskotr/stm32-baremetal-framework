cmake_minimum_required(VERSION 3.20)

project(blue_pill_minimal C ASM)

set(HSS_ENABLE_FREEMODBUS ON CACHE BOOL "Enable FreeModbus for the Blue Pill example" FORCE)

add_subdirectory("../.." framework_v3_build)

hss_add_firmware(blue_pill_minimal
        BOARD blue_pill_temp_transmitter
        src/main.c
)
