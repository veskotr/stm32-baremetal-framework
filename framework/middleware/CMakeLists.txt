add_subdirectory(freemodbus)
add_subdirectory(modbus)

add_library(hss_middleware INTERFACE)

target_link_libraries(hss_middleware
        INTERFACE hss_modbus
)