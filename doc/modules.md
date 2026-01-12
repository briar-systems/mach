## Overview

Mach treats every `.mach` file as a self-contained module. Modules are identified by fully-qualified names (FQNs) that reflect their position in the project and dependency hierarchy. This document explains how the module loader maps FQNs to filesystem paths, how it handles project and dependency namespaces, and how the configuration file (`mach.toml`) influences module resolution.

Many fields referenced in this document are described in detail in [config.md](config.md).

- [Overview](#overview)
- [Namespaces and naming rules](#namespaces-and-naming-rules)
  - [Project source root (`src`)](#project-source-root-src)
  - [Dependency root (`dep`)](#dependency-root-dep)
  - [Project modules](#project-modules)
  - [Dependency modules](#dependency-modules)
- [A note on library entrypoints](#a-note-on-library-entrypoints)

## Namespaces and naming rules

Mach’s module namespace is entirely deterministic.
Given the same `mach.toml`, the compiler will always assign the same FQN to the same file, regardless of the host machine.


### Project source root (`src`)

`[project].src` points to the directory that contains the project’s code.
It is interpreted relative to the project root.


### Dependency root (`dep`)

`[project].dep` defines the directory where dependencies are vendored.
Both local and remote dependencies alike are copied or cloned into this directory under their declared alias names.


### Project modules

1. **Project ID prefix:** `[project].id` is mandatory and becomes the prefix for every module defined inside the project. If `id = "mach"`, the file `src/main.mach` is `mach.main`.
2. **Path segments:** Everything after the prefix mirrors the path from the `src` directory, using dots instead of slashes and dropping the `.mach` extension. `src/driver/pipeline.mach` becomes `mach.driver.pipeline`.
3. **One file, one module:** There is no multi-module source file concept. Duplicating filenames is supported as long as the relative path (and therefore the namespace) remains unique.

Because IDs are user-chosen (there is no global registry), you are responsible for picking values that will not collide with your dependencies.


### Dependency modules

Each dependency is declared under `[deps.<alias>]` (see [config.md](config.md#depsalias-sections)).
The alias is only a user-facing handle used by tools like `mach dep list` and by source imports (`use std.print;`).
When the loader processes an import it:

1. Looks up the alias in the dependency table.
2. Loads the dependency’s own `mach.toml` from `<dep>/<alias>/mach.toml`.
3. Reads that dependency’s `[project].id` and replaces the alias with the real project ID.

This example shows how the filesystem layout maps to module names according to these rules and given hypothetical `mach.toml` files:

```
dep/
    my_dep/
        src/
            dep_module.mach # `my_project.dep_module`
        mach.toml           # [project].id = "my_project"
src/
    main.mach               # `my_app.main`
mach.toml                   # [project].id = "my_app"
```


## A note on library entrypoints

Each target declared in `[targets.<name>]` must specify `entrypoint`, even for library-style builds.
The entrypoint path is relative to `[project].src` and identifies the root module for that build.

This system is in place because of Mach's fully deterministic module resolution system.
The Mach compiler does not "scan" any source tree or "discover" modules automatically.
Instead, it builds a module graph starting from the specified entrypoint module and recursively follows `use` statements.

In a typical "library" mode entrypoint, the root module should `use` all public modules that are intended to be included in the library's API surface.

Note that this does NOT govern "symbol visibility" at the linking stage, it just kickstarts the module resolution process.
Visibility and symbol export rules are determined by the contents of each individual module itself.

An example "library" project with multiple public modules might have this structure:

```
src/
    internal/
        my_helper.mach # private/internal module
    lib.mach           # entrypoint module
    utils.mach         # "public" utility module
mach.toml              # [project].id = "my_library"
```

With `lib.mach` as the entrypoint, it might look like this:

```mach
# lib.mach
use my_library.utils;
```

Note the lack of `use my_library.internal.my_helper;`. That module may still be exported with the library, but only if it is directly used by `utils.mach` in this example.

This explicit entrypoint system ensures that library authors have full control over which modules are included in the build and how the module graph is constructed, leading to predictable and manageable builds, especially for libraries that involve conditionally included modules.
