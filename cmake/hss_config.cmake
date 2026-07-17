include_guard(GLOBAL)

function(hss_enable_freemodbus OUT_TARGET)
    if (TARGET hss_freemodbus)
        set(${OUT_TARGET} hss_freemodbus PARENT_SCOPE)
        return()
    endif()
    add_subdirectory(
            "${HSS_FRAMEWORK_ROOT}/protocols/freemodbus"
            "${CMAKE_BINARY_DIR}/hss_generated/protocols/freemodbus"
    )
    set(${OUT_TARGET} hss_freemodbus PARENT_SCOPE)
endfunction()

function(hss_enable_max31865 OUT_TARGET)
    if (TARGET hss_max31865)
        set(${OUT_TARGET} hss_max31865 PARENT_SCOPE)
        return()
    endif()
    add_subdirectory(
            "${HSS_FRAMEWORK_ROOT}/drivers/max31865"
            "${CMAKE_BINARY_DIR}/hss_generated/drivers/max31865"
    )
    set(${OUT_TARGET} hss_max31865 PARENT_SCOPE)
endfunction()

function(hss_generate_target_config TARGET_NAME CONFIG_FILE OUT_INCLUDE_DIR OUT_FRAMEWORK_LIBRARIES)
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

    set(HSS_TARGET_FRAMEWORK_LIBRARIES "")
    if (DEFINED HSS_CONFIG_VALUE_HSS_ENABLE_FREEMODBUS
            AND HSS_CONFIG_VALUE_HSS_ENABLE_FREEMODBUS)
        hss_enable_freemodbus(HSS_FREEMODBUS_TARGET)
        list(APPEND HSS_TARGET_FRAMEWORK_LIBRARIES ${HSS_FREEMODBUS_TARGET})
    endif()
    if (DEFINED HSS_CONFIG_VALUE_HSS_ENABLE_MAX31865
            AND HSS_CONFIG_VALUE_HSS_ENABLE_MAX31865)
        hss_enable_max31865(HSS_MAX31865_TARGET)
        list(APPEND HSS_TARGET_FRAMEWORK_LIBRARIES ${HSS_MAX31865_TARGET})
    endif()

    if (HSS_CONFIG_FRAMEWORK_COMPILE_DEFINITIONS)
        if (TARGET hss_board)
            target_compile_definitions(hss_board PUBLIC ${HSS_CONFIG_FRAMEWORK_COMPILE_DEFINITIONS})
        endif()
        foreach(HSS_FRAMEWORK_LIBRARY IN LISTS HSS_TARGET_FRAMEWORK_LIBRARIES)
            target_compile_definitions(${HSS_FRAMEWORK_LIBRARY} PUBLIC ${HSS_CONFIG_FRAMEWORK_COMPILE_DEFINITIONS})
        endforeach()
    endif()

    get_cmake_property(HSS_CONFIG_VARIABLES VARIABLES)
    foreach(HSS_CONFIG_VARIABLE IN LISTS HSS_CONFIG_VARIABLES)
        if (HSS_CONFIG_VARIABLE MATCHES "^HSS_CONFIG_VALUE_")
            set(${HSS_CONFIG_VARIABLE} "${${HSS_CONFIG_VARIABLE}}" PARENT_SCOPE)
        endif()
    endforeach()

    set(${OUT_INCLUDE_DIR} "${CONFIG_OUTPUT_DIR}" PARENT_SCOPE)
    set(${OUT_FRAMEWORK_LIBRARIES} "${HSS_TARGET_FRAMEWORK_LIBRARIES}" PARENT_SCOPE)
    set(HSS_CONFIG_COMPILE_DEFINITIONS "${HSS_CONFIG_COMPILE_DEFINITIONS}" PARENT_SCOPE)
    set(HSS_CONFIG_ACTIVE_PROFILES "${HSS_CONFIG_ACTIVE_PROFILES}" PARENT_SCOPE)
endfunction()
