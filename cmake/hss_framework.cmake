include_guard(GLOBAL)

find_program(HSS_OBJCOPY arm-none-eabi-objcopy)
find_program(HSS_SIZE arm-none-eabi-size)
find_program(HSS_OPENOCD openocd)
find_program(HSS_GDB NAMES arm-none-eabi-gdb gdb-multiarch)

set(HSS_OPENOCD_TRANSPORT "swd" CACHE STRING "OpenOCD transport")
set(HSS_VSCODE_BUILD_TYPE "Debug" CACHE STRING "Build type used by generated VS Code configure tasks")
set(HSS_BOARD_PATHS "" CACHE STRING "Semicolon-separated directories containing HSS board folders")
set(HSS_CONFIG_PROFILES "" CACHE STRING "Semicolon-separated HSS config profiles")
set(HSS_CONFIG_OPTIONAL_PROFILES "" CACHE STRING "Semicolon-separated HSS config profiles that may be missing")
set(HSS_CONFIG_PROFILE_FILES "" CACHE STRING "Semicolon-separated additional HSS config profile files")
option(HSS_BOARD_AUTO_SYNC "Run board sync during configure when board metadata is missing" ON)
option(HSS_BOARD_SYNC_BEFORE_BUILD "Run board sync before compiling the selected board" ON)

include("${CMAKE_CURRENT_LIST_DIR}/hss_config.cmake")

function(hss_normalize_target_name OUT_VAR NAME)
    string(MAKE_C_IDENTIFIER "${NAME}" NORMALIZED)
    set(${OUT_VAR} "${NORMALIZED}" PARENT_SCOPE)
endfunction()

function(hss_normalize_board_root OUT_VAR BOARD_ROOT)
    if (IS_ABSOLUTE "${BOARD_ROOT}")
        set(NORMALIZED_ROOT "${BOARD_ROOT}")
    else()
        get_filename_component(NORMALIZED_ROOT "${BOARD_ROOT}" ABSOLUTE BASE_DIR "${CMAKE_SOURCE_DIR}")
    endif()
    set(${OUT_VAR} "${NORMALIZED_ROOT}" PARENT_SCOPE)
endfunction()

function(hss_board_lookup_error BOARD_NAME SEARCHED_PATHS)
    if (SEARCHED_PATHS)
        string(REPLACE ";" "\n  " SEARCHED_MESSAGE "${SEARCHED_PATHS}")
        set(SEARCHED_MESSAGE "Searched:\n  ${SEARCHED_MESSAGE}")
    else()
        set(SEARCHED_MESSAGE "No board paths were searched.")
    endif()

    message(FATAL_ERROR
            "Could not resolve HSS board '${BOARD_NAME}'.\n"
            "${SEARCHED_MESSAGE}\n"
            "Set HSS_BOARD_PATHS to one or more directories containing board folders, "
            "or pass an absolute/relative board directory path.")
endfunction()

function(hss_ensure_board_manifest BOARD_NAME BOARD_DIR)
    if (EXISTS "${BOARD_DIR}/board_manifest.cmake")
        return()
    endif()

    set(SYNC_COMMAND "python3 ${HSS_FRAMEWORK_ROOT}/tools/sync_board.py ${BOARD_DIR}")

    if (NOT HSS_BOARD_AUTO_SYNC)
        message(FATAL_ERROR
                "Board '${BOARD_NAME}' at '${BOARD_DIR}' is missing board_manifest.cmake.\n"
                "Run: ${SYNC_COMMAND}\n"
                "Or configure with -DHSS_BOARD_AUTO_SYNC=ON to let CMake run board sync.")
    endif()

    find_package(Python3 COMPONENTS Interpreter QUIET)
    if (NOT Python3_Interpreter_FOUND)
        message(FATAL_ERROR
                "Board '${BOARD_NAME}' at '${BOARD_DIR}' is missing board_manifest.cmake, "
                "and Python3 was not found for automatic board sync.\n"
                "Run manually after installing Python3: ${SYNC_COMMAND}")
    endif()

    message(STATUS "HSS board '${BOARD_NAME}' is missing generated metadata; running board sync")
    execute_process(
            COMMAND "${Python3_EXECUTABLE}" "${HSS_FRAMEWORK_ROOT}/tools/sync_board.py" "${BOARD_DIR}"
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            RESULT_VARIABLE HSS_BOARD_SYNC_RESULT
            OUTPUT_VARIABLE HSS_BOARD_SYNC_OUTPUT
            ERROR_VARIABLE HSS_BOARD_SYNC_ERROR
    )

    if (NOT HSS_BOARD_SYNC_RESULT EQUAL 0)
        message(FATAL_ERROR
                "Automatic board sync failed for '${BOARD_NAME}' at '${BOARD_DIR}'.\n"
                "Command: ${SYNC_COMMAND}\n"
                "stdout:\n${HSS_BOARD_SYNC_OUTPUT}\n"
                "stderr:\n${HSS_BOARD_SYNC_ERROR}")
    endif()

    if (NOT EXISTS "${BOARD_DIR}/board_manifest.cmake")
        message(FATAL_ERROR
                "Automatic board sync completed for '${BOARD_NAME}', but board_manifest.cmake "
                "was not created in '${BOARD_DIR}'.\n"
                "Run manually to inspect the issue: ${SYNC_COMMAND}")
    endif()
endfunction()

function(hss_get_board_dir OUT_VAR BOARD_NAME)
    set(CANDIDATE_DIRS "")

    if (IS_ABSOLUTE "${BOARD_NAME}")
        list(APPEND CANDIDATE_DIRS "${BOARD_NAME}")
    elseif ("${BOARD_NAME}" MATCHES "[/\\\\]")
        get_filename_component(BOARD_PATH "${BOARD_NAME}" ABSOLUTE BASE_DIR "${CMAKE_SOURCE_DIR}")
        list(APPEND CANDIDATE_DIRS "${BOARD_PATH}")
    else()
        get_filename_component(EXAMPLE_BOARD_ROOT "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../examples/boards" ABSOLUTE)
        foreach(BOARD_ROOT IN LISTS HSS_BOARD_PATHS)
            if (NOT BOARD_ROOT STREQUAL "")
                hss_normalize_board_root(NORMALIZED_BOARD_ROOT "${BOARD_ROOT}")
                list(APPEND CANDIDATE_DIRS "${NORMALIZED_BOARD_ROOT}/${BOARD_NAME}")
            endif()
        endforeach()
        list(APPEND CANDIDATE_DIRS "${EXAMPLE_BOARD_ROOT}/${BOARD_NAME}")
    endif()

    foreach(BOARD_DIR IN LISTS CANDIDATE_DIRS)
        if (EXISTS "${BOARD_DIR}")
            hss_ensure_board_manifest("${BOARD_NAME}" "${BOARD_DIR}")
            set(${OUT_VAR} "${BOARD_DIR}" PARENT_SCOPE)
            return()
        endif()
    endforeach()

    hss_board_lookup_error("${BOARD_NAME}" "${CANDIDATE_DIRS}")
endfunction()

function(hss_add_board_sync_target OUT_VAR BOARD_NAME BOARD_DIR)
    find_package(Python3 COMPONENTS Interpreter QUIET)
    if (NOT Python3_Interpreter_FOUND)
        set(${OUT_VAR} "" PARENT_SCOPE)
        return()
    endif()

    hss_normalize_target_name(BOARD_TARGET_SUFFIX "${BOARD_NAME}")
    set(SYNC_TARGET "sync_board_${BOARD_TARGET_SUFFIX}")
    if (TARGET "${SYNC_TARGET}")
        set(${OUT_VAR} "${SYNC_TARGET}" PARENT_SCOPE)
        return()
    endif()

    add_custom_target("${SYNC_TARGET}"
            COMMAND "${Python3_EXECUTABLE}" "${HSS_FRAMEWORK_ROOT}/tools/sync_board.py" "${BOARD_DIR}"
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            COMMENT "Sync CubeMX board metadata for ${BOARD_NAME}"
            VERBATIM
    )

    if (NOT TARGET hss_sync_boards)
        add_custom_target(hss_sync_boards
                COMMENT "Sync selected HSS board metadata"
        )
    endif()
    add_dependencies(hss_sync_boards "${SYNC_TARGET}")

    set(${OUT_VAR} "${SYNC_TARGET}" PARENT_SCOPE)
endfunction()

function(hss_parse_gpio_pin ROLE_NAME PIN_NAME OUT_PORT_LETTER OUT_PIN_NUMBER)
    string(TOUPPER "${PIN_NAME}" NORMALIZED_PIN_NAME)
    string(REGEX MATCH "^P([A-Z])([0-9]+)$" _ "${NORMALIZED_PIN_NAME}")
    if (NOT CMAKE_MATCH_1 OR NOT CMAKE_MATCH_2)
        message(FATAL_ERROR "${ROLE_NAME} must use a pin name like PA3, got '${PIN_NAME}'")
    endif()

    set(PIN_NUMBER "${CMAKE_MATCH_2}")
    if (PIN_NUMBER LESS 0 OR PIN_NUMBER GREATER 15)
        message(FATAL_ERROR "${ROLE_NAME} must use a GPIO pin number from 0 to 15, got '${PIN_NAME}'")
    endif()

    set(${OUT_PORT_LETTER} "${CMAKE_MATCH_1}" PARENT_SCOPE)
    set(${OUT_PIN_NUMBER} "${PIN_NUMBER}" PARENT_SCOPE)
endfunction()

function(hss_exti_irq_for_pin OUT_HANDLER OUT_IRQN MCU_FAMILY PIN_NUMBER)
    string(TOLOWER "${MCU_FAMILY}" FAMILY)

    if (FAMILY MATCHES "^stm32g0")
        if (PIN_NUMBER LESS 2)
            set(HANDLER "EXTI0_1_IRQHandler")
            set(IRQN "EXTI0_1_IRQn")
        elseif (PIN_NUMBER LESS 4)
            set(HANDLER "EXTI2_3_IRQHandler")
            set(IRQN "EXTI2_3_IRQn")
        else()
            set(HANDLER "EXTI4_15_IRQHandler")
            set(IRQN "EXTI4_15_IRQn")
        endif()
    else()
        if (PIN_NUMBER LESS 5)
            set(HANDLER "EXTI${PIN_NUMBER}_IRQHandler")
            set(IRQN "EXTI${PIN_NUMBER}_IRQn")
        elseif (PIN_NUMBER LESS 10)
            set(HANDLER "EXTI9_5_IRQHandler")
            set(IRQN "EXTI9_5_IRQn")
        else()
            set(HANDLER "EXTI15_10_IRQHandler")
            set(IRQN "EXTI15_10_IRQn")
        endif()
    endif()

    set(${OUT_HANDLER} "${HANDLER}" PARENT_SCOPE)
    set(${OUT_IRQN} "${IRQN}" PARENT_SCOPE)
endfunction()

function(hss_write_board_roles_header BOARD_TARGET_SUFFIX OUT_INCLUDE_DIR OUT_IRQ_SOURCE)
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

    set(WATCHDOG_DEFINES
"#define HSS_BOARD_HAS_WATCHDOG 0
#define HSS_BOARD_WATCHDOG_KIND_IWDG 0
#define HSS_BOARD_WATCHDOG_KIND_WWDG 0
")
    if (DEFINED BOARD_ROLE_WATCHDOG)
        string(TOLOWER "${BOARD_ROLE_WATCHDOG}" WATCHDOG_KIND)
        if (WATCHDOG_KIND STREQUAL "iwdg")
            set(ROLE_EXTRA_INCLUDES "${ROLE_EXTRA_INCLUDES}#include \"iwdg.h\"\n")
            set(WATCHDOG_DEFINES
"#define HSS_BOARD_HAS_WATCHDOG 1
#define HSS_BOARD_WATCHDOG_KIND_IWDG 1
#define HSS_BOARD_WATCHDOG_KIND_WWDG 0
#define HSS_BOARD_WATCHDOG_HANDLE hiwdg
")
        elseif (WATCHDOG_KIND STREQUAL "wwdg")
            set(ROLE_EXTRA_INCLUDES "${ROLE_EXTRA_INCLUDES}#include \"wwdg.h\"\n")
            set(WATCHDOG_DEFINES
"#define HSS_BOARD_HAS_WATCHDOG 1
#define HSS_BOARD_WATCHDOG_KIND_IWDG 0
#define HSS_BOARD_WATCHDOG_KIND_WWDG 1
#define HSS_BOARD_WATCHDOG_HANDLE hwwdg
")
        else()
            message(FATAL_ERROR
                    "BOARD_ROLE_WATCHDOG must be IWDG or WWDG; got '${BOARD_ROLE_WATCHDOG}'")
        endif()
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
#define HSS_BOARD_SENSOR_SPI_HAS_CONFIG 0
#define HSS_BOARD_SENSOR_SPI_HAS_MODE 0
#define HSS_BOARD_SENSOR_SPI_HAS_BAUD_PRESCALER 0
#define HSS_BOARD_SENSOR_SPI_HAS_NSS 0
#define HSS_BOARD_SENSOR_SPI_HAS_FIRST_BIT 0
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

            set(SENSOR_CS_INACTIVE_STATE "GPIO_PIN_SET")
            if (NOT SENSOR_CS_ACTIVE_LOW)
                set(SENSOR_CS_INACTIVE_STATE "GPIO_PIN_RESET")
            endif()
            if (DEFINED BOARD_ROLE_SENSOR_CS_IDLE)
                string(TOLOWER "${BOARD_ROLE_SENSOR_CS_IDLE}" SENSOR_CS_IDLE)
                if (SENSOR_CS_IDLE STREQUAL "active")
                    if (SENSOR_CS_ACTIVE_LOW)
                        set(SENSOR_CS_INACTIVE_STATE "GPIO_PIN_RESET")
                    else()
                        set(SENSOR_CS_INACTIVE_STATE "GPIO_PIN_SET")
                    endif()
                elseif (NOT SENSOR_CS_IDLE STREQUAL "inactive")
                    message(FATAL_ERROR
                            "BOARD_ROLE_SENSOR_CS_IDLE must be active or inactive; got '${BOARD_ROLE_SENSOR_CS_IDLE}'")
                endif()
            endif()

            set(SENSOR_CS_DEFINES
"#define HSS_BOARD_HAS_SENSOR_CS 1
#define HSS_BOARD_SENSOR_CS_PORT GPIO${CMAKE_MATCH_1}
#define HSS_BOARD_SENSOR_CS_PIN GPIO_PIN_${CMAKE_MATCH_2}
#define HSS_BOARD_SENSOR_CS_ACTIVE_LOW ${SENSOR_CS_ACTIVE_LOW}
#define HSS_BOARD_SENSOR_CS_INACTIVE_STATE ${SENSOR_CS_INACTIVE_STATE}
")
        endif()

        set(SENSOR_SPI_CONFIG_DEFINES
"#define HSS_BOARD_SENSOR_SPI_HAS_CONFIG 0
#define HSS_BOARD_SENSOR_SPI_HAS_MODE 0
#define HSS_BOARD_SENSOR_SPI_HAS_BAUD_PRESCALER 0
#define HSS_BOARD_SENSOR_SPI_HAS_NSS 0
#define HSS_BOARD_SENSOR_SPI_HAS_FIRST_BIT 0
")
        set(SENSOR_SPI_HAS_CONFIG 0)
        if (DEFINED BOARD_ROLE_SENSOR_SPI_MODE)
            set(SENSOR_SPI_HAS_CONFIG 1)
            if (BOARD_ROLE_SENSOR_SPI_MODE EQUAL 0)
                set(SENSOR_SPI_CLK_POLARITY "SPI_POLARITY_LOW")
                set(SENSOR_SPI_CLK_PHASE "SPI_PHASE_1EDGE")
            elseif (BOARD_ROLE_SENSOR_SPI_MODE EQUAL 1)
                set(SENSOR_SPI_CLK_POLARITY "SPI_POLARITY_LOW")
                set(SENSOR_SPI_CLK_PHASE "SPI_PHASE_2EDGE")
            elseif (BOARD_ROLE_SENSOR_SPI_MODE EQUAL 2)
                set(SENSOR_SPI_CLK_POLARITY "SPI_POLARITY_HIGH")
                set(SENSOR_SPI_CLK_PHASE "SPI_PHASE_1EDGE")
            elseif (BOARD_ROLE_SENSOR_SPI_MODE EQUAL 3)
                set(SENSOR_SPI_CLK_POLARITY "SPI_POLARITY_HIGH")
                set(SENSOR_SPI_CLK_PHASE "SPI_PHASE_2EDGE")
            else()
                message(FATAL_ERROR "BOARD_ROLE_SENSOR_SPI_MODE must be 0, 1, 2, or 3; got '${BOARD_ROLE_SENSOR_SPI_MODE}'")
            endif()
            set(SENSOR_SPI_MODE_DEFINES
"#define HSS_BOARD_SENSOR_SPI_HAS_MODE 1
#define HSS_BOARD_SENSOR_SPI_MODE ${BOARD_ROLE_SENSOR_SPI_MODE}
#define HSS_BOARD_SENSOR_SPI_CLK_POLARITY ${SENSOR_SPI_CLK_POLARITY}
#define HSS_BOARD_SENSOR_SPI_CLK_PHASE ${SENSOR_SPI_CLK_PHASE}
")
        else()
            set(SENSOR_SPI_MODE_DEFINES "#define HSS_BOARD_SENSOR_SPI_HAS_MODE 0\n")
        endif()

        if (DEFINED BOARD_ROLE_SENSOR_SPI_BAUD_PRESCALER)
            set(SENSOR_SPI_HAS_CONFIG 1)
            set(SENSOR_SPI_BAUD_DEFINES
"#define HSS_BOARD_SENSOR_SPI_HAS_BAUD_PRESCALER 1
#define HSS_BOARD_SENSOR_SPI_BAUD_PRESCALER SPI_BAUDRATEPRESCALER_${BOARD_ROLE_SENSOR_SPI_BAUD_PRESCALER}
")
        else()
            set(SENSOR_SPI_BAUD_DEFINES "#define HSS_BOARD_SENSOR_SPI_HAS_BAUD_PRESCALER 0\n")
        endif()

        if (DEFINED BOARD_ROLE_SENSOR_SPI_NSS)
            set(SENSOR_SPI_HAS_CONFIG 1)
            string(TOLOWER "${BOARD_ROLE_SENSOR_SPI_NSS}" SENSOR_SPI_NSS)
            if (SENSOR_SPI_NSS STREQUAL "software" OR SENSOR_SPI_NSS STREQUAL "soft")
                set(SENSOR_SPI_NSS_VALUE "SPI_NSS_SOFT")
            else()
                message(FATAL_ERROR "BOARD_ROLE_SENSOR_SPI_NSS currently supports only software; got '${BOARD_ROLE_SENSOR_SPI_NSS}'")
            endif()
            set(SENSOR_SPI_NSS_DEFINES
"#define HSS_BOARD_SENSOR_SPI_HAS_NSS 1
#define HSS_BOARD_SENSOR_SPI_NSS ${SENSOR_SPI_NSS_VALUE}
")
        else()
            set(SENSOR_SPI_NSS_DEFINES "#define HSS_BOARD_SENSOR_SPI_HAS_NSS 0\n")
        endif()

        if (DEFINED BOARD_ROLE_SENSOR_SPI_FIRST_BIT)
            set(SENSOR_SPI_HAS_CONFIG 1)
            string(TOLOWER "${BOARD_ROLE_SENSOR_SPI_FIRST_BIT}" SENSOR_SPI_FIRST_BIT)
            if (SENSOR_SPI_FIRST_BIT STREQUAL "msb")
                set(SENSOR_SPI_FIRST_BIT_VALUE "SPI_FIRSTBIT_MSB")
            elseif (SENSOR_SPI_FIRST_BIT STREQUAL "lsb")
                set(SENSOR_SPI_FIRST_BIT_VALUE "SPI_FIRSTBIT_LSB")
            else()
                message(FATAL_ERROR "BOARD_ROLE_SENSOR_SPI_FIRST_BIT must be msb or lsb; got '${BOARD_ROLE_SENSOR_SPI_FIRST_BIT}'")
            endif()
            set(SENSOR_SPI_FIRST_BIT_DEFINES
"#define HSS_BOARD_SENSOR_SPI_HAS_FIRST_BIT 1
#define HSS_BOARD_SENSOR_SPI_FIRST_BIT ${SENSOR_SPI_FIRST_BIT_VALUE}
")
        else()
            set(SENSOR_SPI_FIRST_BIT_DEFINES "#define HSS_BOARD_SENSOR_SPI_HAS_FIRST_BIT 0\n")
        endif()

        set(SENSOR_SPI_CONFIG_DEFINES
"#define HSS_BOARD_SENSOR_SPI_HAS_CONFIG ${SENSOR_SPI_HAS_CONFIG}
${SENSOR_SPI_MODE_DEFINES}${SENSOR_SPI_BAUD_DEFINES}${SENSOR_SPI_NSS_DEFINES}${SENSOR_SPI_FIRST_BIT_DEFINES}")

        set(SENSOR_SPI_DEFINES
"#define HSS_BOARD_HAS_SENSOR_SPI 1
#define HSS_BOARD_SENSOR_SPI_HANDLE h${SENSOR_SPI_HANDLE_SUFFIX}
${SENSOR_CS_DEFINES}${SENSOR_SPI_CONFIG_DEFINES}")
    endif()

    set(EXTI_DEFINES "#define HSS_BOARD_EXTI_ROLE_COUNT 0\n")
    set(EXTI_IRQ_SOURCE "")
    set(EXTI_ROLE_COUNT 0)
    set(EXTI_HANDLER_NAMES "")
    get_cmake_property(HSS_ALL_VARIABLES VARIABLES)
    foreach(HSS_VAR IN LISTS HSS_ALL_VARIABLES)
        if (HSS_VAR MATCHES "^BOARD_ROLE_EXTI_([A-Za-z0-9_]+)$")
            set(EXTI_LABEL "${CMAKE_MATCH_1}")
            if (EXTI_LABEL MATCHES "(_ACTIVE_LOW|_TRIGGER|_IRQ_PRIORITY|_IRQ_SUBPRIORITY)$")
                continue()
            endif()

            hss_parse_gpio_pin("${HSS_VAR}" "${${HSS_VAR}}" EXTI_PORT_LETTER EXTI_PIN_NUMBER)
            hss_exti_irq_for_pin(EXTI_IRQ_HANDLER EXTI_IRQN "${BOARD_MCU_FAMILY}" "${EXTI_PIN_NUMBER}")
            string(TOUPPER "${EXTI_LABEL}" EXTI_LABEL_UPPER)
            string(MAKE_C_IDENTIFIER "${EXTI_LABEL_UPPER}" EXTI_LABEL_ID)

            set(EXTI_ACTIVE_LOW 0)
            set(EXTI_ACTIVE_LOW_VAR "${HSS_VAR}_ACTIVE_LOW")
            if (DEFINED ${EXTI_ACTIVE_LOW_VAR} AND ${${EXTI_ACTIVE_LOW_VAR}})
                set(EXTI_ACTIVE_LOW 1)
            endif()

            set(EXTI_TRIGGER_RISING 0)
            set(EXTI_TRIGGER_FALLING 1)
            set(EXTI_TRIGGER_VAR "${HSS_VAR}_TRIGGER")
            if (DEFINED ${EXTI_TRIGGER_VAR})
                string(TOLOWER "${${EXTI_TRIGGER_VAR}}" EXTI_TRIGGER)
                if (EXTI_TRIGGER STREQUAL "rising")
                    set(EXTI_TRIGGER_RISING 1)
                    set(EXTI_TRIGGER_FALLING 0)
                elseif (EXTI_TRIGGER STREQUAL "falling")
                    set(EXTI_TRIGGER_RISING 0)
                    set(EXTI_TRIGGER_FALLING 1)
                elseif (EXTI_TRIGGER STREQUAL "both")
                    set(EXTI_TRIGGER_RISING 1)
                    set(EXTI_TRIGGER_FALLING 1)
                else()
                    message(FATAL_ERROR "${EXTI_TRIGGER_VAR} must be rising, falling, or both; got '${${EXTI_TRIGGER_VAR}}'")
                endif()
            endif()

            set(EXTI_PRIORITY 0)
            set(EXTI_PRIORITY_VAR "${HSS_VAR}_IRQ_PRIORITY")
            if (DEFINED ${EXTI_PRIORITY_VAR})
                set(EXTI_PRIORITY "${${EXTI_PRIORITY_VAR}}")
            endif()
            set(EXTI_SUBPRIORITY 0)
            set(EXTI_SUBPRIORITY_VAR "${HSS_VAR}_IRQ_SUBPRIORITY")
            if (DEFINED ${EXTI_SUBPRIORITY_VAR})
                set(EXTI_SUBPRIORITY "${${EXTI_SUBPRIORITY_VAR}}")
            endif()

            math(EXPR EXTI_ROLE_COUNT "${EXTI_ROLE_COUNT} + 1")
            string(APPEND EXTI_DEFINES
"#define HSS_BOARD_HAS_EXTI_${EXTI_LABEL_ID} 1
#define HSS_BOARD_EXTI_${EXTI_LABEL_ID}_PORT GPIO${EXTI_PORT_LETTER}
#define HSS_BOARD_EXTI_${EXTI_LABEL_ID}_PIN GPIO_PIN_${EXTI_PIN_NUMBER}
#define HSS_BOARD_EXTI_${EXTI_LABEL_ID}_PIN_NUMBER ${EXTI_PIN_NUMBER}
#define HSS_BOARD_EXTI_${EXTI_LABEL_ID}_ACTIVE_LOW ${EXTI_ACTIVE_LOW}
#define HSS_BOARD_EXTI_${EXTI_LABEL_ID}_TRIGGER_FALLING ${EXTI_TRIGGER_FALLING}
#define HSS_BOARD_EXTI_${EXTI_LABEL_ID}_TRIGGER_RISING ${EXTI_TRIGGER_RISING}
#define HSS_BOARD_EXTI_${EXTI_LABEL_ID}_IRQ_HANDLER ${EXTI_IRQ_HANDLER}
#define HSS_BOARD_EXTI_${EXTI_LABEL_ID}_IRQN ${EXTI_IRQN}
#define HSS_BOARD_EXTI_${EXTI_LABEL_ID}_IRQ_PRIORITY ${EXTI_PRIORITY}
#define HSS_BOARD_EXTI_${EXTI_LABEL_ID}_IRQ_SUBPRIORITY ${EXTI_SUBPRIORITY}
#define HSS_BOARD_EXTI_${EXTI_LABEL_ID}_INPUT ((hss_exti_input_t){GPIO_PIN_${EXTI_PIN_NUMBER}, ${EXTI_IRQN}, ${EXTI_PRIORITY}, ${EXTI_SUBPRIORITY}})
")

            list(FIND EXTI_HANDLER_NAMES "${EXTI_IRQ_HANDLER}" EXTI_HANDLER_INDEX)
            if (EXTI_HANDLER_INDEX EQUAL -1)
                list(APPEND EXTI_HANDLER_NAMES "${EXTI_IRQ_HANDLER}")
                set(EXTI_HANDLER_PINS_${EXTI_IRQ_HANDLER} "")
            endif()
            set(EXTI_HANDLER_PINS_${EXTI_IRQ_HANDLER} "${EXTI_HANDLER_PINS_${EXTI_IRQ_HANDLER}};GPIO_PIN_${EXTI_PIN_NUMBER}")
        endif()
    endforeach()
    string(REPLACE "#define HSS_BOARD_EXTI_ROLE_COUNT 0" "#define HSS_BOARD_EXTI_ROLE_COUNT ${EXTI_ROLE_COUNT}" EXTI_DEFINES "${EXTI_DEFINES}")

    set(EEPROM_DEFINES "#define HSS_BOARD_HAS_EEPROM_EMULATION 0\n")
    if (DEFINED BOARD_ROLE_EEPROM_EMULATION AND BOARD_ROLE_EEPROM_EMULATION)
        set(EEPROM_DEFINES "#define HSS_BOARD_HAS_EEPROM_EMULATION 1\n")
    endif()

    if (EXTI_ROLE_COUNT GREATER 0)
        set(IRQ_FUNCTIONS "")
        foreach(EXTI_HANDLER IN LISTS EXTI_HANDLER_NAMES)
            string(APPEND IRQ_FUNCTIONS "void ${EXTI_HANDLER}(void)\n{\n")
            foreach(EXTI_PIN IN LISTS EXTI_HANDLER_PINS_${EXTI_HANDLER})
                if (NOT EXTI_PIN STREQUAL "")
                    string(APPEND IRQ_FUNCTIONS "    HAL_GPIO_EXTI_IRQHandler(${EXTI_PIN});\n")
                endif()
            endforeach()
            string(APPEND IRQ_FUNCTIONS "}\n\n")
        endforeach()
        set(EXTI_IRQ_SOURCE "${ROLE_INCLUDE_DIR}/hss_board_irqs.c")
        file(WRITE "${EXTI_IRQ_SOURCE}"
"/* Generated by HSS STM32 framework CMake; do not edit by hand. */
#include \"hss_board_roles.h\"

${IRQ_FUNCTIONS}")
    endif()

    file(WRITE "${ROLE_INCLUDE_DIR}/hss_board_roles.h"
"/* Generated by HSS STM32 framework CMake; do not edit by hand. */
#pragma once

#include \"main.h\"
${ROLE_EXTRA_INCLUDES}

${STATUS_LED_DEFINES}
${CONSOLE_UART_DEFINES}
${DEBUG_UART_DEFINES}
${WATCHDOG_DEFINES}
${MODBUS_UART_DEFINES}
${MODBUS_TIMER_DEFINES}
${SENSOR_SPI_DEFINES}
${EXTI_DEFINES}
${EEPROM_DEFINES}")

    set(${OUT_INCLUDE_DIR} "${ROLE_INCLUDE_DIR}" PARENT_SCOPE)
    set(${OUT_IRQ_SOURCE} "${EXTI_IRQ_SOURCE}" PARENT_SCOPE)
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

function(hss_parse_memory_size OUT_VAR SIZE_TEXT)
    string(STRIP "${SIZE_TEXT}" NORMALIZED_SIZE)
    string(TOUPPER "${NORMALIZED_SIZE}" NORMALIZED_SIZE)

    if (NORMALIZED_SIZE MATCHES "^[0-9]+K$")
        string(REGEX REPLACE "K$" "" SIZE_NUMBER "${NORMALIZED_SIZE}")
        math(EXPR SIZE_BYTES "${SIZE_NUMBER} * 1024")
    elseif (NORMALIZED_SIZE MATCHES "^[0-9]+M$")
        string(REGEX REPLACE "M$" "" SIZE_NUMBER "${NORMALIZED_SIZE}")
        math(EXPR SIZE_BYTES "${SIZE_NUMBER} * 1024 * 1024")
    else()
        math(EXPR SIZE_BYTES "${NORMALIZED_SIZE}")
    endif()

    set(${OUT_VAR} "${SIZE_BYTES}" PARENT_SCOPE)
endfunction()

function(hss_format_memory_size OUT_VAR SIZE_BYTES)
    math(EXPR SIZE_REMAINDER "${SIZE_BYTES} % 1024")
    if (SIZE_REMAINDER EQUAL 0)
        math(EXPR SIZE_KIB "${SIZE_BYTES} / 1024")
        set(${OUT_VAR} "${SIZE_KIB}K" PARENT_SCOPE)
    else()
        set(${OUT_VAR} "${SIZE_BYTES}" PARENT_SCOPE)
    endif()
endfunction()

function(hss_generate_linker_script_from_config OUT_VAR BOARD_TARGET_SUFFIX BOARD_SOURCE_LINKER_SCRIPT RESERVE_ORIGIN RESERVE_SIZE)
    set(GENERATED_LINKER_DIR "${CMAKE_BINARY_DIR}/hss_generated/${BOARD_TARGET_SUFFIX}")
    set(GENERATED_LINKER_SCRIPT "${GENERATED_LINKER_DIR}/linker.ld")

    if (NOT EXISTS "${BOARD_SOURCE_LINKER_SCRIPT}")
        message(FATAL_ERROR "Source linker script not found: ${BOARD_SOURCE_LINKER_SCRIPT}")
    endif()

    file(READ "${BOARD_SOURCE_LINKER_SCRIPT}" LINKER_CONTENT)
    string(REGEX MATCH "([ \t]*FLASH \\(rx\\)[ \t]*:[ \t]*ORIGIN = [^,]+,[ \t]*LENGTH = )[^\n]+" FLASH_LINE "${LINKER_CONTENT}")
    if (NOT FLASH_LINE)
        message(FATAL_ERROR "Could not find FLASH memory definition in ${BOARD_SOURCE_LINKER_SCRIPT}")
    endif()

    string(REGEX MATCH "^[ \t]*FLASH \\(rx\\)[ \t]*:[ \t]*ORIGIN = ([^,]+),[ \t]*LENGTH = ([^\n]+)$" _ "${FLASH_LINE}")
    set(REGION_ORIGIN_TEXT "${CMAKE_MATCH_1}")
    set(REGION_LENGTH_TEXT "${CMAKE_MATCH_2}")
    math(EXPR RESERVE_ORIGIN_BYTES "${RESERVE_ORIGIN}")
    math(EXPR RESERVE_SIZE_BYTES "${RESERVE_SIZE}")
    math(EXPR RESERVE_END_BYTES "${RESERVE_ORIGIN_BYTES} + ${RESERVE_SIZE_BYTES}")
    math(EXPR REGION_ORIGIN_BYTES "${REGION_ORIGIN_TEXT}")
    hss_parse_memory_size(REGION_LENGTH_BYTES "${REGION_LENGTH_TEXT}")
    math(EXPR REGION_END_BYTES "${REGION_ORIGIN_BYTES} + ${REGION_LENGTH_BYTES}")

    if (RESERVE_ORIGIN_BYTES LESS REGION_ORIGIN_BYTES OR RESERVE_END_BYTES GREATER REGION_END_BYTES)
        message(FATAL_ERROR
                "EEPROM reservation ${RESERVE_ORIGIN_BYTES}..${RESERVE_END_BYTES} is outside the FLASH region "
                "${REGION_ORIGIN_BYTES}..${REGION_END_BYTES}")
    endif()
    if (NOT RESERVE_END_BYTES EQUAL REGION_END_BYTES)
        message(FATAL_ERROR
                "EEPROM reservation must end at the end of FLASH; expected ${REGION_END_BYTES}, got ${RESERVE_END_BYTES}")
    endif()

    math(EXPR NEW_LENGTH_BYTES "${RESERVE_ORIGIN_BYTES} - ${REGION_ORIGIN_BYTES}")
    if (NEW_LENGTH_BYTES LESS_EQUAL 0)
        message(FATAL_ERROR "EEPROM reservation consumes the entire FLASH region")
    endif()

    hss_format_memory_size(NEW_LENGTH_TEXT "${NEW_LENGTH_BYTES}")
    set(NEW_FLASH_LINE "${CMAKE_MATCH_0}")
    string(REGEX REPLACE "([ \t]*FLASH \\(rx\\)[ \t]*:[ \t]*ORIGIN = [^,]+,[ \t]*LENGTH = )[^\n]+"
            "\\1${NEW_LENGTH_TEXT}"
            NEW_FLASH_LINE "${NEW_FLASH_LINE}")
    string(REPLACE "${FLASH_LINE}" "${NEW_FLASH_LINE}\n/* HSS reserved ${RESERVE_SIZE_BYTES} bytes at ${RESERVE_ORIGIN_BYTES} for flash-backed persistence. */"
            LINKER_CONTENT "${LINKER_CONTENT}")

    file(MAKE_DIRECTORY "${GENERATED_LINKER_DIR}")
    file(WRITE "${GENERATED_LINKER_SCRIPT}" "${LINKER_CONTENT}")
    set(${OUT_VAR} "${GENERATED_LINKER_SCRIPT}" PARENT_SCOPE)
endfunction()

function(hss_validate_eeprom_config TARGET_NAME)
    if (NOT DEFINED HSS_CONFIG_VALUE_HSS_EEPROM_FLASH_ORIGIN
            OR NOT DEFINED HSS_CONFIG_VALUE_HSS_EEPROM_FLASH_SIZE
            OR NOT DEFINED HSS_CONFIG_VALUE_HSS_EEPROM_PAGE_SIZE
            OR NOT DEFINED HSS_CONFIG_VALUE_HSS_EEPROM_SLOT_COUNT)
        message(FATAL_ERROR
                "Config enabled EEPROM emulation for '${TARGET_NAME}', but EEPROM flash values are missing")
    endif()

    math(EXPR HSS_EEPROM_FLASH_ORIGIN_BYTES "${HSS_CONFIG_VALUE_HSS_EEPROM_FLASH_ORIGIN}")
    math(EXPR HSS_EEPROM_FLASH_SIZE_BYTES "${HSS_CONFIG_VALUE_HSS_EEPROM_FLASH_SIZE}")
    math(EXPR HSS_EEPROM_PAGE_SIZE_BYTES "${HSS_CONFIG_VALUE_HSS_EEPROM_PAGE_SIZE}")
    math(EXPR HSS_EEPROM_SLOT_COUNT_VALUE "${HSS_CONFIG_VALUE_HSS_EEPROM_SLOT_COUNT}")

    if (HSS_EEPROM_FLASH_ORIGIN_BYTES LESS_EQUAL 0)
        message(FATAL_ERROR
                "Config enabled EEPROM emulation for '${TARGET_NAME}', but HSS_EEPROM_FLASH_ORIGIN must be non-zero")
    endif()
    if (HSS_EEPROM_FLASH_SIZE_BYTES LESS_EQUAL 0)
        message(FATAL_ERROR
                "Config enabled EEPROM emulation for '${TARGET_NAME}', but HSS_EEPROM_FLASH_SIZE must be non-zero")
    endif()
    if (HSS_EEPROM_PAGE_SIZE_BYTES LESS_EQUAL 0)
        message(FATAL_ERROR
                "Config enabled EEPROM emulation for '${TARGET_NAME}', but HSS_EEPROM_PAGE_SIZE must be non-zero")
    endif()
    if (HSS_EEPROM_SLOT_COUNT_VALUE LESS_EQUAL 0)
        message(FATAL_ERROR
                "Config enabled EEPROM emulation for '${TARGET_NAME}', but HSS_EEPROM_SLOT_COUNT must be non-zero")
    endif()
    math(EXPR HSS_EEPROM_SIZE_REMAINDER "${HSS_EEPROM_FLASH_SIZE_BYTES} % ${HSS_EEPROM_PAGE_SIZE_BYTES}")
    if (NOT HSS_EEPROM_SIZE_REMAINDER EQUAL 0)
        message(FATAL_ERROR
                "Config enabled EEPROM emulation for '${TARGET_NAME}', but HSS_EEPROM_FLASH_SIZE must be a multiple of HSS_EEPROM_PAGE_SIZE")
    endif()
    math(EXPR HSS_EEPROM_ORIGIN_REMAINDER "${HSS_EEPROM_FLASH_ORIGIN_BYTES} % ${HSS_EEPROM_PAGE_SIZE_BYTES}")
    if (NOT HSS_EEPROM_ORIGIN_REMAINDER EQUAL 0)
        message(FATAL_ERROR
                "Config enabled EEPROM emulation for '${TARGET_NAME}', but HSS_EEPROM_FLASH_ORIGIN must be aligned to HSS_EEPROM_PAGE_SIZE")
    endif()
    if (HSS_EEPROM_FLASH_SIZE_BYTES LESS 8)
        message(FATAL_ERROR
                "Config enabled EEPROM emulation for '${TARGET_NAME}', but HSS_EEPROM_FLASH_SIZE must be at least 8 bytes")
    endif()
    math(EXPR HSS_EEPROM_MAX_RECORDS "${HSS_EEPROM_FLASH_SIZE_BYTES} / 8")
    if (HSS_EEPROM_SLOT_COUNT_VALUE GREATER HSS_EEPROM_MAX_RECORDS)
        message(FATAL_ERROR
                "Config enabled EEPROM emulation for '${TARGET_NAME}', but HSS_EEPROM_SLOT_COUNT=${HSS_EEPROM_SLOT_COUNT_VALUE} exceeds the maximum record capacity ${HSS_EEPROM_MAX_RECORDS}")
    endif()
endfunction()

function(hss_register_board BOARD_NAME)
    if (HSS_ACTIVE_BOARD AND NOT HSS_ACTIVE_BOARD STREQUAL "${BOARD_NAME}")
        message(FATAL_ERROR
                "HSS STM32 framework currently supports one selected board per CMake build. "
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
    set(BOARD_HAS_EEPROM_EMULATION OFF)
    if (DEFINED BOARD_ROLE_EEPROM_EMULATION AND BOARD_ROLE_EEPROM_EMULATION)
        set(BOARD_HAS_EEPROM_EMULATION ON)
    endif()
    string(TOUPPER "${BOARD_MCU_FAMILY}" BOARD_MCU_FAMILY_UPPER)
    hss_write_board_roles_header("${BOARD_TARGET_SUFFIX}" BOARD_ROLE_INCLUDE_DIR BOARD_IRQ_SOURCE)

    hss_collect_hal_sources(BOARD_HAL_SOURCES "${BOARD_HAL_DRIVER_DIR}" "${BOARD_CORE_INCLUDE_DIR}")

    add_library("${BOARD_TARGET}" OBJECT
            ${BOARD_STARTUP_SOURCE}
            ${BOARD_CORE_SOURCES}
            ${BOARD_GLUE_SOURCES}
            ${BOARD_IRQ_SOURCE}
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
            "HSS_BOARD_MCU_FAMILY_${BOARD_MCU_FAMILY_UPPER}"
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
            HSS_SOURCE_LINKER_SCRIPT "${BOARD_LINKER_SCRIPT}"
            HSS_LINKER_SCRIPT "${BOARD_LINKER_SCRIPT}"
            HSS_HAS_EEPROM_EMULATION "${BOARD_HAS_EEPROM_EMULATION}"
            HSS_OPENOCD_INTERFACE "${BOARD_OPENOCD_INTERFACE}"
            HSS_OPENOCD_TARGET "${BOARD_OPENOCD_TARGET}"
            HSS_OPENOCD_TRANSPORT "${BOARD_OPENOCD_TRANSPORT}"
            HSS_CPU_FLAGS "${BOARD_CPU_FLAGS}"
    )

    hss_add_board_sync_target(BOARD_SYNC_TARGET "${BOARD_NAME}" "${BOARD_DIR}")
    if (HSS_BOARD_SYNC_BEFORE_BUILD)
        if (BOARD_SYNC_TARGET)
            add_dependencies("${BOARD_TARGET}" "${BOARD_SYNC_TARGET}")
        else()
            message(WARNING
                    "Python3 not found; board sync will not run before building '${BOARD_NAME}'")
        endif()
    endif()

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
    cmake_parse_arguments(ARG
            "GENERATE_VSCODE"
            "BOARD;CONFIG"
            "SOURCES;PROFILES;OPTIONAL_PROFILES;PROFILE_FILES"
            ${ARGN}
    )

    if (ARG_BOARD)
        hss_select_board("${ARG_BOARD}")
    endif()
    hss_require_selected_board()

    set(FIRMWARE_SOURCES ${ARG_SOURCES} ${ARG_UNPARSED_ARGUMENTS})
    if (NOT FIRMWARE_SOURCES)
        message(FATAL_ERROR "hss_add_firmware(${TARGET_NAME}) requires at least one source file")
    endif()

    set(HSS_TARGET_CONFIG_INCLUDE_DIR "")
    set(HSS_TARGET_CONFIG_COMPILE_DEFINITIONS "")
    if (ARG_CONFIG)
        set(ACTIVE_CONFIG_PROFILES ${ARG_PROFILES})
        if (NOT ACTIVE_CONFIG_PROFILES)
            set(ACTIVE_CONFIG_PROFILES ${HSS_CONFIG_PROFILES})
        endif()
        set(ACTIVE_OPTIONAL_CONFIG_PROFILES ${ARG_OPTIONAL_PROFILES})
        if (NOT ACTIVE_OPTIONAL_CONFIG_PROFILES)
            set(ACTIVE_OPTIONAL_CONFIG_PROFILES ${HSS_CONFIG_OPTIONAL_PROFILES})
        endif()
        set(ACTIVE_CONFIG_PROFILE_FILES ${ARG_PROFILE_FILES})
        if (NOT ACTIVE_CONFIG_PROFILE_FILES)
            set(ACTIVE_CONFIG_PROFILE_FILES ${HSS_CONFIG_PROFILE_FILES})
        endif()
        hss_generate_target_config(
                "${TARGET_NAME}"
                "${ARG_CONFIG}"
                HSS_TARGET_CONFIG_INCLUDE_DIR
                PROFILES ${ACTIVE_CONFIG_PROFILES}
                OPTIONAL_PROFILES ${ACTIVE_OPTIONAL_CONFIG_PROFILES}
                PROFILE_FILES ${ACTIVE_CONFIG_PROFILE_FILES}
        )
        set(HSS_TARGET_CONFIG_COMPILE_DEFINITIONS ${HSS_CONFIG_COMPILE_DEFINITIONS})
        message(STATUS "Generated HSS config for ${TARGET_NAME} with profiles: ${HSS_CONFIG_ACTIVE_PROFILES}")
    endif()

    get_target_property(BOARD_NAME "${HSS_ACTIVE_BOARD_TARGET}" HSS_BOARD_NAME)
    get_target_property(BOARD_SOURCE_LINKER_SCRIPT "${HSS_ACTIVE_BOARD_TARGET}" HSS_SOURCE_LINKER_SCRIPT)
    get_target_property(BOARD_HAS_EEPROM_EMULATION "${HSS_ACTIVE_BOARD_TARGET}" HSS_HAS_EEPROM_EMULATION)
    hss_normalize_target_name(BOARD_TARGET_SUFFIX "${BOARD_NAME}")

    set(HSS_FIRMWARE_LINKER_SCRIPT "${BOARD_SOURCE_LINKER_SCRIPT}")
    if (DEFINED HSS_CONFIG_VALUE_HSS_ENABLE_EEPROM_EMULATION AND HSS_CONFIG_VALUE_HSS_ENABLE_EEPROM_EMULATION)
        if (NOT BOARD_HAS_EEPROM_EMULATION)
            message(FATAL_ERROR
                    "Config enabled EEPROM emulation for '${TARGET_NAME}', but board '${BOARD_NAME}' does not declare support in board_roles.cmake")
        endif()
        hss_validate_eeprom_config("${TARGET_NAME}")

        hss_generate_linker_script_from_config(
                HSS_FIRMWARE_LINKER_SCRIPT
                "${BOARD_TARGET_SUFFIX}"
                "${BOARD_SOURCE_LINKER_SCRIPT}"
                "${HSS_CONFIG_VALUE_HSS_EEPROM_FLASH_ORIGIN}"
                "${HSS_CONFIG_VALUE_HSS_EEPROM_FLASH_SIZE}"
        )
    endif()

    add_executable("${TARGET_NAME}"
            ${FIRMWARE_SOURCES}
            $<TARGET_OBJECTS:${HSS_ACTIVE_BOARD_TARGET}>
    )

    target_link_libraries("${TARGET_NAME}" PRIVATE
            hss_framework
            "${HSS_ACTIVE_BOARD_TARGET}"
    )

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

    if (HSS_TARGET_CONFIG_INCLUDE_DIR)
        target_include_directories("${TARGET_NAME}" PRIVATE "${HSS_TARGET_CONFIG_INCLUDE_DIR}")
    endif()
    if (HSS_TARGET_CONFIG_COMPILE_DEFINITIONS)
        target_compile_definitions("${TARGET_NAME}" PRIVATE ${HSS_TARGET_CONFIG_COMPILE_DEFINITIONS})
    endif()

    target_link_options("${TARGET_NAME}" PRIVATE
            ${HSS_CPU_FLAGS}
            "SHELL:-T${HSS_FIRMWARE_LINKER_SCRIPT}"
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
    get_target_property(OPENOCD_TRANSPORT "${BOARD_TARGET}" HSS_OPENOCD_TRANSPORT)
    if (NOT OPENOCD_TRANSPORT)
        set(OPENOCD_TRANSPORT "${HSS_OPENOCD_TRANSPORT}")
    endif()

    add_custom_target("flash_${TARGET_NAME}"
            COMMAND "${HSS_OPENOCD}"
            -f "${OPENOCD_INTERFACE}"
            -f "${OPENOCD_TARGET}"
            -c "transport select ${OPENOCD_TRANSPORT}"
            -c "init"
            -c "reset halt"
            -c "program $<TARGET_FILE:${TARGET_NAME}> verify"
            -c "reset run"
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
    get_target_property(OPENOCD_TRANSPORT "${BOARD_TARGET}" HSS_OPENOCD_TRANSPORT)
    if (NOT OPENOCD_TRANSPORT)
        set(OPENOCD_TRANSPORT "${HSS_OPENOCD_TRANSPORT}")
    endif()

    add_custom_target("openocd_${TARGET_NAME}"
            COMMAND "${HSS_OPENOCD}"
            -f "${OPENOCD_INTERFACE}"
            -f "${OPENOCD_TARGET}"
            -c "transport select ${OPENOCD_TRANSPORT}"
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
    get_target_property(OPENOCD_TRANSPORT "${BOARD_TARGET}" HSS_OPENOCD_TRANSPORT)
    if (NOT OPENOCD_TRANSPORT)
        set(OPENOCD_TRANSPORT "${HSS_OPENOCD_TRANSPORT}")
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
            "--openocd-transport=${OPENOCD_TRANSPORT}"
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
    get_target_property(OPENOCD_TRANSPORT "${BOARD_TARGET}" HSS_OPENOCD_TRANSPORT)
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
    if (NOT OPENOCD_TRANSPORT)
        set(OPENOCD_TRANSPORT "${HSS_OPENOCD_TRANSPORT}")
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
            "--openocd-transport=${OPENOCD_TRANSPORT}"
            "--gdb=${GDB_PATH}"
            RESULT_VARIABLE HSS_VSCODE_RESULT
    )

    if (NOT HSS_VSCODE_RESULT EQUAL 0)
        message(FATAL_ERROR "VS Code workflow generation failed for ${TARGET_NAME}")
    endif()

    message(STATUS "Generated VS Code workflow files for ${TARGET_NAME}")
endfunction()
