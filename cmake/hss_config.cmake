include_guard(GLOBAL)

function(hss_enable_freemodbus)
    if (TARGET hss_freemodbus)
        return()
    endif()
    add_subdirectory(
            "${HSS_FRAMEWORK_ROOT}/protocols/freemodbus"
            "${CMAKE_BINARY_DIR}/hss_generated/protocols/freemodbus"
    )
    target_link_libraries(hss_protocols INTERFACE hss_freemodbus)
endfunction()

function(hss_enable_max31865)
    if (TARGET hss_max31865)
        target_link_libraries(hss_drivers INTERFACE hss_max31865)
        return()
    endif()
    add_subdirectory(
            "${HSS_FRAMEWORK_ROOT}/drivers/max31865"
            "${CMAKE_BINARY_DIR}/hss_generated/drivers/max31865"
    )
    target_link_libraries(hss_drivers INTERFACE hss_max31865)
endfunction()

function(hss_generate_target_config TARGET_NAME CONFIG_FILE OUT_INCLUDE_DIR)
    cmake_parse_arguments(ARG "" "" "PROFILES;OPTIONAL_PROFILES;PROFILE_FILES" ${ARGN})

    find_package(Python3 COMPONENTS Interpreter QUIET)
    if (NOT Python3_Interpreter_FOUND)
        message(FATAL_ERROR "Python3 is required to generate HSS config for ${TARGET_NAME}")
    endif()

    if (IS_ABSOLUTE "${CONFIG_FILE}")
        set(CONFIG_PATH "${CONFIG_FILE}")
    else()
        get_filename_component(CONFIG_PATH "${CONFIG_FILE}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
    endif()

    set(CONFIG_OUTPUT_DIR "${CMAKE_BINARY_DIR}/hss_generated/${TARGET_NAME}")
    set(PROFILE_ARGS "")
    foreach(PROFILE IN LISTS ARG_PROFILES)
        if (NOT PROFILE STREQUAL "")
            list(APPEND PROFILE_ARGS "--profile=${PROFILE}")
        endif()
    endforeach()
    foreach(PROFILE IN LISTS ARG_OPTIONAL_PROFILES)
        if (NOT PROFILE STREQUAL "")
            list(APPEND PROFILE_ARGS "--optional-profile=${PROFILE}")
        endif()
    endforeach()
    foreach(PROFILE_FILE IN LISTS ARG_PROFILE_FILES)
        if (NOT PROFILE_FILE STREQUAL "")
            list(APPEND PROFILE_ARGS "--profile-file=${PROFILE_FILE}")
        endif()
    endforeach()

    execute_process(
            COMMAND "${Python3_EXECUTABLE}" "${HSS_FRAMEWORK_ROOT}/tools/generate_config.py"
            "--config=${CONFIG_PATH}"
            "--out-dir=${CONFIG_OUTPUT_DIR}"
            "--target=${TARGET_NAME}"
            ${PROFILE_ARGS}
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            RESULT_VARIABLE HSS_CONFIG_RESULT
    )

    if (NOT HSS_CONFIG_RESULT EQUAL 0)
        message(FATAL_ERROR "HSS config generation failed for ${TARGET_NAME}")
    endif()

    include("${CONFIG_OUTPUT_DIR}/hss_config.cmake")

    if (DEFINED HSS_CONFIG_VALUE_HSS_ENABLE_FREEMODBUS
            AND HSS_CONFIG_VALUE_HSS_ENABLE_FREEMODBUS)
        hss_enable_freemodbus()
    endif()
    if (DEFINED HSS_CONFIG_VALUE_HSS_ENABLE_MAX31865
            AND HSS_CONFIG_VALUE_HSS_ENABLE_MAX31865)
        hss_enable_max31865()
    endif()

    if (HSS_CONFIG_FRAMEWORK_COMPILE_DEFINITIONS)
        if (TARGET hss_hal)
            target_compile_definitions(hss_hal PUBLIC ${HSS_CONFIG_FRAMEWORK_COMPILE_DEFINITIONS})
        endif()
        if (TARGET hss_freemodbus)
            target_compile_definitions(hss_freemodbus PUBLIC ${HSS_CONFIG_FRAMEWORK_COMPILE_DEFINITIONS})
        endif()
    endif()

    set(${OUT_INCLUDE_DIR} "${CONFIG_OUTPUT_DIR}" PARENT_SCOPE)
    set(HSS_CONFIG_COMPILE_DEFINITIONS "${HSS_CONFIG_COMPILE_DEFINITIONS}" PARENT_SCOPE)
    set(HSS_CONFIG_ACTIVE_PROFILES "${HSS_CONFIG_ACTIVE_PROFILES}" PARENT_SCOPE)
endfunction()
