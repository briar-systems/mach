# `mach.toml` — the project manifest

Every Mach project has a `mach.toml` at its root. It is the complete, readable
statement of what the project builds: its identity, the platforms it targets, the
artifacts it produces, the build variants it offers, and its dependencies. The
compiler finds it by walking up from the working directory until a `mach.toml` is
seen, so any subcommand run inside a project tree resolves the same manifest.

The manifest separates the three orthogonal axes of a build — *what* is built
(`[bin.*]`/`[lib.*]` artifacts), *where* it runs (`[target.*]` platforms), and
*where* outputs land (the `out`/`obj`/`ir`/`asm` path templates). Repetition
between axes is eliminated by the build engine's cartesian product, never by the
file. Nothing is inferred: every value resolves from a stanza present in the file.

## The schema

```toml
[project]
id      = "demo"            # required: root of every module path the project exposes
version = "0.1.0"           # required
src     = "src"             # source dir (default "src")
dep     = "dep"             # vendored-dependency dir (default "dep")
target  = "native"          # default target selector (default "native")
out     = "out/{target}/{profile}/bin/{name}{ext}"  # artifact path template
obj     = "out/{target}/{profile}/obj"              # intermediate-object dir template
ir      = "out/{target}/{profile}/ir"               # IR-dump dir template
asm     = "out/{target}/{profile}/asm"              # ASM-dump dir template
tests   = "out/{target}/{profile}/test/{name}"      # test dispatcher executable path template (mach test)
# optional metadata: name, description, license, authors

[target.linux]             # a platform: a fully-spelled tuple, nothing inferred
isa = "x86_64"
os  = "linux"
abi = "sysv64"

[target.windows]
isa     = "x86_64"
os      = "windows"
abi     = "win64"
ext     = ".exe"           # artifact extension (default "")
libs    = ["kernel32.dll"] # platform link overlay, inherited by every artifact
defines = []               # per-target comptime defines (#1191)

[bin.hello]                # an executable artifact, named by its table key
entry = "hello.mach"       # entry source, relative to the src dir (required)

[lib.core]                 # a library artifact
entry = "lib.mach"
kind  = "static"           # "static" (default) | "shared"

[bin.hello.target.windows] # a per-cell exception refining one artifact-target pair
entry = "hello_win.mach"   # overrides the artifact's entry for this target only
libs  = ["user32.dll"]     # appended to the merged link overlay

[profile.debug]            # a build variant
opt = 0                    # 0 (no optimization) | 1 (standard) | 2 (aggressive)

[profile.release]
opt      = 2
emit_ir  = true            # emit per-module post-pipeline IR for this profile (default false)
emit_asm = false           # emit per-module assembly for this profile (default false)

[deps.mach-std]            # a dependency: exactly one of git|path, plus ref for git
git = "https://github.com/briar-systems/mach-std"
ref = "v0.4.0"
```

## `[project]`

| Key       | Type   | Required | Default    | Meaning |
|-----------|--------|----------|------------|---------|
| `id`      | string | yes      | —          | Root segment of every module path the project exposes. A file at `<src>/foo/bar.mach` is reachable as `<id>.foo.bar`. |
| `version` | string | yes      | —          | Project version. Read by the `$project.version` comptime root. |
| `module`  | string | no       | —          | Src-relative path of the module a bare `use <id>;` / `fwd <id>;` resolves to (see [Bare project-id imports](#bare-project-id-imports)). Never inferred; a library that wants to be importable by its id declares it. A declared path naming no file is an error at build start. |
| `src`     | string | no       | `"src"`    | Source root, relative to the project root. Module paths resolve under it. |
| `dep`     | string | no       | `"dep"`    | Vendored-dependency root, relative to the project root. Each dep lives at `<dep>/<alias>/`. |
| `target`  | string | no       | `"native"` | Default target selector when `--target` is not passed. `native` resolves to the declared target whose tuple matches the host. |
| `out`     | string | no       | `"out/{target}/{profile}/bin/{name}{ext}"` | Artifact path template (see [Path templates](#path-templates)). |
| `obj`     | string | no       | `"out/{target}/{profile}/obj"` | Per-module object tree template. |
| `ir`      | string | no       | `"out/{target}/{profile}/ir"`  | Per-module IR-dump template (used when emission is on). |
| `asm`     | string | no       | `"out/{target}/{profile}/asm"` | Per-module assembly-dump template (used when emission is on). |
| `tests`   | string | no       | `"out/{target}/{profile}/test/{name}"` | Test dispatcher executable path template. `mach test` builds one executable covering every collected `test` block here, with `{name}` the product name (see below). |

Optional metadata read by the `$project.*` comptime roots but not the build:
`name`, `description`, `license`, `authors`.

## `[target.<name>]`

Each `<name>` is a selector you pass to `--target <name>` (or set as
`[project].target`). At least one target must be declared. A target is a
fully-spelled platform tuple; nothing is inferred from another key. `native` is a
reserved name — declaring `[target.native]` is an error.

| Key       | Type   | Required | Default | Meaning |
|-----------|--------|----------|---------|---------|
| `isa`     | string | yes      | —       | Instruction-set architecture. Read by `$project.target.arch`. See the accepted set below. |
| `os`      | string | yes      | —       | Operating system. Read by `$project.target.os`. See the accepted set below. |
| `abi`     | string | yes      | —       | Application binary interface (e.g. `sysv64`, `win64`). Read by `$project.target.abi`. |
| `ext`     | string | no       | `""`    | Artifact filename extension expanded by `{ext}` (e.g. `".exe"`). |
| `libs`    | array of strings | no | `[]` | Platform link overlay — external link inputs inherited by every artifact built for this target (see [Link inputs](#link-inputs)). |
| `defines` | array of strings | no | `[]` | Per-target comptime defines (#1191). Each is `NAME` (a `true` flag) or `NAME=VALUE`; readable as `$mach.build.NAME`. |

### Accepted `isa` values

| Value     | Status |
|-----------|--------|
| `x86_64`  | supported — the only fully working ISA today |
| `aarch64` | recognized; an ISA vtable exists, but codegen is not yet validated end-to-end |

### Accepted `os` values

| Value     | Status |
|-----------|--------|
| `linux`   | supported — the primary host/target |
| `windows` | supported as a cross-compilation target (PE/COFF, Win64 ABI) |
| `darwin`  | recognized; ABI/object-format vtables exist, but the toolchain is not yet validated end-to-end |

## `[bin.<name>]` / `[lib.<name>]`

Every artifact is declared explicitly and named by its table key. A `[bin.*]`
links an executable; a `[lib.*]` produces a library. The artifact name drives
`{name}` in the output templates and is read by `$bin.name`.

| Key       | Type   | Applies | Required | Default | Meaning |
|-----------|--------|---------|----------|---------|---------|
| `entry`   | string | bin, lib | yes     | —        | Entry source, relative to the project `src` dir. The entry module's FQN is `<id>.<entry without .mach>`, with `/` turned into `.`. |
| `kind`    | string | lib      | no       | `"static"` | `"static"` leaves the per-module objects as the deliverable; `"shared"` produces a shared library. |
| `out`     | string | bin, lib | no       | project `out` | Per-artifact override of the artifact path template. |
| `libs`    | array of strings | bin, lib | no | `[]` | Per-artifact link inputs, merged into the link overlay. |
| `defines` | array of strings | bin, lib | no | `[]` | Per-artifact comptime defines. |

### `[bin.<name>.target.<t>]` per-cell exceptions

A per-cell table refines one artifact for one target. It may override `entry` and
`out`, and append `libs`/`defines` at the highest precedence level. An unset key
inherits the artifact's value.

## `[profile.<name>]`

A profile is a build variant. The optimization level and the debug-emission
toggles live here because they are variant concerns.

| Key        | Type    | Default | Meaning |
|------------|---------|---------|---------|
| `opt`      | integer | — (debug) | Optimization level: `0` (no optimization beyond the always-on pipeline), `1` (standard), or `2` (aggressive). `1` and `2` currently select the same pass set; `2` is where future loop/vectorization work lands. Any other integer — or a non-integer — is a manifest error (`profile '<name>': opt must be 0, 1, or 2`). |
| `emit_ir`  | bool    | `false` | Write per-module SSA IR dumps under the `ir` template for this profile — the final post-pipeline IR the object is built from, so it varies with `opt`. |
| `emit_asm` | bool    | `false` | Write per-module assembly dumps under the `asm` template for this profile. |

A CLI `-O0` / `-O1` / `-O2` / `--release` flag overrides the selected profile's
`opt` per invocation, and `--emit-ir` / `--emit-asm` / `--no-emit-ir` /
`--no-emit-asm` override the emission toggles. Absent `--profile`/`--release`, the
first declared profile is used; with no profile declared, a `debug` default (no
optimization, no emission) applies.

## `[deps.<alias>]`

Each `<alias>` names a dependency materialised under `<dep>/<alias>/`. The build
resolves a dependency purely by vendor layout — it reads that directory's own
`mach.toml` for its `[project].id` (the head segment of the dep's module paths)
and `[project].src` (the dep's source dir). A module path whose head matches a
dep's `id` resolves into that dep's tree.

A stanza declares exactly one source key:

| Key    | Type   | Read by    | Meaning |
|--------|--------|------------|---------|
| `git`  | string | `mach dep` | Git URL to clone into `<dep>/<alias>/`. |
| `path` | string | `mach dep` | Local path to another project tree, resolved relative to this manifest's directory; never fetched. `mach dep pull` materialises it at `<dep>/<alias>/` as a relative symlink to the resolved source, so the build reaches its modules by the same vendor layout as a git dep. |
| `ref`  | string | `mach dep` | Git ref to check out (with `git`): a `tag/<name>`, `branch/<name>`, bare tag/branch, or commit SHA. An absent ref means the remote default branch. |

A registry-style `version =` is reserved and rejected. Cloning, lockfile
handling, and transitive resolution are documented in [cli.md](cli.md#mach-dep).
`mach dep` performs only plain git operations, so a checkout the user also commits
as a **submodule** composes naturally; mach never invokes `git submodule`.

A **git** dep is pinned to a resolved commit in `mach.lock`; a **path** dep has no
pinned content, so it carries no lock entry — its `path` in the manifest is the
whole record. `mach dep pull` materialises a path dep at `<dep>/<alias>/` as a
relative symlink to the resolved source and is idempotent: a stale link is
replaced, an already-correct one is left untouched, and a source that already
lives at the vendor location is a no-op. A path that does not exist, that holds no
`mach.toml`, or whose vendor location is occupied by a real directory (a stale git
checkout, foreign vendored files) is a hard error — never silent success.

### Cascading link requirements

A dependency declares its own link requirements as `[target.*].libs` (full-tuple)
or `[os.<name>].libs` (os-component) overlays, and a consumer inherits every such
lib that matches the build it is producing — so a platform link requirement (e.g.
`kernel32.dll`) lives once, in the providing dependency's manifest, and out of
every consumer. Matching is by tuple/component equality; target *names* are local
to each manifest. Within each dependency the matching os overlays precede the
matching target libs; across dependencies the order is topological then
declaration order, and the union is deduplicated by name. See
[Link inputs](#link-inputs) for the overlay merge law.

## `[os.<name>]`

An os overlay scopes a link requirement to a single tuple component — the
operating system — rather than a full `(isa, os, abi)` tuple. A build matches the
overlay iff its selected target's `os` equals `<name>`, so a requirement that
holds for every windows ISA and ABI is stated once:

```toml
[os.windows]
libs = ["kernel32.dll"]   # linked into every windows build, this project's and any consumer's
```

| Key    | Type             | Default | Meaning |
|--------|------------------|---------|---------|
| `libs` | array of strings | `[]`    | Link inputs added to every build whose os matches `<name>` (see [Link inputs](#link-inputs)). |

Os overlays cascade to consumers exactly like `[target.*].libs`. The
single-component `[isa.<name>]` and `[abi.<name>]` overlays are **reserved**:
declaring either is an error until a real case demands them.

## Bare project-id imports

A one-segment `use`/`fwd` path equal to a resolvable project id — a dependency's
`[project].id`, or the current project's own id — resolves to that project's
declared `[project].module`. So a library that sets `module = "glfw.mach"` is
imported as `use glfw;` (binding the `glfw` module) instead of by the full path to
its surface file. Longer paths are unaffected; a single segment is otherwise
unresolvable, so this is purely additive.

```toml
# in glfw's manifest:
[project]
id     = "glfw"
module = "glfw.mach"   # the surface a bare `use glfw;` binds
```

```mach
// in a consumer:
use glfw;              // binds the glfw module (glfw's [project].module)
```

A bare import of a project that declares no `module` is a resolution error naming
the fix (import a full path, or add a `module` to the project's manifest). A
declared `module` that names no file is a manifest error at build start whether or
not anything imports it. A project module that `fwd`s its own id is caught by the
existing circular-module detection.

## Path templates

Output paths come only from the declared templates, expanded over four variables:

- `{target}` — the resolved target name (never the literal `native`).
- `{profile}` — the selected profile name.
- `{name}` — the artifact name.
- `{ext}` — the target's `ext` (the empty string by default).

Manifest paths are always `/`-separated and normalized to the host separator at
the filesystem boundary, so the same manifest is portable; a literal `\` in a path
is a hard error. Two artifacts that resolve to the same `out` path collide and
fail at build start. A per-artifact or per-cell `out` overrides the project
template for that artifact.

The `tests` template is expanded the same way and locates the single test
dispatcher executable `mach test` builds (`{target}`/`{profile}`/`{ext}` resolve
as usual). `{name}` is the selected artifact's product name, falling back to the
project id for artifact-less (library) builds. The dispatcher embeds every
collected test and runs the one selected by a decimal index argument, so a
project's test build lands at, e.g., `out/linux/debug/test/mach`.

## Selection and the build matrix

A build cell is one artifact × one target × one profile.

- `mach build .` builds every declared `[bin.*]`/`[lib.*]` for the default target
  and default profile. `--all-targets` crosses every artifact with every declared
  target. `--bin <name>` / `--lib <name>` narrow to one artifact; `--target
  <name>` selects a declared target; `--profile <name>` (with `--release` sugar)
  selects a profile.
- `mach run .` and `mach test .` build exactly one artifact; with several declared
  and no `--bin`/`--lib`, they ask you to pick one, naming every candidate.

### `native` target resolution

`native` resolves the host's `(isa, os)` tuple against the **declared** targets
only — never a synthesized tuple. Exactly one host match is chosen; several
matching tuples is an ambiguity error naming the candidates; no match warns
(`no declared target matches the host (...)`) and falls back to the first declared
target so cross-only projects still build on a foreign host.

## Link inputs

A `libs` entry (at the target, artifact, or per-cell level) is either an explicit
path — a `.o` object, a `.a` archive, or a `.so` shared library, project-root-
relative or absolute — or a bare `-l`-style name resolved to a `lib<name>.o` /
`<name>.o` / `lib<name>.a` / `<name>.a`, then a shared `lib<name>.so`, at link
time. Loose `.o` objects and static `.a` archives link **statically** (a `.a`
contributes every member object); a shared `.so` is recorded as a **dynamic**
dependency (its `DT_SONAME`), bound against undefined `ext` symbols at load time
through a `PT_INTERP`/PLT. The overlays merge target < artifact < per-cell,
deduplicated by name, and join every `mach build`/`run`/`test` link alongside any
CLI `-L`/`-l`/object arguments. See
[cli.md](cli.md#static-vs-dynamic-resolution) for the full resolution rules and
[language/ext-fun.md](language/ext-fun.md#linking-external-objects) for the
`ext fun` workflow that consumes these inputs.

## Annotated example

The compiler's own manifest builds one binary (`mach`) for two targets, keeping
its output paths flat (no `{profile}` segment) so released paths stay stable:

```toml
[project]
id      = "mach"          # module-path root: this project's code is `mach.*`
name    = "Mach Compiler" # metadata
version = "1.3.0"
src     = "src"           # sources under ./src
dep     = "dep"           # vendored deps under ./dep
target  = "native"        # default target: the host-matching tuple below
out     = "out/{target}/bin/{name}{ext}"  # e.g. out/linux/bin/mach
obj     = "out/{target}/obj"
ir      = "out/{target}/ir"
asm     = "out/{target}/asm"
tests   = "out/{target}/test/{name}"      # e.g. out/linux/test/myproj

[target.linux]
isa = "x86_64"
os  = "linux"
abi = "sysv64"

[target.windows]
isa  = "x86_64"
os   = "windows"
abi  = "win64"
ext  = ".exe"            # the windows artifact is out/windows/bin/mach.exe
libs = ["kernel32.dll"]  # the windows platform link requirement

[bin.mach]
entry = "main.mach"      # entry module mach.main, at src/main.mach

[deps.mach-std]
git = "https://github.com/briar-systems/mach-std"
ref = "branch/dev"
```

`mach build .` (no `--target`) selects `linux` because `[project].target = "native"`
resolves to the host-matching tuple, compiles `src/main.mach` and its transitive
imports — including modules from `mach-std` vendored at `dep/mach-std/` — into
objects under `out/linux/obj/`, and links `out/linux/bin/mach`. `mach-std` is
realized into `dep/mach-std/` by `mach dep pull` from the manifest pin, and
`mach build .` then resolves it purely by that vendor path — no git at build time.

## See also

- [cli.md](cli.md) — the `mach` command-line reference
- [language/files.md](language/files.md) — file layout and `lib.mach` / `main.mach`
- [language/modules.md](language/modules.md) — how files map to module paths
- [language/ext-fun.md](language/ext-fun.md) — linking against external symbols
