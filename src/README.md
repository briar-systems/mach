# mach

The Mach compiler: CLI driver and language implementation.

## Layout

- `main.mach` — program entry point; delegates to the CLI dispatcher.
- `cli/` — command-line driver: argument parsing, configuration, and subcommand dispatch.
- `lang/` — compiler internals: frontend, middle-end, backend, and cross-cutting services.
