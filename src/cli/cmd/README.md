# cli/cmd

Subcommand implementations. Each file exposes a `run(argc, argv)` function that returns an exit code.

## Files

- `build.mach` — compile the current project to an object file or executable.
- `run.mach` — build the project and execute the resulting binary.
- `test.mach` — build and run the project's test suite.
- `dep.mach` — manage project dependencies declared in `mach.toml`.
- `init.mach` — scaffold a new Mach project in the current directory.
- `help.mach` — print usage information for the CLI and its subcommands.
