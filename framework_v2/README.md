# hss_framework

Minimal STM32 framework base for `stm32f1` and `bluepill_f103`.

## Targets

- `hss_mcu_stm32f1` -> static MCU library
- `hss_board_bluepill_f103` -> static board library (links MCU)
- `hss_framework` -> umbrella interface target selected via functions

This folder contains only framework code. Applications live outside this folder.

## Configuration API

- `hss_select_mcu(<mcu>)`
- `hss_select_board(<board>)`
- `hss_add_firmware(<target> <sources...>)`
- `hss_link_firmware(<existing_target>)`

## Example (external app)

```cmake
add_subdirectory(path/to/framework_v2 ${CMAKE_BINARY_DIR}/hss_framework_build)

hss_select_board(bluepill_f103)
hss_add_firmware(firmware src/main.cpp)
```
