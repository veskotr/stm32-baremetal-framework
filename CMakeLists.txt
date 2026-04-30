cmake_minimum_required(VERSION 3.20)

project(stm32_demo C ASM)

# === Framework ===
add_subdirectory(framework)

# === App configuration ===
set(HSS_BOARD "blue_pill_temp_transmitter" CACHE STRING "Board name under framework/board")
hss_select_board(${HSS_BOARD})

# === Firmware target ===
hss_add_firmware(demo_firmware
    main.c
)

hss_add_flash_target(demo_firmware)

hss_generate_debug_script(demo_firmware)