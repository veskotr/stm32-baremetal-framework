include_guard(GLOBAL)

find_program(HSS_OBJCOPY arm-none-eabi-objcopy)
find_program(HSS_SIZE arm-none-eabi-size)
find_program(HSS_OPENOCD openocd)

set(HSS_OPENOCD_TRANSPORT "swd" CACHE STRING "OpenOCD transport")

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
    hss_get_board_dir(BOARD_DIR "${BOARD_NAME}")
    hss_normalize_target_name(BOARD_TARGET_SUFFIX "${BOARD_NAME}")
    set(BOARD_TARGET "hss_board_${BOARD_TARGET_SUFFIX}")

    if (TARGET "${BOARD_TARGET}")
        set(HSS_ACTIVE_BOARD "${BOARD_NAME}" CACHE INTERNAL "")
        set(HSS_ACTIVE_BOARD_TARGET "${BOARD_TARGET}" CACHE INTERNAL "")
        return()
    endif()

    include("${BOARD_DIR}/board_manifest.cmake")
    if (EXISTS "${BOARD_DIR}/board_roles.cmake")
        include("${BOARD_DIR}/board_roles.cmake")
    endif()

    hss_collect_hal_sources(BOARD_HAL_SOURCES "${BOARD_HAL_DRIVER_DIR}" "${BOARD_CORE_INCLUDE_DIR}")

    add_library("${BOARD_TARGET}" OBJECT
            ${BOARD_STARTUP_SOURCE}
            ${BOARD_CORE_SOURCES}
            ${BOARD_GLUE_SOURCES}
            ${BOARD_HAL_SOURCES}
    )

    target_include_directories("${BOARD_TARGET}" PUBLIC
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
    cmake_parse_arguments(ARG "" "BOARD" "SOURCES" ${ARGN})

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
