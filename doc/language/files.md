# Files

Source files use the `.mach` extension. A project has a `mach.toml` at its
root and conventionally one of two entrypoint files:

- **`lib.mach`** — library entrypoint. Used by projects that expose code for
  others to import; no executable produced.
- **`main.mach`** — executable entrypoint. Conventionally holds the program's
  entry function.

Both names are conventions only — the compiler attaches no meaning to either.
A project may declare both files; the target's `mode` and `entrypoint` in
`mach.toml` decide which is built, and the `entrypoint`'s `.mach` basename is
arbitrary.

## Executable entry

The compiler does not special-case a `main` function. An executable's entry is
whichever function **exports the linker symbol** `main`, tagged with
[`#[symbol("main")]`](decorators.md) and matching the runtime's expected
signature:

```mach
use std.runtime;

#[symbol("main")]
fun main(argc: i64, argv: **u8) i64 {
    ret 0;
}
```

`std.runtime` provides the platform-specific `_start` symbol the linker uses as
the true process entrypoint; `_start` decodes `argc`/`argv` and calls whatever
function exports `main`, then terminates the process with its returned exit
code. `use std.runtime;` is **required** to link `_start` into the binary, even
though no code references it by name.

Only the exported `main` symbol binds the entry — the Mach-level function name
is irrelevant, so `fun entry(...)` tagged `#[symbol("main")]` works identically.

## mach.toml

The project manifest. Declares the project's identity, its targets, and its
dependencies.

```toml
[project]
id      = "myproj"
name    = "My Project"
version = "0.1.0"
src     = "."
out     = "out"

[targets.linux]
os         = "linux"
isa        = "x86_64"
abi        = "sysv64"
mode       = "executable"
entrypoint = "main.mach"

[deps.mach-std]
type    = "remote"
path    = "https://github.com/octalide/mach-std"
version = "branch/dev"
```

The `id` is the root of every module path the project exposes. A file at
`src/foo/bar.mach` is reachable as `myproj.foo.bar`.

## See also

- [modules.md](modules.md) — how files map to module paths
- [use.md](use.md) — referencing modules from other modules
- [decorators.md](decorators.md) — `#[symbol("main")]` and the linker-name override
