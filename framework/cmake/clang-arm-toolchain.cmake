set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# === Compilers ===
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_ASM_COMPILER clang)

# Tell clang we target ARM
set(TARGET_TRIPLE arm-none-eabi)

set(CMAKE_C_COMPILER_TARGET ${TARGET_TRIPLE})
set(CMAKE_CXX_COMPILER_TARGET ${TARGET_TRIPLE})
set(CMAKE_ASM_COMPILER_TARGET ${TARGET_TRIPLE})

# Prevent try-compile issues
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# === Common flags ===
set(COMMON_FLAGS
    -ffunction-sections
    -fdata-sections
    -fno-common
    -Wl,--gc-sections
)

set(CMAKE_C_FLAGS "${COMMON_FLAGS}" CACHE INTERNAL "")
set(CMAKE_CXX_FLAGS "${COMMON_FLAGS}" CACHE INTERNAL "")

# === Linker ===
set(CMAKE_LINKER ld.lld)

# Use LLD via clang
set(CMAKE_EXE_LINKER_FLAGS
    "-fuse-ld=lld -Wl,--gc-sections -Wl,--print-memory-usage"
    CACHE INTERNAL ""
)