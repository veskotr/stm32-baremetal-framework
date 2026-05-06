cmake_minimum_required(VERSION 3.20)

set(FRAMEWORK_V2_ROOT ${CMAKE_CURRENT_LIST_DIR} CACHE INTERNAL "")

add_library(hss_mcu_family INTERFACE)
add_library(hss_mcu INTERFACE)
add_library(hss_board INTERFACE)

add_subdirectory(mcu_family)
add_subdirectory(mcu)
add_subdirectory(platform)
add_subdirectory(board)
add_subdirectory(drivers)
add_subdirectory(middleware)

add_library(hss_framework_v2 INTERFACE)
target_link_libraries(hss_framework_v2 INTERFACE
        hss_board
        hss_drivers
        hss_middleware
)

function(hss_v2_add_firmware TARGET_NAME)
    add_executable(${TARGET_NAME}
            ${ARGN}
    )

    set_target_properties(${TARGET_NAME} PROPERTIES
            OUTPUT_NAME "${TARGET_NAME}"
            SUFFIX ".elf"
    )

    target_link_libraries(${TARGET_NAME} PRIVATE
            hss_framework_v2
    )
endfunction()
