# mach.cli.cmd.help

`mach help` — prints usage. The fallback target for
[`cmd.dispatch`](../cmd.md#dispatch) when no subcommand or an
unknown subcommand is given.

Source is `new/cli/cmd/help.mach` (currently empty).

## Functions

### `run`

```mach
pub fun run(argc: usize, argv: *zstr) i64
```

Subcommand entry point. Called by [`cmd.dispatch`](../cmd.md#dispatch),
also from the dispatcher's error path (unknown subcommand or
missing argument).

Behaviour:

- With no further args (`argc < 3`), prints the top-level usage
  summary: program tagline, list of known subcommands and one-line
  descriptions, list of global flags.
- With `argv[2]` naming a known subcommand, prints that
  subcommand's detail page (flags, exit codes).
- With an unknown `argv[2]`, prints the top-level usage and returns
  non-zero.

| Param | Type    | Description                                              |
|-------|---------|----------------------------------------------------------|
| argc  | `usize` | Argument count.                                          |
| argv  | `*zstr` | Argument vector.                                         |

Returns:

- `0` for the top-level usage and for a named-subcommand detail page.
- `1` when invoked with an unknown subcommand name.

## Dependencies

`std.types.size`, `std.types.zstr`,
`std.print`,
[`mach.cli.config`](../config.md).
