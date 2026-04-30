set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)

set(CMAKE_EXECUTABLE_SUFFIX ".elf")

# Standard C/C++ libraries - CMake appends these AFTER all user libraries,
# which is the correct order for static library symbol resolution.
set(CMAKE_C_STANDARD_LIBRARIES "")
set(CMAKE_CXX_STANDARD_LIBRARIES "")
set(CMAKE_EXE_LINKER_FLAGS "")

set(CMAKE_C_FLAGS_INIT "-ffunction-sections -fdata-sections")
# -specs=nosys.specs injects syscall stubs (weak) at the right point in
# GCC's link sequence so that _fstat, _isatty, _kill etc. are always satisfied.
set(CMAKE_EXE_LINKER_FLAGS_INIT "-Wl,--gc-sections -specs=nosys.specs")
