cmake_minimum_required(VERSION 3.20)

project(hss_framework LANGUAGES C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(FRAMEWORK_ROOT ${CMAKE_CURRENT_LIST_DIR} CACHE INTERNAL "")

# Umbrella convenience target (binds to the selected board).
add_library(hss_framework INTERFACE)

function(hss_select_board BOARD_NAME)
    set(BOARD_DIR ${FRAMEWORK_ROOT}/board/${BOARD_NAME})
    set(BOARD_CONFIG ${BOARD_DIR}/board_config.cmake)

    if (NOT EXISTS ${BOARD_CONFIG})
        message(FATAL_ERROR "Unsupported board: ${BOARD_NAME}")
    endif ()

    # Load board-owned configuration first.
    include(${BOARD_CONFIG})

    if (NOT DEFINED BOARD_MCU_FAMILY)
        message(FATAL_ERROR "Board ${BOARD_NAME} must define BOARD_MCU_FAMILY")
    endif ()

    if (NOT DEFINED HSS_BOARD_COMPILE_DEFINITIONS)
        message(FATAL_ERROR "Board ${BOARD_NAME} must define HSS_BOARD_COMPILE_DEFINITIONS")
    endif ()

    # Expose active board config for MCU CMake before MCU subdirectory is added.
    set(HSS_ACTIVE_BOARD ${BOARD_NAME} CACHE INTERNAL "")
    set(HSS_ACTIVE_MCU_FAMILY ${BOARD_MCU_FAMILY} CACHE INTERNAL "")
    set(HSS_ACTIVE_BOARD_DEFINITIONS ${HSS_BOARD_COMPILE_DEFINITIONS} CACHE INTERNAL "")
    set(HSS_ACTIVE_BOARD_FLASH_SIZE ${BOARD_FLASH_SIZE} CACHE INTERNAL "")
    set(HSS_ACTIVE_BOARD_RAM_SIZE ${BOARD_RAM_SIZE} CACHE INTERNAL "")
    set(HSS_ACTIVE_BOARD_OPENOCD_INTERFACE ${BOARD_OPENOCD_INTERFACE} CACHE INTERNAL "")
    set(HSS_ACTIVE_BOARD_OPENOCD_TARGET ${BOARD_OPENOCD_TARGET} CACHE INTERNAL "")

    set(MCU_TARGET hss_mcu_${BOARD_MCU_FAMILY})
    if (NOT TARGET ${MCU_TARGET})
        add_subdirectory(${FRAMEWORK_ROOT}/mcu/${BOARD_MCU_FAMILY}
                ${CMAKE_BINARY_DIR}/hss_framework_mcu_${BOARD_MCU_FAMILY})
    endif ()

    set(BOARD_TARGET hss_board_${BOARD_NAME})
    if (NOT TARGET ${BOARD_TARGET})
        add_subdirectory(${BOARD_DIR} ${CMAKE_BINARY_DIR}/hss_framework_board_${BOARD_NAME})
    endif ()

    target_link_libraries(hss_framework INTERFACE ${BOARD_TARGET})
endfunction()

function(hss_add_firmware TARGET_NAME)
    cmake_parse_arguments(ARG "" "BOARD" "" ${ARGN})

    if (ARG_BOARD)
        hss_select_board(${ARG_BOARD})
        set(SELECTED_BOARD ${ARG_BOARD})
    else ()
        if (NOT DEFINED HSS_ACTIVE_BOARD)
            message(FATAL_ERROR "hss_add_firmware: BOARD not specified and no board selected")
        endif ()
        set(SELECTED_BOARD ${HSS_ACTIVE_BOARD})
    endif ()

    set(BOARD_TARGET hss_board_${SELECTED_BOARD})
    if (NOT TARGET ${BOARD_TARGET})
        message(FATAL_ERROR "hss_add_firmware: unsupported board '${SELECTED_BOARD}'")
    endif ()

    set(SOURCES ${ARG_UNPARSED_ARGUMENTS})

    add_executable(${TARGET_NAME} ${SOURCES})
    target_link_libraries(${TARGET_NAME} PRIVATE ${BOARD_TARGET})

    set_target_properties(${TARGET_NAME} PROPERTIES
            HSS_BOARD ${BOARD_TARGET}
            OUTPUT_NAME "${TARGET_NAME}"
            SUFFIX ".elf"
    )

    target_compile_options(${TARGET_NAME} PRIVATE
            $<$<CONFIG:Debug>:-Og -g3>
            $<$<CONFIG:Release>:-O2 -DNDEBUG>
            $<$<CONFIG:MinSizeRel>:-Oz -DNDEBUG>
    )

    hss_add_flash_target(${TARGET_NAME})
    hss_generate_debug_script(${TARGET_NAME})
endfunction()

function(hss_add_flash_target TARGET_NAME)
    get_target_property(BOARD_TARGET ${TARGET_NAME} HSS_BOARD)

    if (NOT BOARD_TARGET)
        message(FATAL_ERROR "Firmware ${TARGET_NAME} has no board assigned")
    endif ()

    get_target_property(OPENOCD_INTERFACE ${BOARD_TARGET} OPENOCD_INTERFACE)
    get_target_property(OPENOCD_TARGET ${BOARD_TARGET} OPENOCD_TARGET)

    if (NOT OPENOCD_INTERFACE OR NOT OPENOCD_TARGET)
        message(FATAL_ERROR "Board ${BOARD_TARGET} missing OpenOCD config")
    endif ()

    add_custom_target(flash_${TARGET_NAME}
            COMMAND openocd
            -s ${FRAMEWORK_ROOT}/tools/openocd/scripts
            -f ${OPENOCD_INTERFACE}
            -f ${OPENOCD_TARGET}
            -c "transport select swd"
            -c "init"
            -c "reset halt"
            -c "program $<TARGET_FILE:${TARGET_NAME}> verify"
            -c "reset halt"
            -c "exit"
            DEPENDS ${TARGET_NAME}
    )
endfunction()

function(hss_generate_debug_script TARGET_NAME)
    get_target_property(BOARD_TARGET ${TARGET_NAME} HSS_BOARD)

    if (NOT BOARD_TARGET)
        message(FATAL_ERROR "Firmware ${TARGET_NAME} has no board assigned")
    endif ()

    get_target_property(OPENOCD_INTERFACE ${BOARD_TARGET} OPENOCD_INTERFACE)
    get_target_property(OPENOCD_TARGET ${BOARD_TARGET} OPENOCD_TARGET)

    set(TOOLS_DIR ${FRAMEWORK_ROOT}/tools/debug)
    file(MAKE_DIRECTORY ${TOOLS_DIR})

    set(SCRIPT ${TOOLS_DIR}/debug_${TARGET_NAME}.bat)

    file(GENERATE
            OUTPUT ${SCRIPT}
            CONTENT
            "@echo off
set ELF=$<TARGET_FILE:${TARGET_NAME}>

openocd ^
 -s ${FRAMEWORK_ROOT}/tools/openocd/scripts ^
 -f ${OPENOCD_INTERFACE} ^
 -f ${OPENOCD_TARGET} ^
 -c \"transport select swd\" ^
 -c \"gdb_port 3333\" ^
 -c \"init\" ^
 -c \"reset halt\""
    )
endfunction()