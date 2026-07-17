include_guard(GLOBAL)

include(CTest)
enable_testing()

add_library(hss_test_unity STATIC
        ${HSS_FRAMEWORK_ROOT}/tests/support/unity/unity.c
)
target_include_directories(hss_test_unity PUBLIC
        ${HSS_FRAMEWORK_ROOT}/tests/support/unity
)
target_compile_features(hss_test_unity PUBLIC c_std_11)

add_library(hss_host_stm32_fakes STATIC
        ${HSS_FRAMEWORK_ROOT}/tests/support/host_stm32_fakes/hss_host_stm32_fakes.c
)
target_include_directories(hss_host_stm32_fakes PUBLIC
        ${HSS_FRAMEWORK_ROOT}/tests/support/host_stm32_fakes
)
target_compile_features(hss_host_stm32_fakes PUBLIC c_std_11)

add_library(hss_hal_host STATIC
        ${HSS_FRAMEWORK_ROOT}/hal/src/exti.c
        ${HSS_FRAMEWORK_ROOT}/hal/src/gpio.c
        ${HSS_FRAMEWORK_ROOT}/hal/src/spi.c
        ${HSS_FRAMEWORK_ROOT}/hal/src/watchdog.c
)
target_include_directories(hss_hal_host PUBLIC
        ${HSS_FRAMEWORK_ROOT}/hal/include
)
target_link_libraries(hss_hal_host PUBLIC
        hss_common
        hss_host_stm32_fakes
)
target_compile_features(hss_hal_host PUBLIC c_std_11)

function(hss_add_host_watchdog_role_library TARGET_NAME ROLE_INCLUDE_DIR)
    add_library("${TARGET_NAME}" STATIC
            ${HSS_FRAMEWORK_ROOT}/board/src/watchdog.c
    )
    target_include_directories("${TARGET_NAME}" PUBLIC
            "${ROLE_INCLUDE_DIR}"
            ${HSS_FRAMEWORK_ROOT}/hal/include
            ${HSS_FRAMEWORK_ROOT}/board/include
            ${HSS_FRAMEWORK_ROOT}/tests/support/host_stm32_fakes
    )
    target_link_libraries("${TARGET_NAME}" PUBLIC
            hss_hal_host
    )
    target_compile_features("${TARGET_NAME}" PUBLIC c_std_11)
endfunction()

hss_add_host_watchdog_role_library(
        hss_board_host_watchdog_none
        ${HSS_FRAMEWORK_ROOT}/tests/support/host_stm32_fakes
)

hss_add_host_watchdog_role_library(
        hss_board_host_watchdog_iwdg
        ${HSS_FRAMEWORK_ROOT}/tests/support/host_stm32_fakes/iwdg_board_roles
)

hss_add_host_watchdog_role_library(
        hss_board_host_watchdog_wwdg
        ${HSS_FRAMEWORK_ROOT}/tests/support/host_stm32_fakes/wwdg_board_roles
)

function(hss_add_host_test TARGET_NAME)
    cmake_parse_arguments(ARG
            ""
            ""
            "SOURCES;LIBRARIES;INCLUDE_DIRS;DEFINITIONS"
            ${ARGN}
    )

    set(TEST_SOURCES ${ARG_SOURCES} ${ARG_UNPARSED_ARGUMENTS})
    if(NOT TEST_SOURCES)
        message(FATAL_ERROR "hss_add_host_test(${TARGET_NAME}) requires at least one source file")
    endif()

    add_executable("${TARGET_NAME}" ${TEST_SOURCES})
    target_link_libraries("${TARGET_NAME}" PRIVATE
            hss_test_unity
            hss_common
            ${ARG_LIBRARIES}
    )
    target_include_directories("${TARGET_NAME}" PRIVATE ${ARG_INCLUDE_DIRS})
    target_compile_definitions("${TARGET_NAME}" PRIVATE ${ARG_DEFINITIONS})
    target_compile_features("${TARGET_NAME}" PRIVATE c_std_11)

    add_test(NAME "${TARGET_NAME}" COMMAND "${TARGET_NAME}")
    set_tests_properties("${TARGET_NAME}" PROPERTIES LABELS "unit")
endfunction()

function(hss_add_host_integration_test TARGET_NAME)
    cmake_parse_arguments(ARG
            ""
            ""
            "SOURCES;LIBRARIES;INCLUDE_DIRS;DEFINITIONS"
            ${ARGN}
    )

    hss_add_host_test("${TARGET_NAME}"
            SOURCES ${ARG_SOURCES} ${ARG_UNPARSED_ARGUMENTS}
            LIBRARIES ${ARG_LIBRARIES}
            INCLUDE_DIRS ${ARG_INCLUDE_DIRS}
            DEFINITIONS ${ARG_DEFINITIONS}
    )
    set_tests_properties("${TARGET_NAME}" PROPERTIES LABELS "integration")
endfunction()
