cmake_minimum_required(VERSION 3.20)

function(hss_v2_select_mcu_family MCU_FAMILY)
    set(FAMILY_PATH ${CMAKE_CURRENT_LIST_DIR}/${MCU_FAMILY})
    set(FAMILY_CMAKE ${FAMILY_PATH}/CMakeLists.txt)

    if (NOT EXISTS ${FAMILY_CMAKE})
        message(FATAL_ERROR "Unsupported MCU family: ${MCU_FAMILY}")
    endif ()

    if (NOT TARGET hss_mcu_family_${MCU_FAMILY})
        add_subdirectory(${FAMILY_PATH})
    endif ()

    if (NOT TARGET hss_mcu_family_${MCU_FAMILY})
        message(FATAL_ERROR "MCU family target hss_mcu_family_${MCU_FAMILY} was not created by ${FAMILY_CMAKE}")
    endif ()

    target_link_libraries(hss_mcu_family INTERFACE hss_mcu_family_${MCU_FAMILY})

    set_target_properties(hss_mcu_family PROPERTIES
            HSS_SELECTED_MCU_FAMILY ${MCU_FAMILY}
    )
endfunction()
