# `mach.toml` — the project manifest

Every Mach project has a `mach.toml` at its root. It declares the project's
identity, one or more build targets, and its dependencies. The compiler finds
it by walking up from the working directory until a `mach.toml` is seen, so any
subcommand run inside a project tree resolves the same manifest.

This page is the authoritative reference for the manifest schema as the current
compiler reads it. Keys the parser does not consume are noted as **informational**
— they are accepted (TOML still parses) but have no effect on the build today.

## Tables

A manifest has three tables:

- `[project]` — identity and directory layout (required).
- `[targets.<name>]` — one entry per build target (at least one required).
- `[deps.<alias>]` — one entry per dependency (optional).

## `[project]`

| Key       | Type   | Required | Default    | Meaning |
|-----------|--------|----------|------------|---------|
| `id`      | string | yes      | —          | Root segment of every module path the project exposes. A file at `<dir_src>/foo/bar.mach` is reachable as `<id>.foo.bar`. |
| `dir_src` | string | no       | `"src"`    | Source root, relative to the project root. Module paths resolve under it. |
| `dir_dep` | string | no       | `"dep"`    | Vendored-dependency root, relative to the project root. Each dep lives at `<dir_dep>/<alias>/`. |
| `dir_out` | string | no       | `"out"`    | Build-output root, relative to the project root. Objects and binaries are written under it. The legacy key `out` is accepted as a fallback when `dir_out` is absent. |
| `target`  | string | no       | `"native"` | Default target selector when `--target` is not passed. `"native"` / `"host"` resolve to the `[targets.<name>]` entry whose `os`/`isa` match the build host. |

Informational (parsed by TOML, not read by the build): `name`, `version`. They
document the project but do not affect compilation.

## `[targets.<name>]`

Each `<name>` is a selector you pass to `--target <name>` (or set as
`[project].target`). At least one target must be present, or the build fails
with `mach.toml has no [targets.<name>] entries`.

| Key          | Type   | Required | Default  | Meaning |
|--------------|--------|----------|----------|---------|
| `os`         | string | yes      | —        | Target operating system. See the accepted set below. |
| `isa`        | string | yes      | —        | Target instruction-set architecture. See the accepted set below. |
| `entrypoint` | string | yes      | —        | Entry source file, relative to `<dir_src>` (e.g. `"main.mach"`). The entry module's FQN is `<id>.<entrypoint without .mach>`, with `/` turned into `.`. Required for **every** target, including library targets. |
| `binary`     | string | yes      | —        | Output path of the linked binary, relative to `<dir_out>` (e.g. `"linux/bin/mach"`). Required for **every** target, even in library mode where no binary is linked. |
| `artifacts`  | string | no       | `<name>` | Subdirectory under `<dir_out>` holding the per-module object tree (`obj/`) and any `--emit-asm` / `--emit-ir` output. Defaults to the target name; may be nested (e.g. `"smach/linux"`). |
| `mode`       | string | no       | `"executable"` | `"executable"` links the per-module objects into a static binary; `"library"` stops at the objects (no link). Any value other than `"library"` is treated as `"executable"`. |
| `libs`       | array of strings | no | `[]` | Project-level external link inputs. Each entry is either an explicit object/archive path (a `.o` or `.a`, project-root-relative or absolute) or a bare `-l`-style name resolved to a `lib<name>.o` / `<name>.o` / `lib<name>.a` / `<name>.a` at link time. These join every `mach build`/`run`/`test` link in addition to any CLI `-L`/`-l`/object arguments. A non-array value, or a non-string element, is a manifest error. The alias `link` is accepted when `libs` is absent. |

Informational (parsed by TOML, not read by the build): `abi`. The ABI is derived
from `os` (Linux/macOS → SysV, Windows → Win64), so the key documents intent but
does not change the build.

Loose `.o` relocatable objects and static `.a` archives are linked; a `.a`
contributes every member object. Shared libraries (`.so`) are not yet supported.
See [language/ext-fun.md](language/ext-fun.md#linking-external-objects) for the
`ext fun` workflow that consumes these inputs, and [cli.md](cli.md#external-link-inputs)
for the equivalent command-line flags.

### Accepted `os` values

| Value     | Status |
|-----------|--------|
| `linux`   | supported — the only fully working host/target today |
| `darwin`  | recognized; ABI/object-format vtables exist, but the toolchain is not yet validated end-to-end |
| `windows` | recognized; ABI/object-format vtables exist, but the toolchain is not yet validated end-to-end |

Any other value resolves to "unknown" and fails the build with
`unsupported operating system`.

### Accepted `isa` values

| Value     | Status |
|-----------|--------|
| `x86_64`  | supported — the only fully working ISA today |
| `aarch64` | recognized; an ISA vtable exists, but codegen is not yet validated end-to-end |

Any other value resolves to "unknown" and fails target selection.

### Accepted `mode` values

| Value        | Effect |
|--------------|--------|
| `executable` | default; link the objects into a static binary at `<dir_out>/<binary>` |
| `library`    | leave the per-module objects as the deliverable; no binary is linked |

The single fully-supported triple today is `os = "linux"`, `isa = "x86_64"`
(SysV ABI, ELF objects). The other recognized values pass manifest parsing and
target composition but are not exercised by the working bootstrap.

## `[deps.<alias>]`

Each `<alias>` names a vendored dependency under `<dir_dep>/<alias>/`. The
compiler resolves a dependency by reading that directory's own `mach.toml`
(its `[project].id` becomes the head segment of the dep's module paths, and its
`[project].dir_src` locates the dep's sources). A module path whose head matches
a dep's `id` resolves into that dep's tree.

The keys written under `[deps.<alias>]` are **not** consumed by the build's
dependency resolution — resolution is purely by vendor layout. They are read
only by `mach dep list`, which prints `path=<…>` if present, else `source=<…>`:

| Key      | Type   | Read by                 | Meaning |
|----------|--------|-------------------------|---------|
| `path`   | string | `mach dep list` display | Local or remote location of the dependency. Preferred over `source` when both are present. |
| `source` | string | `mach dep list` display | Alternate location field, shown when `path` is absent. |

Other keys seen in practice — `type`, `version` — are informational: parsed by
TOML, displayed by nothing, and not read by the build. They record provenance
for the (not-yet-implemented) `mach dep add`/`sync`/`vendor` actions.

The dependency at `<dir_dep>/<alias>/` must itself be a valid project (have its
own `mach.toml` with a `[project].id`), or the build fails resolving the dep.

## Annotated example

```toml
[project]
id      = "mach"          # module-path root: this project's code is `mach.*`
name    = "Mach Compiler" # informational
version = "1.0.0"         # informational
dir_src = "src"           # sources under ./src
dir_dep = "dep"           # vendored deps under ./dep
dir_out = "out"           # build output under ./out
target  = "native"        # default target: the host-matching entry below

[targets.linux]
os         = "linux"          # target OS (linux | darwin | windows)
isa        = "x86_64"         # target ISA (x86_64 | aarch64)
abi        = "sysv64"         # informational; derived from os
mode       = "executable"     # executable (default) | library
entrypoint = "main.mach"      # entry module: mach.main, under src/
artifacts  = "linux"          # objects under out/linux/obj/...
binary     = "linux/bin/mach" # linked binary at out/linux/bin/mach
# libs     = ["build/libfoo.o", "bar"]  # optional external link inputs

[deps.mach-std]
type    = "remote"                                # informational
path    = "https://github.com/octalide/mach-std"  # shown by `mach dep list`
version = "branch/dev"                            # informational
```

With this manifest, `mach build` (no `--target`) selects `linux` because
`[project].target = "native"` resolves to the host-matching entry, compiles
`src/main.mach` and its transitive imports — including modules from `mach-std`
vendored at `dep/mach-std/` — into objects under `out/linux/obj/`, and links
`out/linux/bin/mach`.

## See also

- [cli.md](cli.md) — the `mach` command-line reference
- [language/files.md](language/files.md) — file layout and `lib.mach` / `main.mach`
- [language/modules.md](language/modules.md) — how files map to module paths
- [language/ext-fun.md](language/ext-fun.md) — linking against external symbols
