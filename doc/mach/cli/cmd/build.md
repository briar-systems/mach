# mach.cli.cmd.build

`mach build` — compiles the current project to an object file or
executable. Loads the project via
[`driver.build_project`](../../lang/driver.md#build_project) and
drives it through resolve, sema, IR lowering, optimisation, and
codegen, then writes the output via the active
[object format](../../lang/target/of.md).

Source is `new/cli/cmd/build.mach` (currently empty).

## Functions

### `run`

```mach
pub fun run(argc: usize, argv: *zstr) i64
```

Subcommand entry point. Called by [`cmd.dispatch`](../cmd.md#dispatch).

Pipeline:

1. [`config.parse`](../config.md#parse) for cross-cutting flags
   (verbose, target, cwd).
2. Resolve the project root by walking up from `config.cwd` until a
   `mach.toml` is found.
3. Initialise a [`Session`](../../lang/session.md#session) on the
   process allocator.
4. Call [`driver.build_project`](../../lang/driver.md#build_project)
   with the chosen target.
5. Drive the middle-end (lower → optimise) and backend (isel →
   regalloc → emit object) for each module's
   [`ResolveResult`](../../lang/fe/resolve.md#resolveresult).
6. Write the object / executable to `<project_root>/out/`.
7. Flush diagnostics and tear down the session.

| Param | Type    | Description                                              |
|-------|---------|----------------------------------------------------------|
| argc  | `usize` | Argument count.                                          |
| argv  | `*zstr` | Argument vector; `argv[0]` is the program, `argv[1]` is `build`. |

Returns:

- `0` on a successful build.
- `1` on a user-facing error (missing `mach.toml`, unknown target,
  compile errors).
- `2` on an internal error (allocation failure, IO error not
  attributable to user input).

## Flags

| Flag                | Effect                                                            |
|---------------------|-------------------------------------------------------------------|
| `--target <name>`   | Selects a `[targets.<name>]` entry from `mach.toml`.              |
| `--release`         | Enables the release optimisation pipeline (defaults to debug).    |
| `--out <path>`      | Override the output path; default is `<project_root>/out/<target>/<project.id>`. |
| `--emit <kind>`     | `obj` / `exe` (default depends on the target).                    |

## Dependencies

`std.types.size`, `std.types.zstr`,
[`mach.cli.config`](../config.md),
[`mach.lang.session`](../../lang/session.md),
[`mach.lang.driver`](../../lang/driver.md),
[`mach.lang.diagnostic`](../../lang/diagnostic.md).
