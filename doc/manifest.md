# `mach.toml` — the project manifest

A Mach project is described by a `mach.toml` at its root: its identity, the
platforms it targets, the artifacts it produces, the build variants it offers, its
external link requirements, the build steps that produce them, and its
dependencies. Every `mach` subcommand takes the project explicitly — a directory
(whose `mach.toml` is read) or a manifest file directly — so the manifest a build
uses is never guessed from the working directory. See [cli.md](cli.md) for the
path argument.

The manifest is built from `[category.name]` tables in seven sections —
`[project]`, `[target.X]`, `[profile.X]`, `[artifact.X]`, `[link.X]`, `[step.X]`,
and `[dep.X]`. TOML itself enforces name uniqueness within a section.

## Convention, then totality

Convention covers what is *absent*: with no manifest at all, `src/` is the module
tree, `src/main.mach` the entry, and a single native debug binary the build — a
pure-mach single-binary project needs no `mach.toml`. Convention operates on absent
manifests and absent sections, never on absent fields.

A table you *declare*, you declare completely. Every field of a declared table is
required; a missing field is a strict-parse error, not a silent default. This is
the manifest twin of Mach's explicitness: there are no field defaults to memorize,
because "any" and "none" are said out loud —

- `"*"` is the explicit any-token for a filter axis or `targets` entry;
- `[]` is the explicit empty list ("none").

The sole exception is **shape-dependence**: a field whose presence follows another
value in the same table. A dependency is `git` *or* `path`; a `[link.X]` names a
`name` *or* a `path` according to its `source`. Nothing else defaults.

Unknown sections and unknown keys are always errors. A path value is always
`/`-separated; a literal `\` is rejected (`manifest paths use '/'`), so the same
manifest is portable and is normalized to the host separator at the filesystem
boundary.

### Root vs. dependency strictness

Manifest strictness is scoped to the manifest being **built**: mach enforces the
no-defaults totality rules on your project's own `mach.toml` (the root of the
build), but parses a **dependency's** `mach.toml` permissively, reading only its
export surface (project id, `export = true` link entries, and the steps those
entries demand). A dependency's own totality is enforced when that dependency is
built as a root — so a project need not wait for its dependencies to migrate, and
historical commit pins keep resolving. Unknown keys are always rejected, in a root
or a dependency.

## The schema at a glance

```toml
[project]
id      = "demo"                       # required: identifier; root of every module path
version = "0.1.0"                      # required
src     = "src"                        # required: source dir, project-root-relative
out     = "out/{target.name}/{profile.name}"  # required: output-path template root

[target.linux]                         # a platform: a fully-spelled tuple
isa = "x86_64"
os  = "linux"
abi = "sysv64"

[profile.debug]                        # a build variant
opt   = 0                              # 0 (debug pipeline) | 1 | 2 (release pipeline)
debug = true                           # emit debug info for this profile

[artifact.demo]                        # a produced artifact
kind    = "bin"                        # "bin" | "static" | "shared"
entry   = "main.mach"                  # entry source, relative to src
out     = "{project.out}/bin/demo"     # this artifact's output path
targets = ["*"]                        # which declared targets build it ("*" = all)
link    = []                           # [link.X] names this artifact links
need    = []                           # [step.X] names this artifact demands directly

[dep.std]                              # a dependency
git = "https://github.com/briar-systems/mach-std"
ref = "branch/main"
```

`[link.X]` and `[step.X]` are shown under [Link requirements](#linkx--link-requirements)
and [Build steps](#stepx--build-steps).

## `[project]`

| Key       | Type   | Meaning |
|-----------|--------|---------|
| `id`      | string | Root segment of every module path the project exposes: a file at `<src>/foo/bar.mach` is reachable as `<id>.foo.bar`. Must be a plain identifier — letters, digits, `_`, `-` — since it names the dependency store and keys step stamp files. Read by `$project.id`. |
| `version` | string | Project version. Read by `$project.version`; the source of truth a Go-style `tag/<version>` acquisition checks. |
| `src`     | string | Source root, project-root-relative. Module paths resolve under it. |
| `out`     | string | The output-path template root, referenced as `{project.out}` by artifact `out`, step paths, and `cmd`s. Expanded over `{target.name}`/`{profile.name}` (see [Path templates](#path-templates)). |

`[project]` is exactly these four keys; `name`, `description`, and any other key
are unknown-key errors in a root manifest.

## `[target.<name>]`

Each `<name>` is a selector you pass to `--target <name>`. A target is a
fully-spelled platform tuple; nothing is inferred from another key. `native` is a
reserved name — declaring `[target.native]` is an error, because `native` resolves
to whichever *declared* target matches the host.

| Key   | Required | Meaning |
|-------|----------|---------|
| `isa` | yes      | Instruction-set architecture. Read by `$project.target.arch`. |
| `os`  | yes      | Operating system. Read by `$project.target.os`. |
| `abi` | yes      | Application binary interface. Read by `$project.target.abi`. |
| `of`  | no       | Object-format override; defers to the os's format when omitted (the one optional target key). See [Object-format override](#object-format-override). |

### Accepted tuple values

| Axis  | Values |
|-------|--------|
| `isa` | `x86_64`, `aarch64`, `riscv64` |
| `os`  | `linux`, `windows`, `darwin`, `freestanding` |
| `abi` | `sysv64`, `win64`, `aapcs64`, `lp64` |

`x86_64`/`linux`/`sysv64` is the primary host and target. `aarch64`-linux builds
and runs natively in CI on every PR; `riscv64`-linux runs under qemu and
self-hosts (#1852). `windows` is a supported cross-compilation target (PE/COFF,
Win64 ABI). `darwin`'s ABI and object-format support exist but the toolchain is not
yet validated end-to-end. `freestanding` targets a raw flat image with no OS
runtime.

A value outside its axis's set is a strict-parse error, so a typo is caught rather
than silently never matching.

### Object-format override

`of` overrides the object format an OS implies. Each os has a default format —
`linux` → `elf`, `windows` → `coff`, `darwin` → `macho`, `freestanding` → `raw` —
and `of` names a different one from the same closed set: `elf`, `coff`, `macho`,
`raw`. It is the only optional key on a target; omit it to take the os default.

```toml
[target.metal]
isa = "x86_64"
os  = "freestanding"   # os default object format is "raw"
abi = "sysv64"
of  = "elf"            # override: emit an ELF object instead
```

## `[profile.<name>]`

A profile is a build variant. The optimization level and debug-emission toggle
live here because they are variant concerns.

| Key     | Type    | Meaning |
|---------|---------|---------|
| `opt`   | integer | Optimization level: `0` selects the debug pipeline (the always-on passes only), `1` and `2` select the release pipeline. `1` and `2` currently share a pass set; `2` is where future loop/vectorization work lands. Any other integer — or a non-integer — is a manifest error. |
| `debug` | bool    | Emit debug info (DWARF on ELF/Mach-O, CodeView on COFF) for this profile. Gates emission only, never the optimizer, so a `release` profile can keep symbols with `debug = true`. A non-boolean is a manifest error. |

Both keys are required in a declared profile. Emission of the human-readable IR and
assembly side-artifacts is **not** a profile concern — it is controlled only by the
`--emit-ir` / `--emit-asm` CLI flags (see [cli.md](cli.md)).

The CLI selects and overrides at invocation time: `--profile <name>` picks the
profile; `-g` forces `debug` on for one build regardless of the profile's key
(precedence `-g` > profile > off — there is no flag to force it off over a
`debug = true` profile; edit the manifest or pick another profile). Absent
`--profile`, the first declared profile is used.

## `[artifact.<name>]`

Every artifact is declared explicitly and named by its table key. `$project.name`
reads the selected artifact's name.

| Key       | Required | Meaning |
|-----------|----------|---------|
| `kind`    | yes | `"bin"`, `"static"`, or `"shared"` (see below). |
| `entry`   | yes | Entry source, relative to the project `src` dir (e.g. `main.mach` for `src/main.mach`). The entry module's FQN is `<id>.<entry without .mach>`, `/` turned into `.`. |
| `out`     | yes | This artifact's output path, a template (see [Path templates](#path-templates)). An executable extension, where wanted, is written literally into this path. |
| `targets` | yes | Array of declared target names this artifact builds for; `["*"]` means every declared target. |
| `link`    | yes | Array of `[link.X]` names this artifact links (see below). `[]` for none. |
| `need`    | yes | Array of `[step.X]` names this artifact demands directly, for step outputs that are not themselves link inputs. `[]` for none. |

- **`bin`** links an executable at the resolved `out` path.
- **`static`** materialises a real `ar` archive at the resolved `out` path — the
  per-module objects with an archive symbol index, the deliverable a consumer links
  as a `.a` (#1997).
- **`shared`** is reserved for a shared-library deliverable; its emission is phase 2
  (#1980).

Per-target extension or per-target entry is not a per-cell exception table — it is a
second artifact stanza, so the condition stays visible like everything else.

## `[link.<name>]` — link requirements

A `[link.X]` is a named external link requirement. Artifacts reference entries by
name in their `link = [...]`; an entry whose filters do not match the build cell is
skipped. An entry with `export = true` also applies to any project that links this
project's modules, so a platform link requirement lives once — in the manifest that
needs it — and cascades to consumers. A standalone build and a consumed build use
the same entries, so nothing behaves differently as a dependency.

| Key      | Required | Meaning |
|----------|----------|---------|
| `source` | yes | `"system"` (a system library resolved by name), `"framework"` (a macOS framework), or `"local"` (a file on disk). |
| `name`   | shape | Library/framework name — required for `source = "system"`/`"framework"`, forbidden for `"local"`. |
| `path`   | shape | File path — required for `source = "local"`, forbidden otherwise. A template (see below). |
| `os`     | yes | Filter axis: a canonical `os` value, `"*"` (any), an array of values, or `[]` (none). |
| `isa`    | yes | Filter axis over `isa`, same forms. |
| `abi`    | yes | Filter axis over `abi`, same forms. |
| `export` | yes | `true` cascades this entry to consumers; `false` keeps it to this project's own builds. |

The `os`/`isa`/`abi` axes select the build cells an entry applies to. Each takes a
single canonical value, `"*"` for any, or an array — `os = "linux"` and
`os = ["linux"]` filter identically. `[]` matches nothing (an entry deliberately
switched off). A non-canonical spelling is a strict-parse error. An entry applies
to a cell when all three axes match.

A `local` entry's `path` must, at build time, either match a `[step.X]`'s `out`
(which demands that step) or already exist on disk — anything else is an up-front
error, so a typo never silently drops an input.

Whether an input links **statically** or **dynamically** follows the resolved file
— a loose `.o` or static `.a` links statically; a shared `.so` is recorded as a
dynamic dependency by its `DT_SONAME`. See
[cli.md](cli.md#static-vs-dynamic-resolution) for the resolution rules and
[language/ext-fun.md](language/ext-fun.md#linking-external-objects) for the
`ext fun` workflow that consumes these inputs.

## `[step.<name>]` — build steps

A step is a command, make-recipe style, that produces files a build consumes
(typically a `local` link input, e.g. a vendored-C object). `<name>` must be a
plain identifier — it keys the step's stamp file.

| Key    | Required | Meaning |
|--------|----------|---------|
| `cmd`  | yes | One command string, run through the platform shell (`sh -c` on posix, `cmd.exe /C` on windows) from the project root. Pipe, `&&`, or invoke a script freely. Templates expand in it. |
| `in`   | yes | Declared input file list. Accepts globs (`*`, `**`), expanded sorted for a stable fingerprint; a glob that matches nothing is a hard error. |
| `out`  | yes | Declared output file list. Concrete paths only — a glob here is an error, since the demand match and cache key expand `out` verbatim. |
| `need` | yes | Array of other `[step.X]` names this step must run after (explicit ordering; cycles error). `[]` for none. |

Steps carry **no filters** and **never run automatically**. A step runs only when
**demanded**:

- by a selected `[link.X]` whose `local` `path` matches the step's `out`;
- by another step's `need`;
- by an artifact's `need` (for outputs that are not link inputs).

Because a step has no filter of its own, the condition for running it lives in the
link entry that demands it: on a build cell where that entry filters out, the step
is never demanded and never runs.

A step is cached by content: its `in` contents plus its expanded `cmd` fingerprint
the step (the query engine's `Q_LINK_CONFIG` pattern). An unchanged step whose
outputs still exist is skipped; change an input or the command and it re-runs.

**Output homing.** `{project.out}` resolves to the **root** project's expanded
`out` in every manifest of the closure. A dependency's step outputs land in the
consumer's output tree — exactly as a dependency's compiled modules do — and the
dependency's own checkout is never written to.

## `[dep.<alias>]`

Each `<alias>` names a dependency materialised under `dep/<alias>/`. The build
resolves a dependency by vendor layout: it reads that directory's own `mach.toml`
for its `[project].id` (the head segment of the dep's module paths) and `src`, and
a module path whose head matches a dep's `id` resolves into that dep's tree.

A stanza declares exactly one source key:

| Key    | Meaning |
|--------|---------|
| `git`  | Git URL to clone into `dep/<alias>/`. Requires `ref`. |
| `path` | Local project tree, resolved relative to this manifest's directory; never fetched. `mach dep pull` materialises it at `dep/<alias>/` as a relative symlink. Forbids `ref`. |
| `ref`  | Git ref to check out (git only): `tag/<name>`, `branch/<name>`, a bare tag/branch, or a commit SHA. |

`git` and `path` are mutually exclusive and exactly one is required. A
registry-style `version =` is reserved and rejected. Cloning, lockfile handling,
and transitive resolution are documented in [cli.md](cli.md#mach-dep); `mach dep`
performs only plain git operations, so a checkout committed as a git **submodule**
composes naturally. A git dep is pinned to a resolved commit in `mach.lock`; a path
dep carries no lock entry (its `path` is the whole record).

Dependencies are exactly today's system: git/path deps, an alias-keyed flat `dep/`
store, `branch`/`tag`/`commit` refs, transitive pull, `mach.lock` as-is. Every
transitively fetched dependency registers its `[project].id` into a single flat id
registry, so a dependency's own surface dependencies resolve in consumers. **The
same id reached from two different sources or refs anywhere in the closure is a
hard error** naming the requirers — the fix is to fork or upstream, never to
version-split inside one build.

A dependency's export surface — all a consumer sees — is its source module tree
(addressed by the dep's id), its `export = true` link entries, and the steps those
entries demand. Nothing else in a dependency's manifest applies to consumers.

## Path templates

Output paths and `cmd`s come only from the declared templates, expanded over a
closed, final set of three variables:

- `{project.out}` — the root project's expanded `[project].out`.
- `{target.name}` — the resolved target name (never the literal `native`).
- `{profile.name}` — the selected profile name.

There are no `{name}`/`{ext}` or bare `{target}`/`{profile}` aliases. An
unresolvable `{...}` reference, or an unterminated `{`, is a strict-parse error.
`{project.out}` is not available inside `[project].out` itself (it would be
self-referential). Two artifacts that resolve to the same `out` path collide and
fail at build start.

## Selection and the build matrix

A build cell is one artifact × one target × one profile.

- `mach build <path>` builds every declared artifact whose `targets` includes the
  selected target, for the default profile. `--all-targets` crosses every artifact
  with every target in its `targets`. `--bin <name>` / `--lib <name>` narrow to one
  artifact; `--target <name>` selects a declared target; `--profile <name>` selects
  a profile.
- `mach run <path>` and `mach test <path>` build exactly one artifact; with several
  declared and no `--bin`/`--lib`, they ask you to pick one, naming every candidate.
- `mach test` links the union of all artifacts' referenced entries plus exported
  dependency entries, filtered to the native target (tests run on native hardware
  only). If two artifacts' objects collide on symbols in that union, that is an
  honest link error — restructure the entries.

### `native` target resolution

`native` resolves the host's `(isa, os)` against the **declared** targets only —
never a synthesized tuple. Exactly one host match is chosen; several matching tuples
is an ambiguity error naming the candidates; no match warns and falls back to the
first declared target, so a cross-only project still builds on a foreign host.

## Worked example: a consumer of C bindings and vendored C

A project that uses a system-lib binding (`glfw`) and a vendored-C library
(`miniz`), building a native binary and cross-compiling to windows. The platform
shim is built by steps and linked through `local` entries; the OS-specific shims
are gated by their link entries' `os` axis, so the x11 step runs on a linux build
and never on a windows one.

```toml
[project]
id      = "demo"
version = "0.1.0"
src     = "src"
out     = "out/{target.name}/{profile.name}"

[dep.std]
git = "https://github.com/briar-systems/mach-std"
ref = "tag/v0.17.0"

[dep.glfw]
git = "https://github.com/briar-systems/mach-glfw"
ref = "tag/v0.2.1"

[dep.mz]
git = "https://github.com/briar-systems/mach-miniz"
ref = "tag/v1.0.3"

[link.shim]
source = "local"
path   = "{project.out}/obj/platform/shim.o"
os     = "*"
isa    = "*"
abi    = "*"
export = false

[link.shim-x11]
source = "local"
path   = "{project.out}/obj/platform/x11.o"
os     = "linux"
isa    = "*"
abi    = "*"
export = false

[link.shim-win32]
source = "local"
path   = "{project.out}/obj/platform/win32.o"
os     = "windows"
isa    = "*"
abi    = "*"
export = false

[link.gl]
source = "system"
name   = "GL"
os     = "linux"
isa    = "*"
abi    = "*"
export = false

[artifact.demo]
kind    = "bin"
entry   = "main.mach"
out     = "{project.out}/bin/demo"
targets = ["linux", "windows"]
link    = ["shim", "shim-x11", "shim-win32", "gl"]
need    = []

[step.shim]
cmd  = "cc -c -O2 -fPIC -Ivendor/platform -o {project.out}/obj/platform/shim.o vendor/platform/shim.c"
in   = ["vendor/platform/shim.c"]
out  = ["{project.out}/obj/platform/shim.o"]
need = []

[step.shim-x11]
cmd  = "cc -c -O2 -fPIC -Ivendor/platform -o {project.out}/obj/platform/x11.o vendor/platform/x11.c"
in   = ["vendor/platform/x11.c"]
out  = ["{project.out}/obj/platform/x11.o"]
need = []

[step.shim-win32]
cmd  = "cc -c -O2 -fPIC -Ivendor/platform -o {project.out}/obj/platform/win32.o vendor/platform/win32.c"
in   = ["vendor/platform/win32.c"]
out  = ["{project.out}/obj/platform/win32.o"]
need = []

[target.linux]
isa = "x86_64"
os  = "linux"
abi = "sysv64"

[target.windows]
isa = "x86_64"
os  = "windows"
abi = "win64"

[profile.debug]
opt   = 0
debug = true

[profile.release]
opt   = 2
debug = false
```

The `gl` and `shim-x11` entries carry `os = "linux"`, so on a windows build cell
they filter out (and `shim-x11`'s step is never demanded); `shim-win32` carries
`os = "windows"` and applies only there. The unconditional `shim` entry (`os = "*"`)
applies to both.

## Worked example: a C-binding dependency's export

`mach-glfw` exports only its `system`/`framework` link entries — a consumer that
imports its modules inherits every `export = true` entry that matches the build:

```toml
[project]
id      = "glfw"
version = "0.2.1"
src     = "src"
out     = "out/{target.name}/{profile.name}"

[link.glfw]
source = "system"
name   = "glfw"
os     = "*"
isa    = "*"
abi    = "*"
export = true

[link.cocoa]
source = "framework"
name   = "Cocoa"
os     = "darwin"
isa    = "*"
abi    = "*"
export = true
```

## Worked example: a vendored-C dependency

`mach-miniz` exports one `local` entry whose path is produced by a step; importing
its surface pulls the entry, and the entry's path demands the step in the
consumer's output tree:

```toml
[project]
id      = "mz"
version = "1.0.3"
src     = "src"
out     = "out/{target.name}/{profile.name}"

[link.miniz]
source = "local"
path   = "{project.out}/obj/miniz/miniz.o"
os     = "*"
isa    = "*"
abi    = "*"
export = true

[step.miniz]
cmd  = "cc -c -O2 -fPIC -Ivendor/miniz -o {project.out}/obj/miniz/miniz.o vendor/miniz/miniz.c"
in   = ["vendor/miniz/*.c", "vendor/miniz/*.h"]
out  = ["{project.out}/obj/miniz/miniz.o"]
need = []
```

## The compiler's own manifest

Mach builds itself from a manifest that declares one binary for six targets and
depends on `mach-std`:

```toml
[project]
id      = "mach"
version = "3.1.0"
src     = "src"
out     = "out/{target.name}/{profile.name}"

[target.linux-x86_64]
isa = "x86_64"
os  = "linux"
abi = "sysv64"

[target.windows-x86_64]
isa = "x86_64"
os  = "windows"
abi = "win64"

[profile.debug]
opt   = 0
debug = false

[profile.release]
opt   = 2
debug = false

[artifact.mach]
kind    = "bin"
entry   = "main.mach"
out     = "bin/mach"
targets = ["*"]
link    = []
need    = []

[dep.mach-std]
git = "https://github.com/briar-systems/mach-std"
ref = "branch/main"
```

(The full manifest declares all six supported targets.) `mach build .` selects the
host-matching target via `native`, compiles `src/main.mach` and its transitive
imports — including modules from `mach-std` vendored at `dep/mach-std/` — and links
`out/<target>/<profile>/bin/mach`. `mach-std` is realized into `dep/mach-std/` by
`mach dep pull` from the manifest pin; the build then resolves it purely by that
vendor path, with no git at build time.

## See also

- [cli.md](cli.md) — the `mach` command-line reference
- [language/files.md](language/files.md) — file layout and `lib.mach` / `main.mach`
- [language/modules.md](language/modules.md) — how files map to module paths
- [language/ext-fun.md](language/ext-fun.md) — linking against external symbols
