# mach.cli.cmd.init

`mach init` — scaffolds a new project. Writes a minimal `mach.toml`,
a starter `src/main.mach`, and an empty `dep/` directory in the
current (or specified) directory.

Source is `new/cli/cmd/init.mach` (currently empty).

## Functions

### `run`

```mach
pub fun run(argc: usize, argv: *zstr) i64
```

Subcommand entry point. Called by [`cmd.dispatch`](../cmd.md#dispatch).

Pipeline:

1. Resolve the project directory (positional `argv[2]` or `config.cwd`).
2. Verify it's empty (or `--force` is set) — refuses to clobber an
   existing project by default.
3. Write `mach.toml` with `[project].id` taken from the directory
   name (or `--name <name>`).
4. Write a `[targets.host]` entry whose `os` / `isa` match the
   running compiler.
5. Write `src/main.mach` containing a hello-world stub.
6. Create an empty `dep/` directory.

| Param | Type    | Description                                              |
|-------|---------|----------------------------------------------------------|
| argc  | `usize` | Argument count.                                          |
| argv  | `*zstr` | Argument vector.                                         |

Returns:

- `0` on success.
- `1` on a user-facing error (non-empty target directory without
  `--force`, invalid name).
- `2` on an internal error.

## Flags

| Flag                | Effect                                                                  |
|---------------------|-------------------------------------------------------------------------|
| `--name <name>`     | Override the project id (defaults to the directory name).               |
| `--force`           | Allow scaffolding into a non-empty directory; clobbers conflicting files. |
| `--bin`             | Generate a binary project layout (the default).                          |
| `--lib`             | Generate a library project layout — `src/lib.mach` instead of `main.mach`, no `entrypoint` in `[targets.host]`. |

## Dependencies

`std.types.size`, `std.types.zstr`,
`std.filesystem`,
[`mach.cli.config`](../config.md).
