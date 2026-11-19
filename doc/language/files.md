# Files

Source files use the `.mach` extension. A project has a `mach.toml` at its
root, a source directory the manifest names (`src` by convention), and one
entry module per artifact, named by the artifact's `entry` key. The
compiler attaches no meaning to any file name. By convention an entry sits
in a directory of its own, so nothing else under `src` reads as one:
`src/bin/` for an executable and `src/lib/` for a library. `mach init`
scaffolds `src/bin/main.mach` for a binary and `src/lib/<id>.mach` for a
library.

An artifact's build is rooted at its entry module and follows `use` and `fwd`
edges from there; a file under `src` that no entry reaches is not part of that
artifact, under `mach build`, `mach check` and `mach test` alike, so a test
runs only in a module an entry reaches (see [test.md](test.md#which-tests-run)).

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

`std.runtime` defines the symbol each system's catalog names as the process
entrypoint: `_start` on linux, wasi and freestanding targets, `mainCRTStartup`
on windows, `start` on darwin and `efi_main` on uefi. The linker enters the
image there and never at `main`. The runtime's entry decodes `argc`/`argv`,
calls whatever function exports `main`, and terminates the process with its
returned exit code, so `main` is kept by that call like any other function.
`use std.runtime;` is **required** to link the entry into the binary, even
though no code references it by name.

Only the exported `main` symbol is what the runtime calls, and the Mach-level
function name is irrelevant, so `fun entry(...)` tagged `#[symbol("main")]` works identically.

## mach.toml

The project manifest. It declares the project's identity, its targets, its
profiles, its artifacts, and its dependencies; every table is required to be
complete. The full reference is [manifest.md](manifest.md). A binary
project as `mach init` writes it, shown with one of the six targets it
declares (`linux-x86_64`, `linux-aarch64`, `linux-riscv64`, `darwin-x86_64`,
`darwin-aarch64`, `windows-x86_64`), each stating every part mink's catalog
gives it but `isa`, which is left to the system's floor:

```toml
[project]
id = "myproj"
version = "0.1.0"
mach = "^7"
src = "src"
work = "out/{target.name}/{profile.name}"

[target.linux-x86_64]
arch = "x86_64"
sys  = "linux"
abi  = "sysv64"

[artifact.myproj]
kind = "bin"
entry = "bin/main.mach"
out = "{project.work}/bin/myproj{artifact.suffix}"
targets = ["*"]

[profile.debug]
default = true
optimize = false
debug = true
simd = "scalarize"

[profile.release]
optimize = true
debug = false
simd = "scalarize"

[dep.std]
git = "https://github.com/briar-systems/mach-std"
version = "^X.Y.Z"
```

The manifest declares no `[link.*]` table: the system libraries a program
needs on darwin and windows are std's `export = true` entries, which travel to
every project that uses it. A library project (`mach init --lib`) differs only
in its artifact, a `static` one with `entry = "lib/<id>.mach"` and
`export = true`. The `[dep.std]` range is the one `mach dep add` writes, with
`X.Y.Z` the std release resolution picks for the running compiler (see
[dependencies.md](dependencies.md#releases-and-resolution)).

The `id` is the root of every module path the project exposes. A file at
`src/foo/bar.mach` is reachable as `myproj.foo.bar`. The dependency `std` is
the standard library, realized as a git submodule at `dep/std/` and addressed
as `std.*` in source.

## What mach init does

- **The directory.** `mach init <dir>` takes exactly one operand, the
  project's directory, made when it does not exist. `mach init .` names the
  working directory.
- **The id.** `--name` gives it, else the directory's name. A `use` path must
  be able to spell the id, so it starts with a letter or `_` and holds only
  letters, digits and `_`. A directory name such as `my-app` becomes `my_app`,
  each `-` written as `_`, and `mach init` says which id it chose. A `--name`
  that is not an id, or a directory name that stays one after that, is refused.
- **std.** `[dep.std]` is declared at the selection the running compiler states
  for std: `ref = "branch/dev"` until the std major that accepts this compiler
  is released, and that major's version range from then on. A ref is written as
  it is. A range is resolved before anything is written, reading std's releases
  from the network once (`mach init` says so first, and checking std out reads
  the same listing), and when no release in it
  exists nothing is created and the error names the way on: `mach init
  --no-deps`, then `mach dep add <dir> std --git <url> --ref branch/<name>` to
  take std from a branch.
- **`--no-deps`** declares no dependency and reads nothing from the network. It
  is for a project offline, or one that names std's source itself. The entry
  still uses std, so the project builds once std is declared, and `mach init`
  prints the `mach dep add` command that declares it.
- **Git.** Unless `--no-git`, the directory is made a repository (unless it
  stands in one already), std is checked out as a submodule, and a `.gitignore`
  holding `/out/`, the work directory every build writes under, is written
  where none exists. `dep/` is not ignored, since each checkout there is a
  submodule its pin records. Under `--no-git` no `.gitignore` is written.
- **The files** are written whole or not at all. When a write fails, every file
  and directory `mach init` made is removed again, and nothing that was there
  before is touched. A file `--force` replaces is replaced after every new file
  is written, so a failure among the new ones replaces nothing.
- **A failure** after the files are written, in `git init` or in checking std
  out, leaves the project complete and says which step to rerun.

## See also

- [modules.md](modules.md) — how files map to module paths
- [use.md](use.md) — referencing modules from other modules
- [decorators.md](decorators.md) — `#[symbol("main")]` and the linker-name override
