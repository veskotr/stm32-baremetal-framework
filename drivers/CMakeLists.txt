option(HSS_ENABLE_MAX31865 "Build MAX31865 driver support" OFF)

if(HSS_ENABLE_MAX31865)
    add_subdirectory(max31865)
endif()
