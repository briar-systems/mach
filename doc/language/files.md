# Files

Source files use the `.mach` extension. A project has a `mach.toml` at its
root, a source directory the manifest names (`src` by convention), and one
entry module per artifact, named by the artifact's `entry` key. The
compiler attaches no meaning to any file name: `mach init` scaffolds
`src/root.mach` for a binary and `src/lib.mach` for a library, and either
basename is arbitrary.

An artifact's build is rooted at its entry module and follows `use` and `fwd`
edges from there; a file under `src` that no entry reaches is not part of that
artifact. `mach test` is the exception: it roots collection at every module in
the project's source tree.

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

The project manifest. It declares the project's identity, its targets, its
profiles, its artifacts, and its dependencies; every table is required to be
complete. The full reference is [manifest.md](../manifest.md). A minimal
binary project, as `mach init` writes it for one target:

```toml
[project]
id = "myproj"
version = "0.1.0"
src = "src"
out = "out/{target.name}/{profile.name}"

[target.linux-x86_64]
isa = "x86_64"
os  = "linux"
abi = "sysv64"

[artifact.myproj]
kind = "bin"
entry = "root.mach"
out = "bin/myproj"
targets = ["*"]
link = []
need = []

[profile.debug]
opt = 0
debug = true
simd = "scalarize"

[dep.std]
git = "https://github.com/briar-systems/mach-std"
ref = "branch/main"
```

The `id` is the root of every module path the project exposes. A file at
`src/foo/bar.mach` is reachable as `myproj.foo.bar`. The dependency `std` is
the standard library, realized as a git submodule at `dep/std/` and addressed
as `std.*` in source.

## See also

- [modules.md](modules.md) — how files map to module paths
- [use.md](use.md) — referencing modules from other modules
- [decorators.md](decorators.md) — `#[symbol("main")]` and the linker-name override
