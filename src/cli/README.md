# cli

Command-line driver for the Mach compiler.

## Files

- `cmd.mach` — subcommand dispatcher. Reads `argv[1]` and routes to the matching handler in `cmd/`.
- `args.mach` — shared argument-parsing helpers (`has_flag`, `get_value`).
- `config.mach` — reads `mach.toml` and produces a build configuration record consumed by the subcommands and the compiler session.
- `cmd/` — one file per subcommand.
