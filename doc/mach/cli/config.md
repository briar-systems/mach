# mach.cli.config

CLI-level configuration shared across subcommands. Centralises the
small handful of cross-cutting settings (color output, verbosity,
working directory) that every subcommand respects in the same way,
so each command's `run` doesn't reimplement the same flag handling.

Source is `new/cli/config.mach` (currently empty — this spec is the
design intent).

## Types

### `Config`

```mach
pub rec Config {
    color:      ColorMode;
    verbose:    bool;
    quiet:      bool;
    cwd:        zstr;
    target:     zstr;
}
```

| Field   | Type                          | Description                                                |
|---------|-------------------------------|------------------------------------------------------------|
| color   | [`ColorMode`](#colormode)     | Resolved color preference.                                 |
| verbose | `bool`                        | `--verbose` was passed; subcommands surface extra detail.  |
| quiet   | `bool`                        | `--quiet` was passed; subcommands suppress non-error output. |
| cwd     | `zstr`                        | Resolved working directory (project root, defaults to `getcwd`). |
| target  | `zstr`                        | Selected target name (`--target <name>`; empty when unset — defaults are subcommand-specific). |

`verbose` and `quiet` are mutually exclusive at parse time —
[`parse`](#parse) emits an error and returns `none` when both are
present.

### `ColorMode`

```mach
pub def ColorMode: u8;
```

| Constant      | Value | Meaning                                                  |
|---------------|-------|----------------------------------------------------------|
| `COLOR_AUTO`  | 0     | Tty-detect (default).                                    |
| `COLOR_ALWAYS`| 1     | Force-enable.                                            |
| `COLOR_NEVER` | 2     | Force-disable.                                           |

## Constants

```mach
pub val COLOR_AUTO:   ColorMode = 0;
pub val COLOR_ALWAYS: ColorMode = 1;
pub val COLOR_NEVER:  ColorMode = 2;
```

## Functions

### `parse`

```mach
pub fun parse(argc: usize, argv: *zstr) Option[Config]
```

Reads the cross-cutting flags out of `argv` via
[`args.has_flag`](args.md#has_flag) / [`args.get_value`](args.md#get_value)
and produces a populated [`Config`](#config). Per-subcommand flags
are left in argv for the subcommand to consume itself.

Recognised flags:

| Flag             | Effect                                                     |
|------------------|------------------------------------------------------------|
| `--verbose` / `-v` | sets `verbose = true`.                                   |
| `--quiet` / `-q`   | sets `quiet = true`.                                     |
| `--color <mode>`   | `auto` / `always` / `never` → [`ColorMode`](#colormode). |
| `--cwd <path>`     | sets `cwd`; default is the process getcwd.               |
| `--target <name>`  | sets `target`.                                           |

Returns `none` on a parse error (e.g. unknown `--color` mode, or
both `--verbose` and `--quiet`).

## Dependencies

`std.types.bool`, `std.types.size`, `std.types.zstr`,
`std.types.option`,
[`mach.cli.args`](args.md).
