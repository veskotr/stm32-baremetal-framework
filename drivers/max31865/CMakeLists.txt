add_library(hss_max31865 STATIC
        src/max31865.c
)
set_target_properties(hss_max31865 PROPERTIES EXCLUDE_FROM_ALL TRUE)

target_include_directories(hss_max31865 PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(hss_max31865 PUBLIC
        hss_common
        hss_hal
)
