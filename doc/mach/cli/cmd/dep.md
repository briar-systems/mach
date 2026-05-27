# mach.cli.cmd.dep

`mach dep` — manages vendored dependencies under the project's
`dir_dep` directory (default `dep/`). Sub-subcommands operate on the
on-disk vendor layout and on the `[deps.<alias>]` entries of
`mach.toml`.

Source is `new/cli/cmd/dep.mach` (currently empty).

## Functions

### `run`

```mach
pub fun run(argc: usize, argv: *zstr) i64
```

Subcommand entry point. Called by [`cmd.dispatch`](../cmd.md#dispatch).
Reads `argv[2]` as the sub-subcommand name and dispatches.

| Param | Type    | Description                                              |
|-------|---------|----------------------------------------------------------|
| argc  | `usize` | Argument count.                                          |
| argv  | `*zstr` | Argument vector; `argv[0]` is the program, `argv[1]` is `dep`. |

Returns:

- `0` on success.
- `1` on a user-facing error (unknown sub-subcommand, missing
  required arg, conflicting entry in `mach.toml`).
- `2` on an internal error.

## Sub-subcommands

| Name      | Effect                                                                  |
|-----------|-------------------------------------------------------------------------|
| `list`    | Print every `[deps.<alias>]` entry from `mach.toml` plus its resolved vendor root. |
| `add`     | `mach dep add <alias> <source>` — clone or unpack `<source>` into `dir_dep/<alias>/` and append a `[deps.<alias>]` entry to `mach.toml`. |
| `remove`  | `mach dep remove <alias>` — delete the vendor directory and strip the entry from `mach.toml`. |
| `sync`    | Re-fetch every dep currently listed in `mach.toml` (idempotent — does nothing on already-present aliases unless `--force`). |
| `vendor`  | `mach dep vendor <path>` — copy a local directory into `dir_dep/` as a vendored dep without going through `<source>` resolution. |

## Flags

| Flag       | Effect                                                                     |
|------------|----------------------------------------------------------------------------|
| `--force`  | Re-download / overwrite even when a target alias is already populated.     |
| `--dry-run`| Print what would be done without touching the filesystem or `mach.toml`.   |

## Dependencies

`std.types.size`, `std.types.zstr`, `std.data.toml`,
`std.filesystem`,
[`mach.cli.config`](../config.md).
