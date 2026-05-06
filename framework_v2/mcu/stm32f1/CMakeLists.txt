cmake_minimum_required(VERSION 3.20)

if (NOT DEFINED HSS_ACTIVE_BOARD_DEFINITIONS)
    message(FATAL_ERROR "HSS_ACTIVE_BOARD_DEFINITIONS is not set. Call hss_select_board() before adding MCU.")
endif ()

include(FetchContent)

FetchContent_Declare(
        stm32cube_f1
        GIT_REPOSITORY https://github.com/STMicroelectronics/STM32CubeF1.git
        GIT_TAG v1.8.5
)

FetchContent_MakeAvailable(stm32cube_f1)

set(MCU_F1_FLAGS
        -mcpu=cortex-m3
        -mthumb
        -ffreestanding
        --specs=nosys.specs
)

# =========================
# HAL CONFIGURATION
# =========================

add_library(stm32f1_hal_config INTERFACE)

target_include_directories(stm32f1_hal_config INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_definitions(stm32f1_hal_config INTERFACE
        USE_HAL_DRIVER
        ${HSS_ACTIVE_BOARD_DEFINITIONS}
)

# =========================
# CMSIS DEVICE (REQUIRED)
# =========================
add_library(cmsis_device_f1 STATIC
        ${stm32cube_f1_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F1xx/Source/Templates/system_stm32f1xx.c
)

target_include_directories(cmsis_device_f1 PUBLIC
        ${stm32cube_f1_SOURCE_DIR}/Drivers/CMSIS/Core/Include
        ${stm32cube_f1_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F1xx/Include
        ${stm32cube_f1_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Inc
)

target_link_libraries(cmsis_device_f1 PUBLIC stm32f1_hal_config)

target_compile_definitions(cmsis_device_f1 PUBLIC
        ${HSS_ACTIVE_BOARD_DEFINITIONS}
)

target_compile_options(cmsis_device_f1 PUBLIC ${MCU_F1_FLAGS})

# =========================
# HAL (FULL BASELINE SET)
# =========================
add_library(stm32f1_hal STATIC
        ${stm32cube_f1_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal.c
        ${stm32cube_f1_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc.c
        ${stm32cube_f1_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_gpio.c
        ${stm32cube_f1_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_cortex.c
)

target_include_directories(stm32f1_hal PUBLIC
        ${stm32cube_f1_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Inc
        ${stm32cube_f1_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F1xx/Include
        ${stm32cube_f1_SOURCE_DIR}/Drivers/CMSIS/Core/Include
)

target_link_libraries(stm32f1_hal PUBLIC stm32f1_hal_config)

target_compile_definitions(stm32f1_hal PUBLIC
        USE_HAL_DRIVER
        ${HSS_ACTIVE_BOARD_DEFINITIONS}
)

target_compile_options(stm32f1_hal PUBLIC ${MCU_F1_FLAGS})

# =========================
# YOUR MCU LAYER
# =========================
add_library(hss_mcu_stm32f1 STATIC
        src/mcu_init.c
        src/stm32f1xx_it.c
        src/syscalls.c
)

target_include_directories(hss_mcu_stm32f1 PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(hss_mcu_stm32f1 PUBLIC stm32f1_hal cmsis_device_f1)

target_compile_options(hss_mcu_stm32f1 PUBLIC ${MCU_F1_FLAGS})