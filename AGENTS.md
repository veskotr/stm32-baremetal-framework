# HSS STM32 Framework Agent Guide

## Start here

Read [docs/INDEX.md](docs/INDEX.md) before broad changes. Use it to load only the documents relevant to the task. Treat [docs/development_rules.md](docs/development_rules.md) and accepted records under [docs/decisions/](docs/decisions/) as constraints, not background suggestions.

Before editing, inspect `git status --short`. The worktree may contain in-progress user changes; preserve and work around them. Do not edit CubeMX-owned files under a board's `Core/` or `Drivers/` directories. Do not hand-edit generated board manifests, generated headers, or generated glue.

## Repository shape

- `common/`: dependency-light shared types and utilities.
- `hal/`: thin STM32 HAL-facing helpers; HAL types may remain visible.
- `drivers/`: reusable device drivers.
- `protocols/`: optional protocol integrations.
- `cmake/`: authoritative framework build behavior.
- `tools/`: idempotent generators and board/config tooling.
- `examples/`: external-consumer examples and temporary reference boards.
- `tests/`: native unit/integration tests and host STM32 fakes.
- `docs/`: architecture, workflows, decisions, and current project direction.

## Working rules

- Keep the framework STM32-only, HAL-first, C11-first, and simple.
- Prefer generic board capabilities over product-specific framework semantics.
- Keep reusable behavior out of applications and board-specific behavior out of shared modules.
- Make dependencies optional unless they are fundamental to the framework.
- Extend host fakes when HAL-facing behavior needs deterministic coverage.
- Update usage or architecture docs in the same change that changes them.
- Record a decision when a change establishes a durable constraint, crosses module boundaries, or reverses an accepted direction. Use `docs/decisions/0000-template.md` and update `docs/decisions/README.md`.

## Verification

Choose the smallest relevant set, then expand when shared CMake, generation, or public behavior changes.

```sh
python3 -m unittest discover tools/config/tests
cmake -S . -B build/host-tests -G Ninja -DHSS_ENABLE_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build/host-tests
ctest --test-dir build/host-tests --output-on-failure
```

For firmware-facing or shared CMake changes, also configure and build both reference families:

```sh
cmake -S examples/blue_pill_minimal -B build/examples/blue_pill_minimal -G Ninja -DCMAKE_TOOLCHAIN_FILE="$PWD/cmake/arm-gcc-toolchain.cmake" -DCMAKE_BUILD_TYPE=Debug
cmake --build build/examples/blue_pill_minimal
cmake -S examples/stm32g0_minimal -B build/examples/stm32g0_minimal -G Ninja -DCMAKE_TOOLCHAIN_FILE="$PWD/cmake/arm-gcc-toolchain.cmake" -DCMAKE_BUILD_TYPE=Debug
cmake --build build/examples/stm32g0_minimal
```

Never claim hardware validation unless the change was actually flashed and exercised on hardware. Report skipped checks and the reason.

## Skills and agents

Use the matching repository skill when the task concerns HAL feature work, board integration, or verification. For broad work with genuinely independent discovery, review, or verification, delegate to the project custom agents when available. Keep small or tightly coupled tasks in one thread. Delegated agents must preserve user changes and return evidence, not assumptions.

## Definition of done

A change is done when its implementation, focused tests, affected documentation, and decision record (when needed) agree; generated ownership boundaries remain intact; and the final report lists commands run, results, and any unverified hardware behavior.
