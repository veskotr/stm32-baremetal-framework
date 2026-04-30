cmake_minimum_required(VERSION 3.20)

add_library(hss_core INTERFACE)

target_include_directories(hss_core INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)