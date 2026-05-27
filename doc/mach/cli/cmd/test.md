# mach.cli.cmd.test

`mach test` — builds the project with `test` declarations enabled
and runs them, reporting pass / fail counts to stdout. Drives the
same pipeline as [`build`](build.md) but emits a test runner stub
instead of the user's `main`.

Source is `new/cli/cmd/test.mach` (currently empty).

## Functions

### `run`

```mach
pub fun run(argc: usize, argv: *zstr) i64
```

Subcommand entry point. Called by [`cmd.dispatch`](../cmd.md#dispatch).

Pipeline:

1. Build the project with the test-runner stub as the entry point
   (the stub iterates every
   [`SYM_TEST`](../../lang/fe/resolve.md#constants) declaration the
   resolver collected).
2. Exec the resulting binary directly; its stdout is the test report.
3. Surface its exit code: `0` if every test passed, `1` if any
   failed.

| Param | Type    | Description                                              |
|-------|---------|----------------------------------------------------------|
| argc  | `usize` | Argument count.                                          |
| argv  | `*zstr` | Argument vector.                                         |

Returns:

- `0` if every collected test passed.
- `1` if any test failed.
- `2` on a build or internal error.

## Flags

Inherits every flag from [`build`](build.md). Adds:

| Flag                  | Effect                                                                   |
|-----------------------|--------------------------------------------------------------------------|
| `--filter <pattern>`  | Run only tests whose `test "..."` name string matches `pattern` (substring match). |
| `--list`              | List the collected tests and exit `0` without running them.              |

## Dependencies

`std.types.size`, `std.types.zstr`,
[`mach.cli.cmd.build`](build.md),
[`mach.cli.config`](../config.md),
[`mach.lang.fe.resolve`](../../lang/fe/resolve.md).
