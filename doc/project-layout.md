# Project layout

A Mach project is defined by a manifest (`mach.toml`), a source tree, and a build pipeline that drives the compiler. This document explains how those pieces fit together using this repository as the template.

## Directory structure

```
mach/
├── Makefile          # reference build for the sample CLI
├── mach.toml         # project manifest consumed by cmach
├── src/
│   ├── main.mach     # program entry point (calls commands_dispatch)
│   └── commands.mach # command-line handling and option parsing
└── doc/              # language spec + project guides (this directory)
```

Future self-hosted compiler sources will live alongside these files.

## `mach.toml`

```toml
[project]
name = "mach"
version = "0.0.1"
entrypoint = "main.mach"

[directories]
src-dir = "src"
dep-dir = "dep"
out-dir = "out"

[deps]
std = { path = "../mach-std", src = "src" }

[modules]
std = "std"
```

- `project` – identifies the build artefact and entry point (relative to `src-dir`).
- `directories` – controls where sources, dependencies, and build outputs live.
- `deps` – maps dependency names (`std`) to filesystem locations. Each entry points at a repository and the subdirectory that contains `.mach` files.
- `modules` – maps import prefixes to dependency entries. With `std = "std"`, `use std.io.console;` resolves into the dependency declared above.

## Makefile pipeline

The `Makefile` keeps the build reproducible:

1. **Compile:**
   ```make
   $(CMACH) build src/main.mach --emit-obj --no-link -o out/obj/main.o
   ```
   `CMACH` defaults to `../mach-c/bin/cmach`. The manifest is auto-detected, so module paths defined in `mach.toml` are available during compilation.

2. **Link:**
   ```make
   cc -nostartfiles -nostdlib -no-pie -o out/bin/mach $(OBJ_FILES) ../mach-std/out/lib/libmachstd.a
   ```
   The sample app links directly against the static standard library. Projects can add additional objects or libraries as needed.

3. **Run / Clean:** `make run` executes `out/bin/mach`; `make clean` removes the `out/` tree.

Adapt this flow for multi-file applications by adding more compilation rules that target `out/obj/*.o` and extending the link step.

## Module usage

Within `src/commands.mach` you can see typical imports:

```mach
use std.io.console;
use std.types.array;
use std.types.string;
```

These work because `mach.toml` maps the prefix `std` to `../mach-std/src`. When you add your own libraries, define new entries under `[deps]` and add corresponding mappings under `[modules]`.

## Dynamic arrays and memory

The codebase relies on the helpers in `std.types.array` to manage growable slices. Keep the naming convention (`array_*`) when adding new helpers so the exported symbols remain stable for the forthcoming self-hosted compiler. Always reassign the result of array helpers:

```mach
arr = array_append<u8>(arr, value);
arr = array_free<u8>(arr);
```

`std.system.memory` exposes low-level allocation (`allocate`, `reallocate`, `deallocate`) if you need custom data structures.

## Updating the layout

As the self-hosted compiler grows, document every new directory or build artifact here. The goal is for contributors to understand the repository at a glance without reading the entire source tree.
