# Manifest

Every project has a `mach.toml` at its root. It states what the project builds and for which platforms. Nothing is inferred from the directory layout.

## A minimal manifest

```toml
[project]
id      = "demo"
version = "0.1.0"
mach    = "^7"
src     = "src"
work    = "out/{target.name}/{profile.name}"

[target.linux-x86_64]
arch = "x86_64"
sys  = "linux"
abi  = "sysv64"

[profile.debug]
optimize = false
debug    = true
simd     = "scalarize"

[artifact.demo]
kind    = "bin"
entry   = "bin/main.mach"
out     = "{project.work}/bin/demo{artifact.suffix}"
targets = ["*"]

[dep.std]
git     = "https://github.com/briar-systems/mach-std"
version = "^10.1.1"
```

`mach init` writes this shape with six targets and a second profile, `release`. An unknown table or key is an error, and a required key has no default:

```text
error[manifest.missing_required] mach.toml: [profile.debug] is missing required key 'simd'. a profile declares optimize, debug and simd
```

## `[project]`

All five keys are required.

| Key | Meaning |
|---|---|
| `id` | The first segment of every module path: `src/a/b.mach` is `<id>.a.b`. Letters, digits and `_` |
| `version` | The project's version |
| `mach` | The compiler versions the project builds with, as a [version range](dependencies.md#version-ranges) |
| `src` | The source directory |
| `work` | The work directory, a [path template](#path-templates). Objects and caches go here |

## `[target.<name>]`

A target is a platform the project builds for, and its name is what `-t` takes.

```toml
[target.windows-x86_64]
arch   = "x86_64"
sys    = "windows"
abi    = "win64"
runner = ["wine", "{artifact.out}"]
```

| Key | Required | Meaning |
|---|---|---|
| `arch` | yes | Architecture |
| `sys` | yes | System |
| `abi` | yes | Calling convention |
| `env` | no | Environment. The default is the system's |
| `isa` | no | Instruction set every machine of the target has, such as `x86-64-v2` |
| `ext` | no | Instruction-set extensions the target may assume, such as `["sha"]` |
| `runner` | no | Command that `mach run` and `mach test` launch the target's programs through |

| Key | Accepted values |
|---|---|
| `arch` | `x86_64`, `aarch64`, `arm64ec`, `riscv64`, `riscv32`, `riscv32e`, `spirv`, `wasm32` |
| `sys` | `linux`, `darwin`, `windows`, `freestanding`, `uefi`, `vulkan`, `wasi` |
| `abi` | `sysv64`, `win64`, `aapcs64`, `darwinpcs`, `aapcs64win`, `arm64ec`, `lp64`, `lp64f`, `lp64d`, `ilp32`, `ilp32f`, `ilp32d`, `ilp32e`, `shader`, `basic` |
| `env` | `gnu`, `macos`, `msvc`, `vulkan1.0` to `vulkan1.3`, `preview1` |

`mach info targets` lists the combinations that exist, with each default marked `*`.

## `[profile.<name>]`

A profile is a build variant. Every manifest declares at least one.

| Key | Required | Meaning |
|---|---|---|
| `optimize` | yes | Run the optimization passes |
| `debug` | yes | Emit debug information |
| `simd` | yes | `"scalarize"` expands a vector operation the target has no instruction for into scalar code. `"require"` refuses it |
| `default` | no | `true` marks the profile used when no `-p` is given |
| `allow` | no | Warning keys this profile silences, such as `["import.unused"]` |

## `[artifact.<name>]`

An artifact is one thing the project produces. A build compiles what its `entry` reaches through `use` and `fwd`.

| Key | Required | Meaning |
|---|---|---|
| `kind` | yes | What it is, from the table below |
| `entry` | yes | The entry source file, relative to `src` |
| `out` | all but `object` | The output path, a path template |
| `targets` | yes | The targets it builds for. `["*"]` is every declared target |
| `link` | no | The `[link.<name>]` tables it links |
| `need` | no | What is built first, as `step.<name>` or `artifact.<name>` |
| `linkage` | no | How a `bin` links: `"pie"`, `"dynamic"` or `"static"`. The default is the target's |
| `default` | no | `true` marks the artifact used when no `-a` is given |

| Kind | Produces | `{artifact.suffix}` |
|---|---|---|
| `bin` | An executable | None, or `.exe` on windows |
| `static` | A static library | `.a`, or `.lib` on windows |
| `shared` | A shared library that exports the declarations marked `#[export]` | `.so`, `.dylib` or `.dll` |
| `object` | Each module's object, left in the work directory | No `out` |
| `module` | A module a runtime loads: a [shader](shaders.md) or a WebAssembly module | `.spv` or `.wasm` |

## `[link.<name>]`

A link table names a library or file outside the project. An artifact links the ones in its `link` array, and [`#[library]`](ext-fun.md) ties an `ext fun` to one.

```toml
[link.c]
source = "system"
name   = "c"
sys    = ["linux"]
arch   = ["*"]
abi    = ["*"]
```

| Key | Required | Meaning |
|---|---|---|
| `source` | yes | `"system"` for a library found by name, `"framework"` for a macOS framework, `"local"` for a file |
| `name` | system, framework | The library's name |
| `path` | local | The file's path, a path template |
| `sys`, `arch`, `abi` | yes | The targets the entry applies to. `["*"]` is any |
| `export` | no | `true` also applies the entry to projects that depend on this one |

On linux a system library needs `env = "gnu"` on the target.

## `[step.<name>]`

A step is a command that produces files the build consumes, such as an object compiled from C.

```toml
[step.add]
argv = ["cc", "-c", "vendor/add.c", "-o", "{project.work}/vendor/add.o"]
in   = ["vendor/add.c"]
out  = ["{project.work}/vendor/add.o"]

[link.add]
source = "local"
path   = "{project.work}/vendor/add.o"
sys    = ["linux"]
arch   = ["x86_64"]
abi    = ["*"]
```

| Key | Required | Meaning |
|---|---|---|
| `argv` | yes | The command, run directly with no shell |
| `in` | yes | The input files. Globs are allowed |
| `out` | yes | The output files, as exact paths |
| `need` | no | Steps that run first, as `step.<name>` |
| `env` | no | A table of variables added to the command's environment |

A step never runs on its own. It runs when a `local` link's `path` matches its `out`, or when an artifact or step lists it in `need`. It runs again only when its inputs change.

## `[dep.<id>]`

A dependency is another project, named by its id and checked out under `dep/<id>`. [Dependencies](dependencies.md) covers the keys and the `mach dep` commands.

## Path templates

Every path is relative to the project root and written with `/`. A path and a step's `argv` may name these variables.

| Variable | Expands to |
|---|---|
| `{project.work}` | The expanded `[project].work` |
| `{target.name}`, `{target.arch}`, `{target.sys}`, `{target.abi}` | The selected target's name and its keys |
| `{profile.name}` | The selected profile's name |
| `{artifact.suffix}` | The file suffix of the artifact's kind on the target. Only in an artifact's `out` |
| `{artifact.<name>.out}` | The output path of an artifact listed in `need` |
| `{artifact.out}` | The program being launched. Only in a `runner` |

Keep `{target.name}` and `{profile.name}` in `work` so two builds never write to the same place.

The full rules are in the reference for [`mach.toml`](../language/manifest.md).
