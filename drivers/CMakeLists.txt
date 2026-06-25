add_library(hss_drivers INTERFACE)

target_link_libraries(hss_drivers INTERFACE
        hss_common
        hss_hal
)

target_include_directories(hss_drivers INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/include
)

option(HSS_ENABLE_MAX31865 "Build MAX31865 driver support" ON)

if(HSS_ENABLE_MAX31865)
    add_subdirectory(max31865)
    target_link_libraries(hss_drivers INTERFACE hss_max31865)
endif()
