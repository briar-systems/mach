# mach.cli.cmd.run

`mach run` — builds the project (same path as
[`build`](build.md)) and executes the resulting binary, forwarding
trailing positional args to the user program.

Source is `new/cli/cmd/run.mach` (currently empty).

## Functions

### `run`

```mach
pub fun run(argc: usize, argv: *zstr) i64
```

Subcommand entry point. Called by [`cmd.dispatch`](../cmd.md#dispatch).

Pipeline:

1. Defers to the [`build`](build.md) pipeline to produce an executable.
2. On a successful build, `exec`s the produced binary with the
   user-side argv (everything after a `--` separator on the
   original argv; or nothing when no separator was given).
3. The child's exit code becomes this subcommand's exit code.

| Param | Type    | Description                                              |
|-------|---------|----------------------------------------------------------|
| argc  | `usize` | Argument count.                                          |
| argv  | `*zstr` | Argument vector.                                         |

Returns:

- The child process's exit code on a successful build + exec.
- `1` on a user-facing build failure (compile errors, unknown target).
- `2` on an internal error.

`--emit obj` is rejected by this subcommand — running an object
file is not meaningful.

## Flags

Inherits every flag from [`build`](build.md). Adds:

| Flag        | Effect                                                                     |
|-------------|----------------------------------------------------------------------------|
| `--`        | Separator; everything after `--` is forwarded to the child as its argv.    |

## Dependencies

`std.types.size`, `std.types.zstr`,
[`mach.cli.cmd.build`](build.md),
[`mach.cli.config`](../config.md).
