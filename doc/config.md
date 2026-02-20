# Project Configuration

Every Mach project has a `mach.toml` file at the project root. It defines project metadata, build targets, and dependencies.

```toml
[project]
id = "mach"
name = "Mach Compiler"
version = "0.8.3"
dir_src = "src"
dir_out = "out"
dir_dep = "dep"
target = "native"

[targets.linux]
os = "linux"
isa = "x86_64"
abi = "sysv64"
mode = "executable"
entrypoint = "main.mach"
artifacts = "linux"
binary = "linux/bin/mach"

[deps.mach-std]
type = "remote"
path = "https://github.com/octalide/mach-std"
version = "branch/dev"
```


## `[project]`

| Field | Description |
|-------|-------------|
| `id` | Project identifier. Becomes the module prefix (e.g. `mach.main` for id `mach`). Must be unique across the dependency graph. |
| `name` | Human-friendly project name for diagnostics and tooling. |
| `version` | Semantic version string. |
| `dir_src` | Source directory relative to project root. Default: `src`. |
| `dir_out` | Output directory for build artifacts. Default: `out`. |
| `dir_dep` | Dependency root directory. Default: `dep`. |
| `target` | Default target name. Use `"native"` to auto-detect the host platform, or a specific target name like `"linux"`. |


## `[targets.<name>]`

Each target describes a build configuration for one platform/ABI combination. The `<name>` is arbitrary (e.g. `linux`, `darwin`) and used as a selector by tooling.

| Field | Description |
|-------|-------------|
| `os` | Target OS: `linux`, `darwin`, `windows` |
| `isa` | Instruction set: `x86_64`, `aarch64` |
| `abi` | ABI: `sysv64`, `win64` |
| `mode` | Build mode: `executable`, `library`, `shared` |
| `entrypoint` | Root module path relative to `dir_src` (e.g. `main.mach`). Required even for libraries to establish the module graph. |
| `artifacts` | Per-target subdirectory under `dir_out`. |
| `binary` | Output binary path relative to `dir_out`. |


## `[deps.<alias>]`

See [dependencies.md](dependencies.md) for full details.

| Field | Description |
|-------|-------------|
| `type` | `remote` (git submodule) or `local` (filesystem path) |
| `path` | Git URL for remote, filesystem path for local |
| `version` | Version selector for remote deps (see [dependencies.md](dependencies.md#version-selectors)) |


## Module Resolution

The project ID determines how module paths map to files. Given `id = "myapp"` and `dir_src = "src"`:

```
src/main.mach           -> myapp.main
src/util/helpers.mach   -> myapp.util.helpers
```

For dependencies, the compiler reads each dependency's `mach.toml` to get its project ID and uses that as the module prefix:

```
dep/mach-std/src/print.mach  -> std.print  (mach-std has id = "std")
```

See [modules.md](modules.md) for full module resolution details.
