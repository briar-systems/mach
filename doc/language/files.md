# Files

Source files use the `.mach` extension. A project has a `mach.toml` at its
root and conventionally one of two entrypoint files:

- **`lib.mach`** — library entrypoint. Used by projects that expose code for
  others to import; no executable produced.
- **`main.mach`** — executable entrypoint. Defines `fun main()`.

A project may have both, but the dominant role is determined by the target's
`mode` and `entrypoint` in `mach.toml`.

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
