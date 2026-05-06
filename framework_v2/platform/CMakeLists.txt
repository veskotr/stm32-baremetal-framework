cmake_minimum_required(VERSION 3.20)

add_library(hss_platform INTERFACE)

target_link_libraries(hss_platform INTERFACE
        hss_mcu
)
