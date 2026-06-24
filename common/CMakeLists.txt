add_library(hss_common INTERFACE)

set(HSS_COMMON_GENERATED_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated/include")
configure_file(
        ${CMAKE_CURRENT_LIST_DIR}/include/hss_version.h.in
        ${HSS_COMMON_GENERATED_INCLUDE_DIR}/hss_version.h
        @ONLY
)

target_include_directories(hss_common INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${HSS_COMMON_GENERATED_INCLUDE_DIR}
)
