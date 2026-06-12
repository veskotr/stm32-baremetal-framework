# Framework Tools

This folder contains host-side tools for setting up and operating the framework.

Current tools:

- `sync_board.py`: compatibility entrypoint for board sync
- `board_sync/`: CubeMX board inspection and generated framework glue

Planned tools:

- VS Code task generation
- VS Code launch configuration generation
- build, flash, debug helper generation
- report and artifact summary helpers

Keep command entrypoints small. Put reusable logic in tool-specific packages so workflows can be called from the shell, CMake, or generated VS Code tasks.
