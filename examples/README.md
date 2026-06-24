# Examples

Current examples:

- `hello_blink`: copyable starter project for platform init and status LED blink. It uses the Blue Pill reference board, but the shape is intended for new application repositories.
- `blue_pill_minimal`: platform init, console, and status LED smoke test for Blue Pill. This target enables FreeModbus at build time so the protocol target keeps compiling while the framework evolves.
- `blue_pill_modbus_slave`: first HSS Modbus API example for Blue Pill. It exposes simple holding/input register arrays through `hss_modbus_*` on the board Modbus UART/timer roles.
- `stm32g0_minimal`: platform init smoke test for the STM32G0 board.

Examples are small firmware projects that show how to consume the framework.

The boards used by these examples live in `examples/boards/`. They are temporary reference inputs for learning and verification; production applications should keep boards in the application repository and set `HSS_BOARD_PATHS` to that location.

They live in this repository for learning and verification, but they should keep the same shape as external application repositories:

- each example has its own `CMakeLists.txt`
- each example calls `add_subdirectory(../.. hss_framework_build)`
- each example sets `HSS_BOARD_PATHS` to `../boards`
- app source stays under the example's `src/` directory
- framework code stays outside the app source

Build an example from this repository:

```sh
cmake -S examples/blue_pill_minimal \
  -B build/examples/blue_pill_minimal \
  -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=$PWD/cmake/arm-gcc-toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Debug
cmake --build build/examples/blue_pill_minimal
```

Examples are allowed to be practical and repetitive. Their job is to teach the normal workflow clearly.
