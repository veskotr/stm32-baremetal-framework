# Repository Index

Use this page as the entry point for project context. Read the smallest set that covers the task.

## Foundation

| Document | Use it for |
| --- | --- |
| [README](../README.md) | Public overview, quick start, examples, and troubleshooting |
| [Architecture](architecture.md) | Layers, ownership boundaries, and long-term shape |
| [Development rules](development_rules.md) | Coding, integration, generation, and documentation rules |
| [Decision records](decisions/README.md) | Durable architectural decisions and their status |
| [Roadmap](roadmap.md) | Current status and intended implementation order |
| [Handoff](handoff.md) | Detailed implementation state and previously verified work |

`roadmap.md` and `handoff.md` are working-state documents. Confirm their claims against the current code and Git history before relying on them as current fact. Accepted decision records and ownership rules take precedence when the documents disagree.

## Workflows

| Document | Use it for |
| --- | --- |
| [CubeMX workflow](cubemx_workflow.md) | Board generation, synchronization, and file ownership |
| [Configuration system](config_system.md) | Profiles, schema, and generated config outputs |
| [Testing strategy](testing.md) | Host, firmware-build, hardware, and release verification lanes |
| [External projects](external_project.md) | FetchContent consumption and application integration |
| [VS Code](vscode.md) | Generated editor, build, flash, and debug tasks |

## Focused designs

| Document | Use it for |
| --- | --- |
| [Watchdog design](watchdog_design.md) | Watchdog API, role mapping, and intended tests |

Focused design documents describe a feature's intended design. Check implementation and tests to determine whether each section is proposed, in progress, or complete.

## Component guides

| Document | Use it for |
| --- | --- |
| [Drivers](../drivers/README.md) | Reusable device-driver expectations |
| [Protocols](../protocols/README.md) | Protocol integration expectations |
| [Examples](../examples/README.md) | Example targets and build commands |
| [Tools](../tools/README.md) | Generator and developer tooling entry points |

## AI collaboration files

- [`AGENTS.md`](../AGENTS.md) is the automatically loaded repository contract.
- [`.agents/skills/`](../.agents/skills/) contains repeatable HSS task workflows.
- [`.codex/agents/`](../.codex/agents/) contains narrow project-scoped subagent roles.
- [`.codex/config.toml`](../.codex/config.toml) contains conservative shared Codex settings.

Do not duplicate project facts into AI configuration when an authoritative project document already owns them. Link to the source instead.
