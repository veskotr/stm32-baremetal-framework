function(hss_select_mcu MCU_NAME)
    if (NOT TARGET hss_mcu_${MCU_NAME})
        message(FATAL_ERROR "Unsupported MCU: ${MCU_NAME}")
    endif ()

    target_link_libraries(hss_mcu INTERFACE hss_mcu_${MCU_NAME})

    set_target_properties(hss_mcu PROPERTIES
        HSS_SELECTED_MCU ${MCU_NAME}
    )
endfunction()

function(hss_select_board BOARD_NAME)
    if (NOT TARGET hss_board_${BOARD_NAME})
        message(FATAL_ERROR "Unsupported board: ${BOARD_NAME}")
    endif ()

    get_target_property(BOARD_MCU hss_board_${BOARD_NAME} HSS_MCU)
    if (NOT BOARD_MCU)
        message(FATAL_ERROR "Board ${BOARD_NAME} does not define HSS_MCU")
    endif ()

    get_target_property(SELECTED_MCU hss_mcu HSS_SELECTED_MCU)
    if (SELECTED_MCU)
        if (NOT SELECTED_MCU STREQUAL BOARD_MCU)
            message(FATAL_ERROR "Selected MCU ${SELECTED_MCU} does not match board ${BOARD_NAME} MCU ${BOARD_MCU}")
        endif ()
    else ()
        hss_select_mcu(${BOARD_MCU})
    endif ()

    target_link_libraries(hss_board INTERFACE hss_board_${BOARD_NAME})

    get_target_property(BOARD_LINKER_SCRIPT hss_board_${BOARD_NAME} HSS_LINKER_SCRIPT)
    set_target_properties(hss_board PROPERTIES
        HSS_BOARD_NAME ${BOARD_NAME}
        HSS_MCU ${BOARD_MCU}
        HSS_LINKER_SCRIPT ${BOARD_LINKER_SCRIPT}
    )
endfunction()

function(hss_link_firmware TARGET_NAME)
    if (NOT TARGET ${TARGET_NAME})
        message(FATAL_ERROR "Target ${TARGET_NAME} does not exist")
    endif ()

    get_target_property(BOARD_NAME hss_board HSS_BOARD_NAME)
    if (NOT BOARD_NAME)
        message(FATAL_ERROR "No board selected. Call hss_select_board(<board>) first.")
    endif ()

    target_link_libraries(${TARGET_NAME} PRIVATE hss_framework)

    set_target_properties(${TARGET_NAME} PROPERTIES
        OUTPUT_NAME ${TARGET_NAME}
        SUFFIX ".elf"
    )
endfunction()

function(hss_add_firmware TARGET_NAME)
    add_executable(${TARGET_NAME} ${ARGN})
    hss_link_firmware(${TARGET_NAME})
endfunction()
