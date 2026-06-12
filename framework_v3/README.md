# framework_v3

`framework_v3` is the next iteration of this STM32 framework.

Its goal is to provide a simple, stable, STM32 HAL-focused foundation that sits between:
- STM32CubeMX-generated board code
- reusable framework helpers
- protocol and driver integrations
- the application layer

This version is intentionally not a generic hardware abstraction framework.
It targets STM32 MCUs that use the HAL, accepts some HAL leakage where it keeps things simpler, and focuses on reuse, board portability inside the STM32 family, and a clean developer workflow.

Start with the docs in [`docs/`](./docs/):
- `architecture.md`
- `development_rules.md`
- `cubemx_workflow.md`
- `external_project.md`
- `testing.md`
- `handoff.md`

Example firmware projects live in [`examples/`](./examples/).
