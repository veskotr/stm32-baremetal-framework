cmake_minimum_required(VERSION 3.20)

include(${CMAKE_CURRENT_LIST_DIR}/board_config.cmake)

add_library(hss_board_bluepill_f103 STATIC
        src/board.c
        src/startup_stm32f103c8tx.s
)

target_link_libraries(hss_board_bluepill_f103 PUBLIC hss_mcu_stm32f1)

target_include_directories(hss_board_bluepill_f103 PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_definitions(hss_board_bluepill_f103 PUBLIC
        ${BOARD_COMPILE_DEFINITIONS}
)

target_compile_options(hss_board_bluepill_f103 PUBLIC
        -mcpu=cortex-m3
        -mthumb
)

set(FLASH_SIZE ${BOARD_FLASH_SIZE})
set(RAM_SIZE ${BOARD_RAM_SIZE})
set(MIN_HEAP_SIZE ${BOARD_MIN_HEAP_SIZE})
set(MIN_STACK_SIZE ${BOARD_MIN_STACK_SIZE})
set(FLASH_ORIGIN ${BOARD_FLASH_ORIGIN})
set(RAM_ORIGIN ${BOARD_RAM_ORIGIN})
set(LINKER_SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/stm32f1.ld)

configure_file(
        ${CMAKE_CURRENT_LIST_DIR}/../../mcu/stm32f1/linker/stm32f1.ld.in
        ${LINKER_SCRIPT}
        @ONLY
)

target_link_options(hss_board_bluepill_f103 PUBLIC
        -mcpu=cortex-m3
        -mthumb
        -T${LINKER_SCRIPT}
        -Wl,--gc-sections
)

set_target_properties(hss_board_bluepill_f103 PROPERTIES
        HSS_MCU stm32f1
        HSS_FLASH_SIZE ${BOARD_FLASH_SIZE}
        HSS_RAM_SIZE ${BOARD_RAM_SIZE}
        HSS_LINKER_SCRIPT ${LINKER_SCRIPT}
        HSS_BOARD_COMPILE_DEFINITIONS "${BOARD_COMPILE_DEFINITIONS}"
        OPENOCD_INTERFACE ${BOARD_OPENOCD_INTERFACE}
        OPENOCD_TARGET ${BOARD_OPENOCD_TARGET}
)
