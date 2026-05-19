# Modules

Every `.mach` file is a module. Modules are identified by fully-qualified names (FQNs) derived from the project structure and `mach.toml` configuration.


## Import Syntax

### Bare Import

```mach
use std.types.bool;
```

A bare import makes the module's public symbols available without a prefix. Functions and types from `std.types.bool` can be used directly.

### Aliased Import

```mach
use print: std.print;
```

An aliased import requires the alias prefix to access symbols:

```mach
print.println("hello");
print.printf("n=%d\n", 42::i64);
```


## Module Resolution

Module paths are dot-separated identifiers that map to filesystem paths. The compiler resolves them deterministically from `mach.toml`.

### Project Modules

`[project].id` is the prefix for all modules in the project. Given `id = "myapp"` and `dir_src = "src"`:

```
src/main.mach           -> myapp.main
src/util/helpers.mach   -> myapp.util.helpers
```

### Dependency Modules

Dependencies are declared in `[deps.<alias>]`. The compiler reads the dependency's own `mach.toml` to find its `[project].id` and uses that as the module prefix:

```
dep/mach-std/src/print.mach  -> std.print  (if mach-std has id = "std")
```

The alias in `[deps.<alias>]` is a local handle for tooling. Import paths use the dependency's actual project ID.


## Module Graph

The compiler does not scan directories. It starts from the target's entrypoint and recursively follows `use` statements to build the module graph. Only modules reachable from the entrypoint are compiled.

For library targets, the entrypoint should `use` all modules intended to be part of the library's API:

```mach
# lib.mach (library entrypoint)
use mylib.parser;
use mylib.formatter;
```


## One File, One Module

There is no multi-module file concept. Each `.mach` file corresponds to exactly one module. File paths determine the module's FQN.


## Visibility

Only declarations marked `pub` are visible to other modules. Without `pub`, declarations are private to their module.
