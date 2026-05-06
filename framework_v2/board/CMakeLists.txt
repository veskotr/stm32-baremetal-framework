cmake_minimum_required(VERSION 3.20)

function(hss_v2_select_board BOARD_NAME)
    set(BOARD_PATH ${CMAKE_CURRENT_LIST_DIR}/${BOARD_NAME})
    set(BOARD_CMAKE ${BOARD_PATH}/CMakeLists.txt)

    if (NOT EXISTS ${BOARD_CMAKE})
        message(FATAL_ERROR "Unsupported board: ${BOARD_NAME}")
    endif ()

    if (NOT TARGET hss_board_${BOARD_NAME})
        add_subdirectory(${BOARD_PATH})
    endif ()

    if (NOT TARGET hss_board_${BOARD_NAME})
        message(FATAL_ERROR "Board target hss_board_${BOARD_NAME} was not created by ${BOARD_CMAKE}")
    endif ()

    get_target_property(HSS_MCU hss_board_${BOARD_NAME} HSS_MCU)
    if (NOT HSS_MCU)
        message(FATAL_ERROR "Board ${BOARD_NAME} does not define HSS_MCU")
    endif ()
    hss_v2_select_mcu(${HSS_MCU})

    target_link_libraries(hss_board INTERFACE hss_board_${BOARD_NAME})

    set_target_properties(hss_board PROPERTIES
            HSS_BOARD_NAME ${BOARD_NAME}
            HSS_MCU ${HSS_MCU}
    )
endfunction()
