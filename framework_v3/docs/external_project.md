# External Project Usage

`framework_v3` is intended to be consumed by application repositories.

An application should be able to add this framework as a submodule or copied dependency and keep app code outside the framework tree.

Minimal external project shape:

```text
my_app/
  CMakeLists.txt
  src/main.c
  third_party/framework_v3/
```

Minimal `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.20)

project(my_app C ASM)

add_subdirectory(third_party/framework_v3)

hss_select_board(blue_pill_temp_transmitter)
hss_add_firmware(my_app src/main.c)
```

Configure with the framework toolchain file:

```sh
cmake -S . -B build/debug -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=third_party/framework_v3/cmake/arm-gcc-toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Debug
cmake --build build/debug
```

Generated build outputs:

- `my_app.elf`
- `my_app.bin`
- `my_app.hex`
- `my_app.map`

Generated targets:

- `my_app`
- `flash_my_app`
- `openocd_my_app`
- `sync_board_<board>`

On Linux the framework uses the system `openocd` found in `PATH`.
The bundled Windows OpenOCD tools from older framework versions are not used by this CMake scaffold.

For copyable starter projects, see `framework_v3/examples/`.

## Current Verified Examples

These examples are currently expected to build:

- `framework_v3/examples/blue_pill_minimal`
- `framework_v3/examples/stm32g0_minimal`

Build commands:

```sh
cmake -S framework_v3/examples/blue_pill_minimal \
  -B build/examples/blue_pill_minimal \
  -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=framework_v3/cmake/arm-gcc-toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Debug
cmake --build build/examples/blue_pill_minimal
```

```sh
cmake -S framework_v3/examples/stm32g0_minimal \
  -B build/examples/stm32g0_minimal \
  -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=framework_v3/cmake/arm-gcc-toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Debug
cmake --build build/examples/stm32g0_minimal
```
