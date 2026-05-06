cmake_minimum_required(VERSION 3.20)

function(hss_v2_select_mcu MCU_NAME)
    set(MCU_PATH ${CMAKE_CURRENT_LIST_DIR}/${MCU_NAME})
    set(MCU_CMAKE ${MCU_PATH}/CMakeLists.txt)

    if (NOT EXISTS ${MCU_CMAKE})
        message(FATAL_ERROR "Unsupported MCU: ${MCU_NAME}")
    endif ()

    if (NOT TARGET hss_mcu_${MCU_NAME})
        add_subdirectory(${MCU_PATH})
    endif ()

    if (NOT TARGET hss_mcu_${MCU_NAME})
        message(FATAL_ERROR "MCU target hss_mcu_${MCU_NAME} was not created by ${MCU_CMAKE}")
    endif ()

    get_target_property(HSS_MCU_FAMILY hss_mcu_${MCU_NAME} HSS_MCU_FAMILY)
    if (NOT HSS_MCU_FAMILY)
        message(FATAL_ERROR "MCU ${MCU_NAME} does not define HSS_MCU_FAMILY")
    endif ()

    hss_v2_select_mcu_family(${HSS_MCU_FAMILY})

    target_link_libraries(hss_mcu INTERFACE
            hss_mcu_${MCU_NAME}
    )

    set_target_properties(hss_mcu PROPERTIES
            HSS_SELECTED_MCU ${MCU_NAME}
            HSS_SELECTED_MCU_FAMILY ${HSS_MCU_FAMILY}
    )
endfunction()
