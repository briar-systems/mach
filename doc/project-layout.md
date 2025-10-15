# Project layout

Mach applications are organised around a manifest (`mach.toml`) and a set of module search paths. This repository captures the conventions we expect most early projects to follow.

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

- `project` names the binary and records the entry point relative to `src-dir`.
- `directories` tells `cmach` where to find source files, where to drop build artefacts, and where dependencies should be staged.
- `deps` declares external trees. Each entry points at a filesystem path and a source directory within that tree. The example maps the prefix `std` to `../mach-std/src`.
- `modules` defines import aliases. `std = "std"` makes `use std.io.console;` resolve through the dependency declared above.

## Source tree

- `src/main.mach` wires command-line entry to `commands_dispatch`.
- `src/commands.mach` contains the CLI implementation. It demonstrates:
  - dynamically growing slices using `std.types.array` (`array_append` and friends),
  - string manipulation helpers from `std.types.string`,
  - console output via `std.io.console`.

Adding new modules is as simple as dropping additional `.mach` files into `src/` and referencing them with `use`.

## Build pipeline

The `Makefile` keeps the steps reproducible:

1. `cmach build src/main.mach --emit-obj --no-link -o out/obj/main.o` – compile to an object file using the module mappings from `mach.toml`.
2. `cc out/obj/main.o ../mach-std/out/lib/libmachstd.a -o out/bin/mach` – link the executable against the standard library archive.

Adjust the `CMACH` variable if the compiler lives somewhere else, or extend the make rules with additional objects once the application grows beyond a single file.

## Memory management

The array helpers in `std.types.array` encode the “growable slice” policy used across the repositories. They store the current capacity and element stride immediately before the slice data pointer. When you call `array_append` or `array_reserve`, always reassign the return value—the helper may allocate a new buffer.

Freeing the slice is explicit:

```mach
use std.types.array;

var bytes: []u8 = []u8{ nil, 0 };
bytes = array_append<u8>(bytes, 42);
bytes = array_free<u8>(bytes);
```

This pattern mirrors what you will find inside `src/commands.mach` when it collects CLI options.
