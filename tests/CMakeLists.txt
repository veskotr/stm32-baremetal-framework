hss_add_host_test(hss_result_tests
        SOURCES
        unit/test_hss_result.c
)

hss_add_host_test(hss_modbus_codec_tests
        SOURCES
        unit/test_hss_modbus_codec.c
        ${HSS_FRAMEWORK_ROOT}/protocols/freemodbus/src/modbus_codec.c
        INCLUDE_DIRS
        ${HSS_FRAMEWORK_ROOT}/protocols/freemodbus/include
)

hss_add_host_integration_test(hss_gpio_host_tests
        SOURCES
        integration/test_hss_gpio_host.c
        LIBRARIES
        hss_hal_host
)

hss_add_host_integration_test(hss_exti_host_tests
        SOURCES
        integration/test_hss_exti_host.c
        LIBRARIES
        hss_hal_host
)

hss_add_host_integration_test(hss_spi_host_tests
        SOURCES
        integration/test_hss_spi_host.c
        LIBRARIES
        hss_hal_host
)

hss_add_host_integration_test(hss_watchdog_host_tests
        SOURCES
        integration/test_hss_watchdog_host.c
        LIBRARIES
        hss_hal_host
        hss_board_host_watchdog_none
)

hss_add_host_integration_test(hss_watchdog_iwdg_role_host_tests
        SOURCES
        integration/test_hss_watchdog_iwdg_role_host.c
        LIBRARIES
        hss_board_host_watchdog_iwdg
)

hss_add_host_integration_test(hss_watchdog_wwdg_role_host_tests
        SOURCES
        integration/test_hss_watchdog_wwdg_role_host.c
        LIBRARIES
        hss_board_host_watchdog_wwdg
)
