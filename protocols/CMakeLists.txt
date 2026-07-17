option(HSS_ENABLE_FREEMODBUS "Build FreeModbus protocol support" OFF)

if(HSS_ENABLE_FREEMODBUS)
    add_subdirectory(freemodbus)
endif()
