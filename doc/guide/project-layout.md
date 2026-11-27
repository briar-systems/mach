# Project layout

A project is a directory with a `mach.toml` manifest at its root and `.mach` files under a source directory. Every command takes that directory, as in `mach build .`.

## What mach init writes

```sh
mach init demo
```

```text
demo/
  .gitignore
  .gitmodules
  mach.toml
  dep/
    std/
  src/
    bin/
      main.mach
```

| Path | Holds |
|---|---|
| `mach.toml` | The project's id, targets, artifacts, profiles and dependencies. See [Manifest](manifest.md) |
| `src/` | Every source file. The manifest's `src` key names it |
| `src/bin/main.mach` | The entry of the executable |
| `dep/std/` | The standard library, checked out as a git submodule. See [Dependencies](dependencies.md) |
| `.gitignore` | One line, `/out/` |

The directory is made a git repository unless it is already inside one.

| Flag | Effect |
|---|---|
| `--lib` | Write a library project |
| `--name <id>` | Set the project id. The default is the directory's name, with each `-` written as `_` |
| `--no-git` | No repository, no `.gitignore`, and `dep/std` as a plain checkout |
| `--no-deps` | No `dep/`, and nothing read from the network |

## Build output

`mach build` writes under `out/`, one directory per target and profile. `mach clean .` removes what builds wrote.

```sh
mach build .
mach build . -p release
```

```text
out/
  linux-x86_64/
    debug/
      bin/demo
    release/
      bin/demo
```

Object files and caches sit beside each `bin/`.

## Module paths

Every source file is a module. Its path is the project id, then the file's path under `src`, with `.` between the parts.

| File | Module |
|---|---|
| `src/config.mach` | `demo.config` |
| `src/shapes/circle.mach` | `demo.shapes.circle` |
| `src/bin/main.mach` | `demo.bin.main` |
| `dep/std/src/print.mach` | `std.print` |

A module is named by its full path everywhere, inside its own project too. [Modules](modules.md) covers importing one.

## Entries

An entry is the file a build starts from. Each artifact in the manifest names one, relative to `src`.

| Entry | Written by | Builds |
|---|---|---|
| `src/bin/main.mach` | `mach init` | An executable, under `bin/` |
| `src/lib/<id>.mach` | `mach init --lib` | A static library, under `lib/` |

- The names are conventions. The compiler gives no meaning to `bin`, `lib` or `main`.
- A build compiles the entry and every module it reaches through imports. A file no entry reaches is not compiled, so an error in it goes unreported.
- A project can declare several artifacts, each with its own entry.
- The function a program starts at is marked by a decorator, shown in [Hello world](hello-world.md).

The full rules are in the reference for [files](../language/files.md) and [modules](../language/modules.md).
