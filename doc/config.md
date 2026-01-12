- [Project Configuration (`mach.toml`)](#project-configuration-machtoml)
- [Environment variables in configuration](#environment-variables-in-configuration)
- [`[project]` section](#project-section)
  - [`id`](#id)
  - [`name`](#name)
  - [`version`](#version)
  - [`dir_src`](#dir_src)
  - [`dir_out`](#dir_out)
  - [`dir_dep`](#dir_dep)
  - [`target`](#target)
- [`[targets.<name>]` sections](#targetsname-sections)
  - [`os`](#os)
  - [`isa`](#isa)
  - [`abi`](#abi)
  - [`entrypoint`](#entrypoint)
  - [`artifacts`](#artifacts)
  - [`binary`](#binary)
  - [`mode`](#mode)
- [`[deps.<alias>]` sections](#depsalias-sections)
  - [`type`](#type)
  - [`path`](#path)
  - [`version`](#version-1)


## Project Configuration (`mach.toml`)

Every Mach project is driven by a `mach.toml` file located at the project root. The bootstrap compiler (`cmach`) refuses to build without one, and nearly every tool (`mach build`, `mach run`, `mach dep …`) reads it to discover modules, targets, and dependency metadata. This document explains the structure of the file, how the compiler interprets each setting, and how configuration ties into module discovery and dependency management.


```
[project]
id = "mach"
name = "Mach Compiler"
version = "0.7.1"
dir_src = "src"
dir_out = "out"
dir_dep = "dep"
target = "native"

[targets.linux]
os = "linux"
isa = "x86_64"
abi = "sysv64"
entrypoint = "main.mach"
artifacts = "linux"
binary = "linux/bin/mach"
mode = "executable"

[deps.mach-std]
type = "remote"
path = "https://github.com/octalide/mach-std"
version = "branch/main"
```

The configuration file uses [TOML](https://toml.io/en/) syntax. It consists of three main sections, each represented as a TOML table:

| TOML table         | Purpose                                                       |
| ------------------ | ------------------------------------------------------------- |
| `[project]`        | Global metadata, source layout, default target selection      |
| `[targets.<name>]` | Build artifacts, entry points, and backend options per target |
| `[deps.<alias>]`   | Declarative dependency list, also seeds module aliases        |

As a general rule of thumb, there are no optional fields or default values supplied on omission. Every field in a `mach.toml` must be explicitly set unless the documentation below specifies a default value or behavior.


## Environment variables in configuration

The compiler expands `${VAR}` expressions inside string values when reading `mach.toml` to allow for dynamic paths and settings based on the user’s environment.


## `[project]` section

The `[project]` table contains metadata about the current package as well as layout hints for locating source files and dependencies.


### `id`

The project ID is a globally unique identifier for the package.
It serves as the prefix for all modules defined within the project.
For example, if the project ID is `mach`, then a file located at `src/main.mach` would have the fully-qualified module name `mach.main`.
A project's ID should be distinct enough to avoid common naming collisions across different packages in the dependency graph.

There is NO central registry enforcing uniqueness; it is the author's responsibility to pick an appropriate ID.

> In the future, dependencies may be allowed to declare an "alias" for a package ID to help avoid conflicts.


### `name`

The human-friendly name of the project.
This is primarily used for display purposes in diagnostics and packaging tools.


### `version`

The version string of the project, following [semantic versioning](https://semver.org/) conventions.


### `dir_src`

The relative path to the project's source directory.
All modules defined within the project are expected to reside under this directory.
Project module discovery and resolution are based on this root path.

### `dir_out`

The directory where build output is written (per-target, under `dir_out/<artifacts>`).


### `dir_dep`

The root directory where dependencies are vendored into.

See the [`[deps.<alias>]` sections](#depsalias-sections) for more information about declaring dependencies.


### `target`

The default target name used when invoking `mach build .` without the `--target` flag.
This may be a concrete target name (e.g., `linux`, `darwin`) or `"native"` to auto-detect the host platform and select a matching target.


## `[targets.<name>]` sections

Each target table describes how to build for one platform and ABI combination.
The name (`linux`, `darwin`, `windows`, etc.) is arbitrary and only used as a selector by tooling.


### `os`

The target operating system identifier (e.g. `linux`).


### `isa`

The target instruction set architecture identifier (e.g. `x86_64`).


### `abi`

The target ABI identifier (e.g. `sysv64`).


### `entrypoint`

The `entrypoint` field defines the path to the root module that serves as the starting point for compilation.
This path is relative to `[project].dir_src` and is required for all targets.

> While it may not make sense to have an "entrypoint" for a library target, this field (and a corresponding module) is still required to establish the module graph.
> See `lib.mach` in the standard library for an example of a library entrypoint.


### `artifacts`

The `artifacts` field names the per-target subdirectory under `[project].dir_out`.
For example, with `dir_out = "out"` and `artifacts = "linux"`, intermediate and output files live under:

```
out/linux/
```


### `binary`

The `binary` field defines the output path for the produced executable or library.
This path is relative to `[project].dir_out/<artifacts>`.


### `mode`

The `mode` field determines the build mode for the target.
It accepts one of three values:
- `executable`: The driver links a final executable binary.
- `library`: The driver produces a static library archive (`.a` file).
- `shared`: The driver produces a shared object (`.so`, `.dll`, etc.).


## `[deps.<alias>]` sections

Dependencies are declared per-alias.
The `<alias>` is not significant beyond serving as an alias for command-line tools and diagnostics.

When using the `mach dep pull` command, dependencies are checked out under `<project-root>/<dir_dep>/<alias>` where `<alias>` is defined as `[deps.<alias>]`.

Note that this `<alias>` is distinct from the dependency's own project ID, which may differ.
For remote dependencies, this key corresponds to the repository name by default.
The directory referenced by `dir_dep` must live inside the project root so that `mach dep` can manage git submodules safely.


### `type`

The `type` field allows for two kinds of dependencies:
- `remote`: Vendored via git submodules.
- `local`: Path to an existing directory on disk.

> `local` dependencies do not support versioning, submodule management, or any form of automated updates.
> They can be updated using dependency management commands, but are directly copied from the specified path instead of being cloned or fetched.


### `path`

The `path` field specifies the location of the dependency. Its interpretation depends on the `type` field:
- When `"remote"`: Git URL of the repository to clone. Must be a valid, cloneable URL.
- When `"local"`: Filesystem path to the dependency directory. May be absolute or relative to the project root.


### `version`

The `version` field specifies the reference selector for remote dependencies.
It accepts commit hashes, branch selectors (`branch/<name>`), or semantic version constraints (e.g., `^1.2.3`, `~1.2.3`).
Local dependencies ignore `version` and do not require it to be set.
