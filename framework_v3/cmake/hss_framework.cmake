include_guard(GLOBAL)

find_program(HSS_OBJCOPY arm-none-eabi-objcopy)
find_program(HSS_SIZE arm-none-eabi-size)
find_program(HSS_OPENOCD openocd)
find_program(HSS_GDB arm-none-eabi-gdb)

set(HSS_OPENOCD_TRANSPORT "swd" CACHE STRING "OpenOCD transport")
set(HSS_VSCODE_BUILD_TYPE "Debug" CACHE STRING "Build type used by generated VS Code configure tasks")

function(hss_normalize_target_name OUT_VAR NAME)
    string(MAKE_C_IDENTIFIER "${NAME}" NORMALIZED)
    set(${OUT_VAR} "${NORMALIZED}" PARENT_SCOPE)
endfunction()

function(hss_get_board_dir OUT_VAR BOARD_NAME)
    if (IS_ABSOLUTE "${BOARD_NAME}")
        set(BOARD_DIR "${BOARD_NAME}")
    else()
        set(BOARD_DIR "${HSS_FRAMEWORK_ROOT}/boards/${BOARD_NAME}")
    endif()

    if (NOT EXISTS "${BOARD_DIR}/board_manifest.cmake")
        message(FATAL_ERROR
                "Board '${BOARD_NAME}' does not have board_manifest.cmake. "
                "Run: python3 ${HSS_FRAMEWORK_ROOT}/tools/sync_board.py ${BOARD_DIR}")
    endif()

    set(${OUT_VAR} "${BOARD_DIR}" PARENT_SCOPE)
endfunction()

function(hss_add_board_sync_target BOARD_NAME BOARD_DIR)
    find_package(Python3 COMPONENTS Interpreter QUIET)
    if (NOT Python3_Interpreter_FOUND)
        return()
    endif()

    hss_normalize_target_name(BOARD_TARGET_SUFFIX "${BOARD_NAME}")
    set(SYNC_TARGET "sync_board_${BOARD_TARGET_SUFFIX}")
    if (TARGET "${SYNC_TARGET}")
        return()
    endif()

    add_custom_target("${SYNC_TARGET}"
            COMMAND "${Python3_EXECUTABLE}" "${HSS_FRAMEWORK_ROOT}/tools/sync_board.py" "${BOARD_DIR}"
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            COMMENT "Sync CubeMX board metadata for ${BOARD_NAME}"
            VERBATIM
    )
endfunction()

function(hss_write_board_roles_header BOARD_TARGET_SUFFIX OUT_INCLUDE_DIR)
    set(ROLE_INCLUDE_DIR "${CMAKE_BINARY_DIR}/hss_generated/${BOARD_TARGET_SUFFIX}")
    file(MAKE_DIRECTORY "${ROLE_INCLUDE_DIR}")
    set(ROLE_EXTRA_INCLUDES "")

    set(STATUS_LED_DEFINES "#define HSS_BOARD_HAS_STATUS_LED 0\n")
    if (DEFINED BOARD_ROLE_STATUS_LED)
        string(TOUPPER "${BOARD_ROLE_STATUS_LED}" STATUS_LED_PIN_NAME)
        string(REGEX MATCH "^P([A-Z])([0-9]+)$" _ "${STATUS_LED_PIN_NAME}")
        if (NOT CMAKE_MATCH_1 OR NOT CMAKE_MATCH_2)
            message(FATAL_ERROR
                    "BOARD_ROLE_STATUS_LED must use a pin name like PC13, got '${BOARD_ROLE_STATUS_LED}'")
        endif()

        set(STATUS_LED_ACTIVE_LOW 0)
        if (DEFINED BOARD_ROLE_STATUS_LED_ACTIVE_LOW AND BOARD_ROLE_STATUS_LED_ACTIVE_LOW)
            set(STATUS_LED_ACTIVE_LOW 1)
        endif()

        set(STATUS_LED_DEFINES
"#define HSS_BOARD_HAS_STATUS_LED 1
#define HSS_BOARD_STATUS_LED_PORT GPIO${CMAKE_MATCH_1}
#define HSS_BOARD_STATUS_LED_PIN GPIO_PIN_${CMAKE_MATCH_2}
#define HSS_BOARD_STATUS_LED_ACTIVE_LOW ${STATUS_LED_ACTIVE_LOW}
")
    endif()

    set(CONSOLE_UART_DEFINES "#define HSS_BOARD_HAS_CONSOLE_UART 0\n")
    if (DEFINED BOARD_ROLE_CONSOLE_UART)
        string(TOLOWER "${BOARD_ROLE_CONSOLE_UART}" CONSOLE_UART_HANDLE_SUFFIX)
        string(REGEX REPLACE "^usart" "uart" CONSOLE_UART_HANDLE_SUFFIX "${CONSOLE_UART_HANDLE_SUFFIX}")
        set(CONSOLE_UART_DEFINES
"#define HSS_BOARD_HAS_CONSOLE_UART 1
#define HSS_BOARD_CONSOLE_UART_HANDLE h${CONSOLE_UART_HANDLE_SUFFIX}
")
    endif()

    set(DEBUG_UART_DEFINES "#define HSS_BOARD_HAS_DEBUG_UART 0\n")
    if (DEFINED BOARD_ROLE_DEBUG_UART)
        string(TOLOWER "${BOARD_ROLE_DEBUG_UART}" DEBUG_UART_HANDLE_SUFFIX)
        string(REGEX REPLACE "^usart" "uart" DEBUG_UART_HANDLE_SUFFIX "${DEBUG_UART_HANDLE_SUFFIX}")
        set(DEBUG_UART_DEFINES
"#define HSS_BOARD_HAS_DEBUG_UART 1
#define HSS_BOARD_DEBUG_UART_HANDLE h${DEBUG_UART_HANDLE_SUFFIX}
")
    endif()

    set(MODBUS_UART_DEFINES
"#define HSS_BOARD_HAS_MODBUS_UART 0
#define HSS_BOARD_MODBUS_RS485_HARDWARE_DE 0
#define HSS_BOARD_MODBUS_RS485_MANUAL_DE 0
")
    if (DEFINED BOARD_ROLE_MODBUS_UART)
        string(TOLOWER "${BOARD_ROLE_MODBUS_UART}" MODBUS_UART_HANDLE_SUFFIX)
        string(REGEX REPLACE "^usart" "uart" MODBUS_UART_HANDLE_SUFFIX "${MODBUS_UART_HANDLE_SUFFIX}")

        set(MODBUS_RS485_HARDWARE_DE 0)
        if (DEFINED BOARD_ROLE_MODBUS_RS485_MODE)
            string(TOLOWER "${BOARD_ROLE_MODBUS_RS485_MODE}" MODBUS_RS485_MODE)
            if (MODBUS_RS485_MODE STREQUAL "hardware")
                set(MODBUS_RS485_HARDWARE_DE 1)
            elseif (NOT MODBUS_RS485_MODE STREQUAL "manual" AND NOT MODBUS_RS485_MODE STREQUAL "none")
                message(FATAL_ERROR
                        "BOARD_ROLE_MODBUS_RS485_MODE must be hardware, manual, or none; got '${BOARD_ROLE_MODBUS_RS485_MODE}'")
            endif()
        endif()

        set(MODBUS_RS485_DEFINES "#define HSS_BOARD_MODBUS_RS485_MANUAL_DE 0\n")
        if (DEFINED BOARD_ROLE_MODBUS_RS485_DE)
            string(TOUPPER "${BOARD_ROLE_MODBUS_RS485_DE}" MODBUS_RS485_DE_PIN_NAME)
            string(REGEX MATCH "^P([A-Z])([0-9]+)$" _ "${MODBUS_RS485_DE_PIN_NAME}")
            if (NOT CMAKE_MATCH_1 OR NOT CMAKE_MATCH_2)
                message(FATAL_ERROR
                        "BOARD_ROLE_MODBUS_RS485_DE must use a pin name like PA8, got '${BOARD_ROLE_MODBUS_RS485_DE}'")
            endif()

            set(MODBUS_RS485_DE_ACTIVE_HIGH 1)
            if (DEFINED BOARD_ROLE_MODBUS_RS485_DE_ACTIVE_HIGH AND NOT BOARD_ROLE_MODBUS_RS485_DE_ACTIVE_HIGH)
                set(MODBUS_RS485_DE_ACTIVE_HIGH 0)
            endif()

            set(MODBUS_RS485_DEFINES
"#define HSS_BOARD_MODBUS_RS485_MANUAL_DE 1
#define HSS_BOARD_MODBUS_RS485_DE_PORT GPIO${CMAKE_MATCH_1}
#define HSS_BOARD_MODBUS_RS485_DE_PIN GPIO_PIN_${CMAKE_MATCH_2}
#define HSS_BOARD_MODBUS_RS485_DE_ACTIVE_HIGH ${MODBUS_RS485_DE_ACTIVE_HIGH}
")
        endif()

        set(MODBUS_UART_DEFINES
"#define HSS_BOARD_HAS_MODBUS_UART 1
#define HSS_BOARD_MODBUS_UART_HANDLE h${MODBUS_UART_HANDLE_SUFFIX}
#define HSS_BOARD_MODBUS_RS485_HARDWARE_DE ${MODBUS_RS485_HARDWARE_DE}
${MODBUS_RS485_DEFINES}")
    endif()

    set(MODBUS_TIMER_DEFINES "#define HSS_BOARD_HAS_MODBUS_TIMER 0\n")
    if (DEFINED BOARD_ROLE_MODBUS_TIMER)
        string(TOLOWER "${BOARD_ROLE_MODBUS_TIMER}" MODBUS_TIMER_HANDLE_SUFFIX)
        set(MODBUS_TIMER_DEFINES
"#define HSS_BOARD_HAS_MODBUS_TIMER 1
#define HSS_BOARD_MODBUS_TIMER_HANDLE h${MODBUS_TIMER_HANDLE_SUFFIX}
")
    endif()

    set(SENSOR_SPI_DEFINES
"#define HSS_BOARD_HAS_SENSOR_SPI 0
#define HSS_BOARD_HAS_SENSOR_CS 0
")
    if (DEFINED BOARD_ROLE_SENSOR_SPI)
        string(TOLOWER "${BOARD_ROLE_SENSOR_SPI}" SENSOR_SPI_HANDLE_SUFFIX)
        set(ROLE_EXTRA_INCLUDES "${ROLE_EXTRA_INCLUDES}#include \"spi.h\"\n")

        set(SENSOR_CS_DEFINES "#define HSS_BOARD_HAS_SENSOR_CS 0\n")
        if (DEFINED BOARD_ROLE_SENSOR_CS)
            string(TOUPPER "${BOARD_ROLE_SENSOR_CS}" SENSOR_CS_PIN_NAME)
            string(REGEX MATCH "^P([A-Z])([0-9]+)$" _ "${SENSOR_CS_PIN_NAME}")
            if (NOT CMAKE_MATCH_1 OR NOT CMAKE_MATCH_2)
                message(FATAL_ERROR
                        "BOARD_ROLE_SENSOR_CS must use a pin name like PA4, got '${BOARD_ROLE_SENSOR_CS}'")
            endif()

            set(SENSOR_CS_ACTIVE_LOW 1)
            if (DEFINED BOARD_ROLE_SENSOR_CS_ACTIVE_LOW AND NOT BOARD_ROLE_SENSOR_CS_ACTIVE_LOW)
                set(SENSOR_CS_ACTIVE_LOW 0)
            endif()

            set(SENSOR_CS_DEFINES
"#define HSS_BOARD_HAS_SENSOR_CS 1
#define HSS_BOARD_SENSOR_CS_PORT GPIO${CMAKE_MATCH_1}
#define HSS_BOARD_SENSOR_CS_PIN GPIO_PIN_${CMAKE_MATCH_2}
#define HSS_BOARD_SENSOR_CS_ACTIVE_LOW ${SENSOR_CS_ACTIVE_LOW}
")
        endif()

        set(SENSOR_SPI_DEFINES
"#define HSS_BOARD_HAS_SENSOR_SPI 1
#define HSS_BOARD_SENSOR_SPI_HANDLE h${SENSOR_SPI_HANDLE_SUFFIX}
${SENSOR_CS_DEFINES}")
    endif()

    file(WRITE "${ROLE_INCLUDE_DIR}/hss_board_roles.h"
"/* Generated by framework_v3 CMake; do not edit by hand. */
#pragma once

#include \"main.h\"
${ROLE_EXTRA_INCLUDES}

${STATUS_LED_DEFINES}
${CONSOLE_UART_DEFINES}
${DEBUG_UART_DEFINES}
${MODBUS_UART_DEFINES}
${MODBUS_TIMER_DEFINES}
${SENSOR_SPI_DEFINES}")

    set(${OUT_INCLUDE_DIR} "${ROLE_INCLUDE_DIR}" PARENT_SCOPE)
endfunction()

function(hss_collect_hal_sources OUT_VAR HAL_DRIVER_DIR BOARD_CORE_INCLUDE_DIR)
    get_filename_component(HAL_DRIVER_NAME "${HAL_DRIVER_DIR}" NAME)
    string(REGEX MATCH "STM32[A-Za-z0-9]+xx" HAL_DEVICE_PREFIX "${HAL_DRIVER_NAME}")
    if (NOT HAL_DEVICE_PREFIX)
        message(FATAL_ERROR "Could not infer HAL source prefix from ${HAL_DRIVER_DIR}")
    endif()
    string(TOLOWER "${HAL_DEVICE_PREFIX}" HAL_SOURCE_PREFIX)

    set(HAL_SOURCES "${HAL_DRIVER_DIR}/Src/${HAL_SOURCE_PREFIX}_hal.c")

    file(GLOB HAL_CONF_FILES CONFIGURE_DEPENDS
            "${BOARD_CORE_INCLUDE_DIR}/stm32*xx_hal_conf.h"
    )
    if (NOT HAL_CONF_FILES)
        message(FATAL_ERROR "Could not find stm32*xx_hal_conf.h in ${BOARD_CORE_INCLUDE_DIR}")
    endif()
    list(GET HAL_CONF_FILES 0 HAL_CONF)

    file(STRINGS "${HAL_CONF}" ENABLED_MODULE_LINES
            REGEX "^[ \t]*#[ \t]*define[ \t]+HAL_[A-Z0-9_]+_MODULE_ENABLED"
    )

    foreach(MODULE_LINE IN LISTS ENABLED_MODULE_LINES)
        string(REGEX MATCH "HAL_([A-Z0-9_]+)_MODULE_ENABLED" _ "${MODULE_LINE}")
        set(MODULE_NAME "${CMAKE_MATCH_1}")
        if (MODULE_NAME STREQUAL "")
            continue()
        endif()

        string(TOLOWER "${MODULE_NAME}" MODULE_SOURCE_NAME)
        set(MODULE_SOURCE "${HAL_DRIVER_DIR}/Src/${HAL_SOURCE_PREFIX}_hal_${MODULE_SOURCE_NAME}.c")
        set(MODULE_EX_SOURCE "${HAL_DRIVER_DIR}/Src/${HAL_SOURCE_PREFIX}_hal_${MODULE_SOURCE_NAME}_ex.c")

        if (EXISTS "${MODULE_SOURCE}")
            list(APPEND HAL_SOURCES "${MODULE_SOURCE}")
        endif()
        if (EXISTS "${MODULE_EX_SOURCE}")
            list(APPEND HAL_SOURCES "${MODULE_EX_SOURCE}")
        endif()
    endforeach()

    list(REMOVE_DUPLICATES HAL_SOURCES)
    set(${OUT_VAR} "${HAL_SOURCES}" PARENT_SCOPE)
endfunction()

function(hss_register_board BOARD_NAME)
    if (HSS_ACTIVE_BOARD AND NOT HSS_ACTIVE_BOARD STREQUAL "${BOARD_NAME}")
        message(FATAL_ERROR
                "framework_v3 currently supports one selected board per CMake build. "
                "Already selected '${HSS_ACTIVE_BOARD}', cannot also select '${BOARD_NAME}'.")
    endif()

    hss_get_board_dir(BOARD_DIR "${BOARD_NAME}")
    hss_normalize_target_name(BOARD_TARGET_SUFFIX "${BOARD_NAME}")
    set(BOARD_TARGET "hss_board_${BOARD_TARGET_SUFFIX}")

    if (TARGET "${BOARD_TARGET}")
        target_link_libraries(hss_framework_board INTERFACE "${BOARD_TARGET}")
        set(HSS_ACTIVE_BOARD "${BOARD_NAME}" CACHE INTERNAL "")
        set(HSS_ACTIVE_BOARD_TARGET "${BOARD_TARGET}" CACHE INTERNAL "")
        return()
    endif()

    include("${BOARD_DIR}/board_manifest.cmake")
    if (EXISTS "${BOARD_DIR}/board_roles.cmake")
        include("${BOARD_DIR}/board_roles.cmake")
    endif()
    hss_write_board_roles_header("${BOARD_TARGET_SUFFIX}" BOARD_ROLE_INCLUDE_DIR)

    hss_collect_hal_sources(BOARD_HAL_SOURCES "${BOARD_HAL_DRIVER_DIR}" "${BOARD_CORE_INCLUDE_DIR}")

    add_library("${BOARD_TARGET}" OBJECT
            ${BOARD_STARTUP_SOURCE}
            ${BOARD_CORE_SOURCES}
            ${BOARD_GLUE_SOURCES}
            ${BOARD_HAL_SOURCES}
    )

    target_include_directories("${BOARD_TARGET}" PUBLIC
            "${BOARD_ROLE_INCLUDE_DIR}"
            "${BOARD_GENERATED_INCLUDE_DIR}"
            "${BOARD_CORE_INCLUDE_DIR}"
            "${BOARD_HAL_DRIVER_DIR}/Inc"
            "${BOARD_HAL_DRIVER_DIR}/Inc/Legacy"
            "${BOARD_CMSIS_DEVICE_DIR}"
            "${BOARD_CMSIS_CORE_DIR}"
    )

    target_compile_definitions("${BOARD_TARGET}" PUBLIC
            USE_HAL_DRIVER
            ${BOARD_MCU_DEFINE}
    )

    target_compile_options("${BOARD_TARGET}" PUBLIC
            ${BOARD_CPU_FLAGS}
            -ffunction-sections
            -fdata-sections
    )

    set_target_properties("${BOARD_TARGET}" PROPERTIES
            HSS_BOARD_NAME "${BOARD_NAME}"
            HSS_BOARD_DIR "${BOARD_DIR}"
            HSS_LINKER_SCRIPT "${BOARD_LINKER_SCRIPT}"
            HSS_OPENOCD_INTERFACE "${BOARD_OPENOCD_INTERFACE}"
            HSS_OPENOCD_TARGET "${BOARD_OPENOCD_TARGET}"
            HSS_CPU_FLAGS "${BOARD_CPU_FLAGS}"
    )

    hss_add_board_sync_target("${BOARD_NAME}" "${BOARD_DIR}")

    target_link_libraries(hss_framework_board INTERFACE "${BOARD_TARGET}")

    set(HSS_ACTIVE_BOARD "${BOARD_NAME}" CACHE INTERNAL "")
    set(HSS_ACTIVE_BOARD_TARGET "${BOARD_TARGET}" CACHE INTERNAL "")
endfunction()

function(hss_select_board BOARD_NAME)
    hss_register_board("${BOARD_NAME}")
endfunction()

function(hss_require_selected_board)
    if (NOT HSS_ACTIVE_BOARD_TARGET OR NOT TARGET "${HSS_ACTIVE_BOARD_TARGET}")
        message(FATAL_ERROR "No board selected. Call hss_select_board(<board>) before adding firmware.")
    endif()
endfunction()

function(hss_add_firmware TARGET_NAME)
    cmake_parse_arguments(ARG "GENERATE_VSCODE" "BOARD" "SOURCES" ${ARGN})

    if (ARG_BOARD)
        hss_select_board("${ARG_BOARD}")
    endif()
    hss_require_selected_board()

    set(FIRMWARE_SOURCES ${ARG_SOURCES} ${ARG_UNPARSED_ARGUMENTS})
    if (NOT FIRMWARE_SOURCES)
        message(FATAL_ERROR "hss_add_firmware(${TARGET_NAME}) requires at least one source file")
    endif()

    add_executable("${TARGET_NAME}"
            ${FIRMWARE_SOURCES}
            $<TARGET_OBJECTS:${HSS_ACTIVE_BOARD_TARGET}>
    )

    target_link_libraries("${TARGET_NAME}" PRIVATE
            hss_framework
            "${HSS_ACTIVE_BOARD_TARGET}"
    )

    get_target_property(HSS_LINKER_SCRIPT "${HSS_ACTIVE_BOARD_TARGET}" HSS_LINKER_SCRIPT)
    get_target_property(HSS_CPU_FLAGS "${HSS_ACTIVE_BOARD_TARGET}" HSS_CPU_FLAGS)

    set_target_properties("${TARGET_NAME}" PROPERTIES
            OUTPUT_NAME "${TARGET_NAME}"
            SUFFIX ".elf"
            HSS_BOARD_TARGET "${HSS_ACTIVE_BOARD_TARGET}"
            HSS_FIRMWARE_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}"
    )

    target_compile_options("${TARGET_NAME}" PRIVATE
            ${HSS_CPU_FLAGS}
            $<$<CONFIG:Debug>:-Og -g3>
            $<$<CONFIG:Release>:-O2 -DNDEBUG>
            $<$<CONFIG:MinSizeRel>:-Os -DNDEBUG>
    )

    target_link_options("${TARGET_NAME}" PRIVATE
            ${HSS_CPU_FLAGS}
            "SHELL:-T${HSS_LINKER_SCRIPT}"
            "SHELL:-Wl,-Map=$<TARGET_FILE_DIR:${TARGET_NAME}>/${TARGET_NAME}.map"
            "SHELL:-Wl,--undefined=__io_putchar"
            "SHELL:-Wl,--undefined=__io_getchar"
            -Wl,--gc-sections
            -specs=nosys.specs
            $<$<CONFIG:Debug>:-g3>
    )

    if (HSS_OBJCOPY)
        add_custom_command(TARGET "${TARGET_NAME}" POST_BUILD
                COMMAND "${HSS_OBJCOPY}" -O binary "$<TARGET_FILE:${TARGET_NAME}>" "$<TARGET_FILE_DIR:${TARGET_NAME}>/${TARGET_NAME}.bin"
                COMMAND "${HSS_OBJCOPY}" -O ihex "$<TARGET_FILE:${TARGET_NAME}>" "$<TARGET_FILE_DIR:${TARGET_NAME}>/${TARGET_NAME}.hex"
                COMMENT "Generating ${TARGET_NAME}.bin and ${TARGET_NAME}.hex"
                VERBATIM
        )
    else()
        message(WARNING "arm-none-eabi-objcopy not found; ${TARGET_NAME}.bin and ${TARGET_NAME}.hex will not be generated")
    endif()

    if (HSS_SIZE)
        add_custom_command(TARGET "${TARGET_NAME}" POST_BUILD
                COMMAND "${HSS_SIZE}" --format=berkeley "$<TARGET_FILE:${TARGET_NAME}>"
                COMMENT "Firmware size for ${TARGET_NAME}"
                VERBATIM
        )
    else()
        message(WARNING "arm-none-eabi-size not found; firmware size will not be reported")
    endif()

    hss_add_flash_target("${TARGET_NAME}")
    hss_add_openocd_target("${TARGET_NAME}")
    hss_add_vscode_target("${TARGET_NAME}")
    if (ARG_GENERATE_VSCODE)
        hss_generate_vscode("${TARGET_NAME}")
    endif()
endfunction()

function(hss_add_flash_target TARGET_NAME)
    if (NOT HSS_OPENOCD)
        message(WARNING "openocd not found; flash_${TARGET_NAME} target will not be available")
        return()
    endif()

    get_target_property(BOARD_TARGET "${TARGET_NAME}" HSS_BOARD_TARGET)
    get_target_property(OPENOCD_INTERFACE "${BOARD_TARGET}" HSS_OPENOCD_INTERFACE)
    get_target_property(OPENOCD_TARGET "${BOARD_TARGET}" HSS_OPENOCD_TARGET)

    add_custom_target("flash_${TARGET_NAME}"
            COMMAND "${HSS_OPENOCD}"
            -f "${OPENOCD_INTERFACE}"
            -f "${OPENOCD_TARGET}"
            -c "transport select ${HSS_OPENOCD_TRANSPORT}"
            -c "init"
            -c "reset halt"
            -c "program $<TARGET_FILE:${TARGET_NAME}> verify"
            -c "reset halt"
            -c "exit"
            DEPENDS "${TARGET_NAME}"
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
            COMMENT "Flashing ${TARGET_NAME} with OpenOCD"
            VERBATIM
    )
endfunction()

function(hss_add_openocd_target TARGET_NAME)
    if (NOT HSS_OPENOCD)
        return()
    endif()

    get_target_property(BOARD_TARGET "${TARGET_NAME}" HSS_BOARD_TARGET)
    get_target_property(OPENOCD_INTERFACE "${BOARD_TARGET}" HSS_OPENOCD_INTERFACE)
    get_target_property(OPENOCD_TARGET "${BOARD_TARGET}" HSS_OPENOCD_TARGET)

    add_custom_target("openocd_${TARGET_NAME}"
            COMMAND "${HSS_OPENOCD}"
            -f "${OPENOCD_INTERFACE}"
            -f "${OPENOCD_TARGET}"
            -c "transport select ${HSS_OPENOCD_TRANSPORT}"
            -c "gdb_port 3333"
            -c "init"
            -c "reset halt"
            DEPENDS "${TARGET_NAME}"
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
            COMMENT "Starting OpenOCD for ${TARGET_NAME}"
            VERBATIM
    )
endfunction()

function(hss_add_vscode_target TARGET_NAME)
    find_package(Python3 COMPONENTS Interpreter QUIET)
    if (NOT Python3_Interpreter_FOUND)
        message(WARNING "Python3 not found; vscode_${TARGET_NAME} target will not be available")
        return()
    endif()

    get_target_property(BOARD_TARGET "${TARGET_NAME}" HSS_BOARD_TARGET)
    get_target_property(BOARD_NAME "${BOARD_TARGET}" HSS_BOARD_NAME)
    get_target_property(OPENOCD_INTERFACE "${BOARD_TARGET}" HSS_OPENOCD_INTERFACE)
    get_target_property(OPENOCD_TARGET "${BOARD_TARGET}" HSS_OPENOCD_TARGET)
    hss_normalize_target_name(BOARD_TARGET_SUFFIX "${BOARD_NAME}")

    set(OPENOCD_PATH "openocd")
    if (HSS_OPENOCD)
        set(OPENOCD_PATH "${HSS_OPENOCD}")
    endif()

    set(GDB_PATH "arm-none-eabi-gdb")
    if (HSS_GDB)
        set(GDB_PATH "${HSS_GDB}")
    endif()

    set(GENERATOR_ARG "")
    if (CMAKE_GENERATOR)
        set(GENERATOR_ARG "--generator=${CMAKE_GENERATOR}")
    endif()

    add_custom_target("vscode_${TARGET_NAME}"
            COMMAND "${Python3_EXECUTABLE}" "${HSS_FRAMEWORK_ROOT}/tools/generate_vscode.py"
            "--workspace=${CMAKE_SOURCE_DIR}"
            "--source-dir=${CMAKE_SOURCE_DIR}"
            "--build-dir=${CMAKE_BINARY_DIR}"
            "--target=${TARGET_NAME}"
            "--board=${BOARD_NAME}"
            "--board-target-suffix=${BOARD_TARGET_SUFFIX}"
            "--elf=$<TARGET_FILE:${TARGET_NAME}>"
            "--toolchain-file=${HSS_FRAMEWORK_ROOT}/cmake/arm-gcc-toolchain.cmake"
            "--build-type=${HSS_VSCODE_BUILD_TYPE}"
            ${GENERATOR_ARG}
            "--openocd=${OPENOCD_PATH}"
            "--openocd-interface=${OPENOCD_INTERFACE}"
            "--openocd-target=${OPENOCD_TARGET}"
            "--openocd-transport=${HSS_OPENOCD_TRANSPORT}"
            "--gdb=${GDB_PATH}"
            COMMENT "Generating VS Code tasks and launch config for ${TARGET_NAME}"
            VERBATIM
    )
endfunction()

function(hss_generate_vscode TARGET_NAME)
    if (NOT TARGET "${TARGET_NAME}")
        message(FATAL_ERROR "hss_generate_vscode(${TARGET_NAME}) requires an existing firmware target")
    endif()

    find_package(Python3 COMPONENTS Interpreter QUIET)
    if (NOT Python3_Interpreter_FOUND)
        message(FATAL_ERROR "Python3 is required to generate VS Code workflow files")
    endif()

    get_target_property(BOARD_TARGET "${TARGET_NAME}" HSS_BOARD_TARGET)
    if (NOT BOARD_TARGET)
        message(FATAL_ERROR "hss_generate_vscode(${TARGET_NAME}) requires a target created by hss_add_firmware()")
    endif()

    get_target_property(BOARD_NAME "${BOARD_TARGET}" HSS_BOARD_NAME)
    get_target_property(OPENOCD_INTERFACE "${BOARD_TARGET}" HSS_OPENOCD_INTERFACE)
    get_target_property(OPENOCD_TARGET "${BOARD_TARGET}" HSS_OPENOCD_TARGET)
    get_target_property(FIRMWARE_BINARY_DIR "${TARGET_NAME}" HSS_FIRMWARE_BINARY_DIR)
    get_target_property(FIRMWARE_OUTPUT_NAME "${TARGET_NAME}" OUTPUT_NAME)
    get_target_property(FIRMWARE_SUFFIX "${TARGET_NAME}" SUFFIX)

    if (NOT FIRMWARE_BINARY_DIR)
        set(FIRMWARE_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    endif()
    if (NOT FIRMWARE_OUTPUT_NAME)
        set(FIRMWARE_OUTPUT_NAME "${TARGET_NAME}")
    endif()
    if (NOT FIRMWARE_SUFFIX)
        set(FIRMWARE_SUFFIX ".elf")
    endif()

    hss_normalize_target_name(BOARD_TARGET_SUFFIX "${BOARD_NAME}")

    set(OPENOCD_PATH "openocd")
    if (HSS_OPENOCD)
        set(OPENOCD_PATH "${HSS_OPENOCD}")
    endif()

    set(GDB_PATH "arm-none-eabi-gdb")
    if (HSS_GDB)
        set(GDB_PATH "${HSS_GDB}")
    endif()

    set(GENERATOR_ARGS "")
    if (CMAKE_GENERATOR)
        list(APPEND GENERATOR_ARGS "--generator=${CMAKE_GENERATOR}")
    endif()

    set(FIRMWARE_ELF "${FIRMWARE_BINARY_DIR}/${FIRMWARE_OUTPUT_NAME}${FIRMWARE_SUFFIX}")

    execute_process(
            COMMAND "${Python3_EXECUTABLE}" "${HSS_FRAMEWORK_ROOT}/tools/generate_vscode.py"
            "--workspace=${CMAKE_SOURCE_DIR}"
            "--source-dir=${CMAKE_SOURCE_DIR}"
            "--build-dir=${CMAKE_BINARY_DIR}"
            "--target=${TARGET_NAME}"
            "--board=${BOARD_NAME}"
            "--board-target-suffix=${BOARD_TARGET_SUFFIX}"
            "--elf=${FIRMWARE_ELF}"
            "--toolchain-file=${HSS_FRAMEWORK_ROOT}/cmake/arm-gcc-toolchain.cmake"
            "--build-type=${HSS_VSCODE_BUILD_TYPE}"
            ${GENERATOR_ARGS}
            "--openocd=${OPENOCD_PATH}"
            "--openocd-interface=${OPENOCD_INTERFACE}"
            "--openocd-target=${OPENOCD_TARGET}"
            "--openocd-transport=${HSS_OPENOCD_TRANSPORT}"
            "--gdb=${GDB_PATH}"
            RESULT_VARIABLE HSS_VSCODE_RESULT
    )

    if (NOT HSS_VSCODE_RESULT EQUAL 0)
        message(FATAL_ERROR "VS Code workflow generation failed for ${TARGET_NAME}")
    endif()

    message(STATUS "Generated VS Code workflow files for ${TARGET_NAME}")
endfunction()
