# Framework Tools

This folder contains host-side tools for setting up and operating the framework.

Current tools:

- `sync_board.py`: compatibility entrypoint for board sync
- `board_sync/`: CubeMX board inspection and generated framework glue
- `generate_config.py`: compatibility entrypoint for target config generation
- `config/`: profile-aware HSS config parsing, schema/defaults, and generated config rendering
- `generate_vscode.py`: compatibility entrypoint for VS Code workflow generation
- `vscode/`: generated VS Code tasks and Cortex-Debug launch configuration

Planned tools:

- build, flash, debug helper generation
- report and artifact summary helpers

Keep command entrypoints small. Put reusable logic in tool-specific packages so workflows can be called from the shell, CMake, or generated VS Code tasks.
