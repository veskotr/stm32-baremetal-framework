cmake_minimum_required(VERSION 3.20)

add_library(hss_max31865_driver INTERFACE)

target_link_libraries(hss_max31865_driver INTERFACE
        hss_adi_max31865
)