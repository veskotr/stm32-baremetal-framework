add_library(hss_drivers INTERFACE)

target_link_libraries(hss_drivers INTERFACE
        hss_common
        hss_hal
)

