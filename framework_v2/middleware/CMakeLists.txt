cmake_minimum_required(VERSION 3.20)

add_library(hss_middleware INTERFACE)

target_link_libraries(hss_middleware INTERFACE
    hss_board
)

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/freemodbus/CMakeLists.txt)
    add_subdirectory(freemodbus)
endif ()

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/modbus/CMakeLists.txt)
    add_subdirectory(modbus)
    if (TARGET hss_modbus)
        target_link_libraries(hss_middleware INTERFACE hss_modbus)
    endif ()
endif ()
