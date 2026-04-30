cmake_minimum_required(VERSION 3.20)

enable_language(ASM)

add_library(hss_board_blue_pill_temp_transmitter STATIC)

# === Board Configuration ===
set(BOARD_UART_COUNT 1)
set(BOARD_SPI_COUNT 1)
set(BOARD_DMA 1)
set(BOARD_UART1 1)
set(BOARD_SPI1 1)
set(BOARD_TIM2 1)
set(BOARD_TIMER_COUNT 4)   # Must be max(timer_id) + 1. HSS_TIMER2=1, so size must be 2.

# Auto-generate board_config.h from template
configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/board_config.h.in
        ${CMAKE_CURRENT_BINARY_DIR}/board_config.h
        @ONLY
)

# === CPU flags ===
set(MCU_CPU_FLAGS
        -mcpu=cortex-m3
        -mthumb
)

set_target_properties(hss_board_blue_pill_temp_transmitter PROPERTIES
        OPENOCD_INTERFACE "interface/stlink.cfg"
        OPENOCD_TARGET "target/stm32f1x.cfg"
)

set_source_files_properties(startup_stm32f103xb.s PROPERTIES
        LANGUAGE ASM
)

target_compile_options(hss_board_blue_pill_temp_transmitter PUBLIC
        ${MCU_CPU_FLAGS}
        -ffunction-sections
        -fdata-sections
)

target_compile_definitions(hss_board_blue_pill_temp_transmitter PUBLIC
        STM32F103xB
        USE_HAL_DRIVER
)

# === Include directories ===
# Make generated board_config.h available to the board library itself and consumers
target_include_directories(hss_board_blue_pill_temp_transmitter PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}
        Core/Inc
        Drivers/STM32F1xx_HAL_Driver/Inc
        Drivers/STM32F1xx_HAL_Driver/Inc/Legacy
        Drivers/CMSIS/Device/ST/STM32F1xx/Include
        Drivers/CMSIS/Include
)

# === Source files ===
file(GLOB_RECURSE HAL_SOURCES
        Drivers/STM32F1xx_HAL_Driver/Src/*.c
)

file(GLOB_RECURSE CORE_SOURCES
        Core/Src/*.c
)

target_sources(hss_board_blue_pill_temp_transmitter PRIVATE
        ${HAL_SOURCES}
        ${CORE_SOURCES}
        startup_stm32f103xb.s
)

# These overrides must end up in the final ELF even when the board is packaged as a static archive.
set_target_properties(hss_board_blue_pill_temp_transmitter PROPERTIES
        HSS_ALWAYS_LINK_SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/Core/Src/stm32f1xx_it.c;${CMAKE_CURRENT_LIST_DIR}/Core/Src/stm32f1xx_hal_msp.c;${CMAKE_CURRENT_LIST_DIR}/Core/Src/syscalls.c;${CMAKE_CURRENT_LIST_DIR}/Core/Src/sysmem.c"
)

# === Linker script ===
set(MCU_LINKER_SCRIPT
        ${CMAKE_CURRENT_LIST_DIR}/STM32F103XX_FLASH.ld
)

set_target_properties(hss_board_blue_pill_temp_transmitter PROPERTIES
        HSS_LINKER_SCRIPT ${MCU_LINKER_SCRIPT}
)

target_link_options(hss_board_blue_pill_temp_transmitter PUBLIC
        ${MCU_CPU_FLAGS}
        -Wl,--gc-sections
)

target_link_libraries(hss_board_blue_pill_temp_transmitter PUBLIC
        hss_core
)
