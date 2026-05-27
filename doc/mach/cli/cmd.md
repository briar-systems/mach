# mach.cli.cmd

Subcommand dispatcher. Single entry point [`dispatch`](#dispatch)
called from [`mach.main`](../main.md), routes the second positional
argument (the subcommand name) to one of the per-command modules
under [`cli/cmd/`](#known-subcommands).

## Functions

### `dispatch`

```mach
pub fun dispatch(argc: usize, argv: *zstr) i64
```

Inspects `argv[1]` and forwards to the matching subcommand's `run`.
With fewer than two args, or with an unknown subcommand, prints an
error (when applicable) and invokes [`cmd_help.run`](cmd/help.md#run).

| Param | Type    | Description                                              |
|-------|---------|----------------------------------------------------------|
| argc  | `usize` | Argument count including `argv[0]`.                      |
| argv  | `*zstr` | Pointer to `argc` zero-terminated byte pointers.         |

Returns the process exit code:
- The subcommand's own exit code on a successful dispatch.
- `1` when no subcommand was given (after printing help).
- `1` when the subcommand was unknown (after printing the error and help).

## Known subcommands

| Name    | Module                          | Purpose                                                  |
|---------|---------------------------------|----------------------------------------------------------|
| `build` | [`cmd_build`](cmd/build.md)     | Compile the current project to an object / executable.   |
| `run`   | [`cmd_run`](cmd/run.md)         | Build and execute the resulting binary.                  |
| `test`  | [`cmd_test`](cmd/test.md)       | Build and run the project's test entries.                |
| `dep`   | [`cmd_dep`](cmd/dep.md)         | Manage the project's vendored dependencies.              |
| `init`  | [`cmd_init`](cmd/init.md)       | Scaffold a new project.                                  |
| `help`  | [`cmd_help`](cmd/help.md)       | Print usage.                                             |

The dispatch table is hard-coded — subcommands are not pluggable
across builds. Adding one requires editing both this dispatch and
the [Known subcommands](#known-subcommands) table.

## Dependencies

`std.types.size`, `std.types.zstr`, `std.print`,
[`mach.cli.cmd.help`](cmd/help.md),
[`mach.cli.cmd.build`](cmd/build.md),
[`mach.cli.cmd.run`](cmd/run.md),
[`mach.cli.cmd.test`](cmd/test.md),
[`mach.cli.cmd.dep`](cmd/dep.md),
[`mach.cli.cmd.init`](cmd/init.md).
