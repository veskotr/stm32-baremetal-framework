cmake_minimum_required(VERSION 3.20)

project(example_firmware C ASM)

set(HSS_ENABLE_FREEMODBUS ON CACHE BOOL "Enable FreeModbus for the Blue Pill Modbus slave example" FORCE)

add_subdirectory(framework_v3)

hss_add_firmware(example_firmware
        BOARD blue_pill_temp_transmitter
        GENERATE_VSCODE
        src/main.c
)
