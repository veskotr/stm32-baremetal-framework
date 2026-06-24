add_subdirectory(max31865)

add_library(hss_drivers INTERFACE)

target_link_libraries(hss_drivers INTERFACE
        hss_common
        hss_hal
        hss_max31865
)

target_include_directories(hss_drivers INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/include
)
