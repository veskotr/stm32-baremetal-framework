cmake_minimum_required(VERSION 3.20)

set(FRAMEWORK_ROOT ${CMAKE_CURRENT_LIST_DIR} CACHE INTERNAL "")

# Selected board adapter target.
add_library(board INTERFACE)

# Core framework components.
add_subdirectory(core)
add_subdirectory(platform)
add_subdirectory(drivers)
add_subdirectory(middleware)

# Consumer-facing umbrella target.
add_library(hss_framework INTERFACE)
target_link_libraries(hss_framework INTERFACE
        board
        hss_platform
        hss_drivers
        hss_middleware
)

function(hss_select_board BOARD_NAME)
    set(BOARD_PATH ${FRAMEWORK_ROOT}/board/${BOARD_NAME})
    set(BOARD_CMAKE ${BOARD_PATH}/CMakeLists.txt)

    if (NOT EXISTS ${BOARD_CMAKE})
        message(FATAL_ERROR "Unsupported board: ${BOARD_NAME}")
    endif ()

    if (TARGET hss_board_${BOARD_NAME})
        # Board already configured by a previous call.
    else ()
        add_subdirectory(${BOARD_PATH})
    endif ()

    if (NOT TARGET hss_board_${BOARD_NAME})
        message(FATAL_ERROR "Board target hss_board_${BOARD_NAME} was not created by ${BOARD_CMAKE}")
    endif ()

    # Link board target to INTERFACE so all consumers get board headers and defines
    target_link_libraries(board INTERFACE hss_board_${BOARD_NAME})

    # Propagate board include directories through INTERFACE
    get_target_property(BOARD_INCLUDE_DIRS hss_board_${BOARD_NAME} INTERFACE_INCLUDE_DIRECTORIES)
    if (BOARD_INCLUDE_DIRS)
        target_include_directories(board INTERFACE ${BOARD_INCLUDE_DIRS})
    endif ()

    get_target_property(HSS_LINKER_SCRIPT hss_board_${BOARD_NAME} HSS_LINKER_SCRIPT)
    if (NOT HSS_LINKER_SCRIPT)
        message(FATAL_ERROR "Board ${BOARD_NAME} does not define HSS_LINKER_SCRIPT")
    endif ()

    get_target_property(OPENOCD_INTERFACE hss_board_${BOARD_NAME} OPENOCD_INTERFACE)
    get_target_property(OPENOCD_TARGET hss_board_${BOARD_NAME} OPENOCD_TARGET)
    if (NOT OPENOCD_INTERFACE OR NOT OPENOCD_TARGET)
        message(FATAL_ERROR "Board ${BOARD_NAME} does not define OpenOCD config")
    endif ()

    set_target_properties(board PROPERTIES
            HSS_BOARD_NAME ${BOARD_NAME}
            HSS_LINKER_SCRIPT ${HSS_LINKER_SCRIPT}
            OPENOCD_INTERFACE ${OPENOCD_INTERFACE}
            OPENOCD_TARGET ${OPENOCD_TARGET}
    )
    set(OPENOCD_INTERFACE ${OPENOCD_INTERFACE} CACHE STRING "OpenOCD interface config")
    set(OPENOCD_TARGET ${OPENOCD_TARGET} CACHE STRING "OpenOCD target config")
endfunction()

function(hss_add_firmware TARGET_NAME)
    add_executable(${TARGET_NAME}
            ${ARGN}
    )

    # Force embedded output artifact naming to <target>.elf.
    set_target_properties(${TARGET_NAME} PROPERTIES
            OUTPUT_NAME "${TARGET_NAME}"
            SUFFIX ".elf"
    )

    target_link_libraries(${TARGET_NAME} PRIVATE
            hss_framework
    )

    # Pull in board override sources directly so IRQ/MSP strong definitions are not lost in archives.
    get_target_property(BOARD_NAME board HSS_BOARD_NAME)
    if (BOARD_NAME)
        set(BOARD_TARGET hss_board_${BOARD_NAME})
        if (TARGET ${BOARD_TARGET})
            get_target_property(HSS_ALWAYS_LINK_SOURCES ${BOARD_TARGET} HSS_ALWAYS_LINK_SOURCES)
            if (HSS_ALWAYS_LINK_SOURCES)
                target_sources(${TARGET_NAME} PRIVATE ${HSS_ALWAYS_LINK_SOURCES})
            endif ()
        endif ()
    endif ()

    get_target_property(HSS_LINKER_SCRIPT board HSS_LINKER_SCRIPT)
    if (NOT HSS_LINKER_SCRIPT)
        message(FATAL_ERROR "No board selected. Call hss_select_board(<board>) before hss_add_firmware(...)")
    endif ()

    target_compile_options(${TARGET_NAME} PRIVATE
            $<$<CONFIG:Debug>:-Og -g3>
            $<$<CONFIG:Release>:-O2 -DNDEBUG>
            $<$<CONFIG:MinSizeRel>:-Oz -DNDEBUG>
    )

    target_link_options(${TARGET_NAME} PRIVATE
            "SHELL:-T${HSS_LINKER_SCRIPT}"
            -Wl,-Map=${TARGET_NAME}.map
            $<$<CONFIG:Debug>:-g3>
    )

    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND arm-none-eabi-objcopy -O binary $<TARGET_FILE:${TARGET_NAME}> ${TARGET_NAME}.bin
            COMMAND arm-none-eabi-objcopy -O ihex $<TARGET_FILE:${TARGET_NAME}> ${TARGET_NAME}.hex
    )

    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND arm-none-eabi-size --format=berkeley $<TARGET_FILE:${TARGET_NAME}>
    )

endfunction()

function(hss_add_flash_target TARGET_NAME)
    add_custom_target(flash
            COMMAND ${FRAMEWORK_ROOT}/tools/openocd/bin/openocd.exe
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
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endfunction()

function(hss_generate_debug_script TARGET_NAME)

    get_target_property(OPENOCD_INTERFACE board OPENOCD_INTERFACE)
    get_target_property(OPENOCD_TARGET board OPENOCD_TARGET)

    if (NOT OPENOCD_INTERFACE OR NOT OPENOCD_TARGET)
        message(FATAL_ERROR "Board missing OpenOCD config")
    endif()

    set(TOOLS_DIR ${FRAMEWORK_ROOT}/tools/debug)
    file(MAKE_DIRECTORY ${TOOLS_DIR})

    set(SCRIPT_BAT ${TOOLS_DIR}/debug_${TARGET_NAME}.bat)

    file(GENERATE
            OUTPUT ${SCRIPT_BAT}
            CONTENT
            "@echo off

set ELF=$<TARGET_FILE:${TARGET_NAME}>

echo Starting OpenOCD...

start \"\" /MIN openocd ^
 -s ${FRAMEWORK_ROOT}/tools/openocd/scripts ^
 -f ${OPENOCD_INTERFACE} ^
 -f ${OPENOCD_TARGET} ^
 -c \"transport select swd\" ^
 -c \"gdb_port 3333\" ^
 -c \"init\" ^
 -c \"reset halt\""
    )

    message(STATUS "Debug script generated for ${TARGET_NAME}: ${SCRIPT_BAT}")

endfunction()