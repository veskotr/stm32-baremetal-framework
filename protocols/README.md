# Protocols

Reusable protocol integrations live here. Each protocol is an opt-in, named
CMake target with explicit dependencies.

`hss_freemodbus` also provides `hss_modbus_codec.h` for Modbus register-pair
encoding and decoding of 32-bit integer and float values. The codec is
board-independent; applications still own their register maps and semantics.

Planned initial protocol integration:

- FreeModbus STM32 port
