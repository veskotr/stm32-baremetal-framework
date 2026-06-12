# Examples

Examples are small firmware projects that show how to consume the framework.

They live in this repository for learning and verification, but they should keep the same shape as external application repositories:

- each example has its own `CMakeLists.txt`
- each example calls `add_subdirectory(../.. framework_v3_build)`
- app source stays under the example's `src/` directory
- framework code stays outside the app source

Build an example from this repository:

```sh
cmake -S framework_v3/examples/blue_pill_minimal \
  -B build/examples/blue_pill_minimal \
  -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=framework_v3/cmake/arm-gcc-toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Debug
cmake --build build/examples/blue_pill_minimal
```

Examples are allowed to be practical and repetitive. Their job is to teach the normal workflow clearly.
