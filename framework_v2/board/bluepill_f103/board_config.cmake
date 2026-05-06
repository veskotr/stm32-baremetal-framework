# =========================
# Board identity
# =========================
set(BOARD_MCU_FAMILY stm32f1)
set(BOARD_MCU_DEFINE STM32F103xB)

# =========================
# Memory layout
# =========================
set(BOARD_FLASH_SIZE 128K)
set(BOARD_RAM_SIZE 20K)
set(BOARD_MIN_HEAP_SIZE 0x200)
set(BOARD_MIN_STACK_SIZE 0x400)
set(BOARD_FLASH_ORIGIN 0x08000000)
set(BOARD_RAM_ORIGIN 0x20000000)
# =========================
# Clock constants
# =========================
set(BOARD_HSE_VALUE 8000000U)
set(BOARD_HSI_VALUE 8000000U)

# =========================
# HAL / CMSIS required macros
# =========================
set(BOARD_HSE_STARTUP_TIMEOUT 100U)
set(BOARD_LSE_STARTUP_TIMEOUT 5000U)
set(BOARD_TICK_INT_PRIORITY 15U)
set(BOARD_VECT_TAB_OFFSET 0x00000000U)

# =========================
# OpenOCD metadata
# =========================
set(BOARD_OPENOCD_INTERFACE interface/stlink.cfg)
set(BOARD_OPENOCD_TARGET target/stm32f1x.cfg)

# =========================
# Single canonical macro list
# =========================
set(HSS_BOARD_COMPILE_DEFINITIONS
        USE_HAL_DRIVER
        ${BOARD_MCU_DEFINE}
        HSE_VALUE=${BOARD_HSE_VALUE}
        HSI_VALUE=${BOARD_HSI_VALUE}
        HSE_STARTUP_TIMEOUT=${BOARD_HSE_STARTUP_TIMEOUT}
        LSE_STARTUP_TIMEOUT=${BOARD_LSE_STARTUP_TIMEOUT}
        TICK_INT_PRIORITY=${BOARD_TICK_INT_PRIORITY}
        VECT_TAB_OFFSET=${BOARD_VECT_TAB_OFFSET}
)

# Optional compatibility alias for board CMakeLists
set(BOARD_COMPILE_DEFINITIONS ${HSS_BOARD_COMPILE_DEFINITIONS})