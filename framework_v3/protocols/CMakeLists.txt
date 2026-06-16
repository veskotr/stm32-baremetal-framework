add_library(hss_protocols INTERFACE)

target_link_libraries(hss_protocols INTERFACE
        hss_common
        hss_hal
)

option(HSS_ENABLE_FREEMODBUS "Build FreeModbus protocol support" OFF)

if(HSS_ENABLE_FREEMODBUS)
    add_subdirectory(freemodbus)
    target_link_libraries(hss_protocols INTERFACE hss_freemodbus)
endif()
