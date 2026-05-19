# mach.main

Program entry point. Forwards argc/argv to the CLI dispatcher and
returns its exit code.

## Functions

### `main`

```mach
$main.symbol = "main";
fun main(argc: usize, argv: *zstr) i64
```

Linked under the symbol `main` (the runtime's `_start` calls this).
Forwards to [`cmd.dispatch`](cli/cmd.md#dispatch) and returns its result.

| Param | Type    | Description                                              |
|-------|---------|----------------------------------------------------------|
| argc  | `usize` | Argument count including `argv[0]`.                      |
| argv  | `*zstr` | Pointer to `argc` zero-terminated byte pointers.         |

Returns the process exit code.

## Dependencies

`std.runtime`, `std.types.size`, `std.types.zstr`,
[`mach.cli.cmd`](cli/cmd.md).

