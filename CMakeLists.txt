cmake_minimum_required(VERSION 3.20)

project(stm32_demo C CXX ASM)

# === Framework ===
add_subdirectory(framework_v2)

hss_add_firmware(my_app
        BOARD bluepill_f103
        main.cpp
)