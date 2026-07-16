---
name: hss-verify-change
description: Select and run the appropriate HSS verification matrix for a code, CMake, generator, board, documentation, or public API change. Use when validating implementation work, preparing a handoff or review, investigating CI-equivalent coverage, or deciding whether host tests, Python tests, ARM builds, or hardware checks are required.
---

# Verify an HSS change

1. Read `AGENTS.md` and inspect `git diff --stat`, `git diff`, and untracked files without assuming every worktree change belongs to the current task.
2. Map changed paths to lanes:
   - `tools/config/` or config generation: Python unittest lane;
   - C logic, `hal/`, `common/`, drivers, protocols, fakes, or `tests/`: host configure/build/CTest lane;
   - `cmake/`, board sync/generation, public headers, HAL behavior, board roles, drivers, or protocols: both reference-family ARM builds;
   - documentation-only: link/path and consistency checks; do not run compilers without a concrete reason.
3. Prefer focused tests while iterating, then run the complete affected lane before declaring success.
4. Use the canonical commands in `AGENTS.md`. Reconfigure when CMake inputs, generated manifests, source lists, compile definitions, or toolchain inputs changed.
5. Do not delete or reset build directories you do not own. A fresh directory under `/tmp` is acceptable when cached state could hide a problem.
6. Inspect failures before retrying. Distinguish product defects, missing local dependencies, stale configuration, and unavailable hardware.
7. For board-generator changes, confirm a second identical run produces no content diff.
8. Report:
   - exact commands run;
   - pass/fail counts or build targets;
   - lanes skipped and why;
   - whether hardware was actually flashed and exercised.

Host tests validate deterministic logic and fake HAL interactions. ARM builds validate compilation and linkage for representative families. Neither proves clocks, pins, interrupts, timing, or peripherals work on physical hardware.
