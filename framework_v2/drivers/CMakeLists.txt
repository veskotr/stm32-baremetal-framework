cmake_minimum_required(VERSION 3.20)

add_library(hss_drivers INTERFACE)

target_link_libraries(hss_drivers INTERFACE
    hss_board
)

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/analog_devices_no_os/CMakeLists.txt)
    add_subdirectory(analog_devices_no_os)
    if (TARGET hss_adi_max31865)
        target_link_libraries(hss_drivers INTERFACE hss_adi_max31865)
    endif ()
endif ()

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/max31865/CMakeLists.txt)
    add_subdirectory(max31865)
    if (TARGET hss_max31865_driver)
        target_link_libraries(hss_drivers INTERFACE hss_max31865_driver)
    endif ()
endif ()
