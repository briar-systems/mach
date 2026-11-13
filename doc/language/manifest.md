# `mach.toml` — the project manifest

A Mach project is described by a `mach.toml` at its root: its identity, the
platforms it targets, the artifacts it produces, the build variants it offers, its
external link requirements, the build steps that produce them, and its
dependencies. Every `mach` subcommand takes the project explicitly — a directory
(whose `mach.toml` is read) or a manifest file directly — so the manifest a build
uses is never guessed from the working directory. `mach help <command>`
describes the path argument.

The manifest is built from `[category.name]` tables in seven sections —
`[project]`, `[target.X]`, `[profile.X]`, `[artifact.X]`, `[link.X]`, `[step.X]`,
and `[dep.X]`. TOML itself enforces name uniqueness within a section.

## Totality

A manifest is required. A project directory without one does not build:

```
error[project.no_manifest]: no mach.toml in the project directory
```

Nothing is inferred from the directory layout. `mach init` writes a complete
manifest so a new project never starts from that error (`mach help init`).

Every key has one rule, the same in every manifest, whoever reads it. A key may
be left out only when leaving it out safely means that it does not apply: no
entries, nothing exported, no limit, no extensions selected. Every other key is
required, and a missing one is refused, never replaced by a silent default. A
target must name its operating system, since nothing can be assumed in its
place, but it need not name extensions. An artifact need not list what it links
or needs. The explicit forms remain for when they are meant:

- `"*"` is the explicit any-token for a filter axis or `targets` entry;
- `[]` is the explicit empty list ("none").

One further case is allowed: a key whose presence is decided by another value of
the same table. A dependency is `git` *or* `path`; a `[link.X]` names a `name`
*or* a `path` according to its `source`.

The tables below mark each key **yes** (required), **shape** (decided by another
value of the table) or with what leaving it out means. Every key also has one
type. A value of the wrong type is refused where it is written, naming the type
the key takes, and a string key never takes the empty string:

```
error[manifest.value_type]: mach.toml: [target.windows-x86_64].stack_reserve must be an integer
```

A manifest refusal points at what it refuses, after the message: the value it
rejects, the key token of a name or key it rejects, the table a required key is
missing from, or the start of the file when a required table is missing from
it, in the manifest that states it, a dependency's own included.
[`--diagnostics json`](diagnostics-json.md#failures)
carries the same place as the failure's `primary` span.

```
error[manifest.invalid_value]: mach.toml: profile 'debug': simd must be "scalarize" or "require" (got "fast")
  --> mach.toml:15:8
   |
15 | simd = "fast"
   |        ^^^^^^
```

Unknown sections and unknown keys are always errors. A path value is always
`/`-separated; a literal `\` is rejected (`manifest paths use '/'`), so the same
manifest is portable and is normalized to the host separator at the filesystem
boundary.

### Dependency manifests

A dependency's `mach.toml` is held to exactly the rules of the project being
built. It is read once, and an unknown, removed, mistyped or missing key in it
fails the consumer's build naming the dependency:

```
error[manifest.unknown_key]: dep 'std': mach.toml: unknown key 'bogus' in [project]
error[manifest.missing_required]: dep 'gfx': mach.toml: missing [profile] table; a build needs an explicit [profile.<name>] declaring optimize, debug and simd
```

What a consumer *uses* from a dependency's manifest is its export surface: the
project id, its source modules, which a consumer imports by source path (see
[modules.md](modules.md#a-use-path-is-a-project-id-and-a-source-path)), its
`export = true` link entries, the steps those entries demand, and what its
`export = true` library artifact requires — the artifacts and steps named in
that artifact's `need` (see
[Dependency requirements travel](#dependency-requirements-travel)). Nothing else
travels: a `bin` artifact's `need`, any other library's, and every other
requirement of the dependency stay its own.

A dependency declares its `[profile.*]` tables like any project, since it also
builds on its own, but they are never read to build the consumer, which resolves
its own profile and builds everything with it. A dependency's
`[target.*]` tables are read for exactly one purpose: the targets its travelling
requirements name, `env` included, since those artifacts are built for the
targets the dependency declares for them. No other `[target.*]` entry is read.

## The schema at a glance

```toml
[project]
id      = "demo"                       # required: identifier; root of every module path
version = "0.1.0"                      # required
mach    = "^5.3"                       # required: the compiler range
src     = "src"                        # required: source dir, project-root-relative
work    = "out/{target.name}/{profile.name}"  # required: the work directory template

[target.linux]                         # a platform: a fully-spelled tuple
arch = "x86_64"
sys  = "linux"
abi = "sysv64"
# runner = ["wine", "{artifact.out}"]  # optional: the command run and test launch through

[profile.debug]                        # a build variant; every manifest declares at least one
optimize = false                       # run the optimization pass set
debug = true                           # emit debug info for this profile
simd  = "scalarize"                    # SIMD lever: "scalarize"
# relax = ["float-reassoc"]            # optional: departures from exact semantics
# linker_relax = false                 # optional: whether the linker may relax the code
# allow = ["import.unused"]            # optional: warning keys this profile silences

[artifact.demo]                        # a produced artifact
kind    = "bin"                        # "bin" | "static" | "shared" | "object" | "module"
# linkage = "static"                   # optional: "pie" | "dynamic" | "static", bin only
entry   = "main.mach"                  # entry source, relative to src
out     = "{project.work}/bin/demo{artifact.suffix}"  # output path, under the work directory
targets = ["*"]                        # which declared targets build it ("*" = all)
# link  = ["kernel32"]                 # optional: [link.X] names this artifact links
# need  = ["step.generate"]            # optional: step.X / artifact.X requirements
# subsystem = "windows"                # optional: the PE subsystem, as mink spells it
# icon = "assets/demo.ico"             # optional: PE executable icon
# manifest = "assets/demo.manifest"    # optional: PE application manifest

[dep.std]                              # a dependency
git = "https://github.com/briar-systems/mach-std"
ref = "branch/main"
```

`[link.X]` and `[step.X]` each have their own section below.

## `[project]`

| Key       | Type   | Meaning |
|-----------|--------|---------|
| `id`      | string | Root segment of every module path the project exposes: a file at `<src>/foo/bar.mach` is reachable as `<id>.foo.bar`. Must be a plain identifier — letters, digits, `_`, `-` — since it names the dependency store and keys step stamp files. Read by `$project.id`. |
| `version` | string | Project version. Read by `$project.version` and `$project.version.{major,minor,patch}`, and stamped into a Windows executable's version resource. |
| `src`     | string | Source root, project-root-relative. Module paths resolve under it. |
| `work`    | string | The work directory template: everything a build writes besides its artifacts, referenced as `{project.work}` by artifact `out`, step `argv`, `in` and `out`, and local link paths. Expanded over `{target.name}`/`{target.arch}`/`{target.sys}`/`{target.abi}`/`{profile.name}` (see [Path templates](#path-templates)). `-w` names another work directory for one build or test. |
| `mach`    | string | The compiler versions this project builds with, as a [version range](#version-ranges) (`"^5.3"`). Required in every manifest, a dependency's included. See [Compiler range](#compiler-range). |
| `out`     | removed | Refused as a removed key: the work directory is `work`. Its refusal carries the rename as a fix. |

`[project]` is exactly the five keys with a type, and every one is required. Any other
key, `name` and `description` included, is an unknown-key error
(`mach.toml: unknown key 'name' in [project]`). The previous major's `out` is
refused by name, with the rename to `work` as its fix, and so is `{project.out}`
in any template, with `{project.work}` as its fix. `[profile.<name>]`
likewise carries no `emit_ir` or `emit_asm`: emission is `--emit ir` and `--emit asm` on
the command line.

### The work directory

Everything a build and its cache write besides its artifacts lands under the
expanded `work`, in one layout:

| Path | Holds |
| --- | --- |
| `obj/` | one object per module, each carrying its cache key; it is the [object cache](#stepname--build-steps) and is read by developers and tooling too |
| `ir/`, `asm/` | the human-readable views `--emit ir` and `--emit asm` write |
| `.cache/` | compiler-only state, read and written by nothing but the compiler |
| `.cache/steps/` | one fingerprint stamp per [build step](#stepname--build-steps) |
| `.stage/` | build step scratch space, one directory per step, reset before the step runs |
| `test/<artifact>/dispatch.o` | the test dispatcher object of a tested artifact |
| `test/<artifact>/<artifact>` | the test dispatcher executable |
| `test/<artifact>/log/` | a failing test's captured output |
| `dep/<id>/` | a dependency's artifact outputs (see [Dependency requirements travel](#dependency-requirements-travel)) |

Test objects sit in `obj/` beside the module objects, as
`obj/<project>/<module>.test.o`. An artifact goes wherever its own `out` names,
under the work directory when it is written from `{project.work}`.

`mach clean` removes `obj/`, `ir/`, `asm/`, `.cache/`, `.stage/`, `test/` and
`dep/` along with every artifact output, for every cell `mach build --all` plans,
dependency requirement cells included, so the build after it is a cold one that
reuses nothing. It plans as that build does, so a manifest the build refuses is
refused by `mach clean` too. A dependency never pulled was never built, so it
adds nothing to remove and `mach clean` does not refuse it. A step output may not
name a path inside `obj/<project.id>/`, `.cache/` or `.stage/` (see
[build steps](#stepname--build-steps)).

### Compiler range

`mach` states which compilers a project builds with, and every command that
reads the dependency closure (build, test, check, `mach dep verify`, the
language server) checks it for the root and for every realized dependency. A
compiler outside any of those ranges is refused once, with every unmet
requirement and the chain that states it, pointing at the first unmet range in
the manifest that states it:

```
error[mach.version_unaccepted]: this is mach 5.2.1, and the dependency closure does not accept it:
    app (mach.toml) requires mach ^5.3
    app -> gfx -> glfw requires mach >=5.4, <6
 --> mach.toml:2:11
  |
2 | mach    = "^5.3"
  |           ^^^^^^
```

Every manifest must state `mach`, a dependency's included. One without it is
refused like any missing required key (`mach.toml: [project] is missing required
key 'mach'`), and for a dependency the build fails naming it. `mach init` writes the same range. It is the oldest
release of the running compiler's major that reads the key: `^5.3` for every
5.x compiler, since 5.3.0 is the first release that accepts `mach`, and `^N.0`
for a later major N, since a caret cannot span majors. The range depends only on
the running major, so two authors on one project write the same line.

The compiler's version is the last release it was built from. A build from an
unreleased tree reports that release, so a project cannot require an
unreleased feature by version: a feature is a compatibility promise only once
it is released.

### Version ranges

`[project].mach` and `[dep.<id>].version` share one range grammar, defined
here and pinned by the compiler's tests:

```
range   = clause *( "," clause )        ; the intersection of every clause
clause  = op partial
op      = "^" / "~" / ">=" / ">" / "<=" / "<" / "="
partial = major [ "." minor [ "." patch [ "-" pre ] ] ]
```

A version satisfies a range when it satisfies every clause. Each clause names
its operator, so a bare `1.2` is refused (`a clause needs an operator, such as
^1.2 or >=1.2`). Whitespace is allowed around `,` and between an operator and
its version, and nowhere else. A missing component is 0.

| clause | means |
|---|---|
| `^1.2.3` | `>=1.2.3, <2.0.0` |
| `^1.2` | `>=1.2.0, <2.0.0` |
| `^1` | `>=1.0.0, <2.0.0` |
| `~1.2.3` | `>=1.2.3, <1.3.0` |
| `~1.2` | `>=1.2.0, <1.3.0` |
| `~1` | `>=1.0.0, <2.0.0` |
| `>=1.2`, `>1.2`, `<=1.2`, `<2` | the bound with missing components as 0: `>1.2` is `>1.2.0` |
| `=1.2.3` | exactly `1.2.3`; `=` needs all three components |

Below 1.0 a minor release is breaking, so caret fixes everything up to the
first nonzero component:

| clause | means |
|---|---|
| `^0.4.2` | `>=0.4.2, <0.5.0` |
| `^0.4` | `>=0.4.0, <0.5.0` |
| `^0.0.3` | `=0.0.3` |
| `^0.0` | `>=0.0.0, <0.1.0` |
| `^0` | `>=0.0.0, <1.0.0` |

A pre-release version (`1.3.0-rc.1`) satisfies a range only when one of its
clauses names a pre-release of that same release. So `^1.2` never selects
`1.3.0-rc.1`, and `>=1.3.0-rc.1, <2` does. Build metadata (`+...`) is refused
in a range and ignored in a release's version. There is no `*` and no `||`.
Either can be added later without changing what an existing range means.

## `[target.<name>]`

Each `<name>` is a selector you pass to `-t <name>`. A target is a
fully-spelled platform tuple; nothing is inferred from another key. `native` is a
reserved name — declaring `[target.native]` is an error, because `native` resolves
to whichever *declared* target matches the host.

| Key   | Required | Meaning |
|-------|----------|---------|
| `arch` | yes      | Architecture, such as `x86_64` or `riscv64`. Read by `$project.target.arch`. |
| `isa` | absent: the system's floor; required on `freestanding` | The instruction set every machine of the target has, spelled as masc's catalog for the architecture spells a selection, such as `rv32imc` or `x86-64-v2`. Every architecture takes it, and masc refuses a selection its catalog does not spell. An `isa` that names an architecture is v6's spelling of `arch`, refused with the rename `mach migrate` writes. See [RISC-V baselines](#risc-v-baselines). |
| `sys`  | yes      | System. Read by `$project.target.sys`. |
| `abi` | yes      | Application binary interface. Read by `$project.target.abi`. |
| `of`  | absent: the system's format | Object-format override; defers to the system's format when omitted. See [Object-format override](#object-format-override). |
| `base` | absent: the system's base | Load-address override (integer, not negative). Overrides the system's default base virtual address; defers to it (`0` for `freestanding`) when omitted. |
| `platform` | absent: no platform | Open platform tag (string), surfaced to comptime as `$mach.build.platform` (empty when unset). A support library keys its backend on it; the compiler treats it as opaque. See [Platform targets](#platform-targets-bare-metal). |
| `stack_reserve` | absent: the format's default (8 MiB on PE), and no stack on an image no loader maps | Main stack size in bytes, not negative: the reserve a PE or Mach-O header records, or the size of the `.stack` section of a freestanding ELF or flat image. See [Image stack size](#image-stack-size). |
| `stack_commit` | absent: the format's default | Main stack commit in bytes, not negative, on PE only. See [Image stack size](#image-stack-size). |
| `code_model` | absent: the host's default for the target | The code model the target's code is built under (string), by the psABI name mink's catalog gives it, such as `medany` on linux `riscv64` or `medlow` on freestanding `riscv64`. Any target may state it. A model the target's host does not offer is refused naming those it does, and one mirl does not carry is refused with mirl's reason. `mach info targets` lists each target's models, its default first, and `--code-model` narrows an entry build's target to one. |
| `default` | removed | Refused as a removed key, with its removal as the fix `mach migrate` applies. A target has no default: with no `-t` a command takes the declared target matching the host. See [`native` target resolution](build.md#native-target-resolution). |
| `os`, `baseline`, `extensions` | removed | v6's names of `sys`, `isa` and `ext`, refused with the rename `mach migrate` writes. |
| `ext` | absent: none selected | Array of instruction-set extension names the target may assume, such as `["sha", "ssse3"]`. Each name must be in the architecture's vocabulary. See [Instruction-set extensions](#instruction-set-extensions). |
| `runner` | absent: the host runs its own programs, and no other target's | Array of argv templates the target's programs are launched through by `mach run` and `mach test`, whether or not the target is the host. See [Execution command](#execution-command). |
| `env` | absent: none | The environment the target's system runs it under (string), such as `macos` on `darwin`, `msvc` on `windows`, `vulkan1.2` on `vulkan` or `preview1` on `wasi`. Any target may state it. The values are the system's, owned by mink's host rows and mirl's target record, and an `env` the system does not list is a manifest error. It is required where the system has no host without an environment, as on `darwin`, `windows` and `wasi`. An architecture that defines its own environments, today only `spirv`, also checks the value against them; see [Finished-module targets](#finished-module-targets). |

### Execution command

A target may declare one execution command, `runner`, which `mach run` and
`mach test` launch the target's programs through. It is an array of argv
templates, the first naming the program, resolved on `PATH` as a command name
would be, and each entry expanded as a [path template](#path-templates) over
`{target.*}`, `{profile.name}`, `{project.work}` and `{artifact.out}`, the path of
the program being launched. Both paths are joined to the project root as given on
the command line, so they hold from the directory mach runs in, as the runner does. The run's
own arguments are appended: what follows `--` for `mach run`, and the dispatcher's
test index for `mach test`. A brace group outside the reserved namespaces is text,
as in a step's `argv`.

```toml
[target.windows]
arch    = "x86_64"
sys     = "windows"
abi    = "win64"
runner = ["wine", "{artifact.out}"]

[target.pc]
arch    = "x86_64"
sys     = "freestanding"
abi    = "sysv64"
runner = ["qemu-system-x86_64", "-drive", "format=raw,file={project.work}/disk.img",
          "-device", "isa-debug-exit"]
```

mach knows no runner by name and adds nothing for one: no emulator list, no
semihosting and no exit hook. The runner's exit status is the program's: `mach run`
exits with it, and a test reads it as a native test's (`0` a pass, `77` a skip,
anything else a failure), so how a guest reports its status is the runner's
business. A disk image or a bootloader wrapping is an ordinary
[`[step]`](#stepname--build-steps) the artifact `need`s, never part of the runner.

A target that declares a runner runs through it whether or not it is the host. One
that declares none runs its programs directly on the host and nowhere else: `mach run`
refuses it, naming the target, and `mach test` builds it and reports it on a `skip`
line. `--runner <cmd>` replaces the declared runner for one invocation, launching
`<cmd> <program> <args>`. An unknown variable or a variable the runner does not admit
is refused when the manifest loads.

### Instruction-set extensions

`ext` lists the extensions a target may assume beyond its `isa`:

```toml
[target.linux-x86_64-sha]
arch = "x86_64"
ext  = ["sha", "ssse3", "sse41"]
sys  = "linux"
abi  = "sysv64"
```

Each architecture owns its vocabulary. The names are identifiers, so each one is also a
comptime member, `$mach.build.ext.<name>` (see [`$mach`](comptime-mach.md)):

| `arch` | `isa` | Extensions |
|-------|----------|------------|
| `x86_64` | SSE2 | `ssse3`, `sse41`, `sse42`, `sha`, `fsgsbase`, `popcnt`, `lzcnt`, `bmi1`, `bmi2`, `cx16`, `avx`, `avx2`, `fma`, `movbe`, `f16c`, `avx512f`, `avx512bw`, `avx512cd`, `avx512dq`, `avx512vl`, `aes`, `pclmul` |
| `aarch64` | the target's `isa` | `FEAT_SHA1`, `FEAT_SB`, `FEAT_AES`, `FEAT_PMULL`, `FEAT_FP16` |
| `riscv64`, `riscv32` | the target's `isa` | `i`, `m`, `a`, `f`, `d`, `c`, `zicond`, `zicsr`, `zifencei`, `zfhmin`, `zfh`, `zkt` |
| `wasm32` | `wasm1` | masc's wasm features, such as `simd128` and `multivalue` |
| `spirv` | | `float16`, `int8`, `int16`, `int64`, `float64`, `zero_init_workgroup`, `storage_read_without_format`, `storage_write_without_format`, `vulkan_memory_model`, `vulkan_memory_model_device_scope`, `buffer_device_address`, `subgroup_arithmetic`, `subgroup_clustered`, `subgroup_vote`, `subgroup_ballot`, `subgroup_shuffle`, `subgroup_shuffle_relative`, `subgroup_quad`, `subgroup_graphics_stages`, `buffer_int64_atomics`, `shared_int64_atomics`, `buffer_float32_atomics`, `buffer_float32_atomic_add`, `buffer_float32_atomic_min_max`, `buffer_float64_atomics`, `buffer_float64_atomic_add`, `buffer_float64_atomic_min_max`, `shared_float32_atomics`, `shared_float32_atomic_add`, `shared_float32_atomic_min_max`, `shared_float64_atomics`, `shared_float64_atomic_add`, `shared_float64_atomic_min_max`, `buffer_float16_atomics`, `buffer_float16_atomic_add`, `buffer_float16_atomic_min_max`, `shared_float16_atomics`, `shared_float16_atomic_add`, `shared_float16_atomic_min_max`, `image_int64_atomics`, `image_float32_atomics`, `image_float32_atomic_add`, `image_float32_atomic_min_max`, `storage_image_multisample`, `resource_min_lod`, `image_gather_extended`, `maintenance8`, `storage_buffer_16bit_access`, `uniform_and_storage_buffer_16bit_access`, `storage_push_constant16`, `storage_input_output16`, `storage_buffer_8bit_access`, `uniform_and_storage_buffer_8bit_access`, `storage_push_constant8` |

On `spirv` each name is a device feature a module may require.
`vulkan1.3` selects `zero_init_workgroup`, `storage_read_without_format`,
`storage_write_without_format`, `vulkan_memory_model`,
`vulkan_memory_model_device_scope` and `buffer_device_address`. The atomic and
sampling features are described with [`op`](gpu.md#intrinsictarget-set-name--a-function-that-is-a-target-instruction),
and the image features with [handles](types.md#handles).

A name the selected isa does not hold is refused when the target resolves, with the
names it does hold:

```
error[target.invalid]: target: `sha2` is not an extension or level of isa 'x86_64'; its extensions are:
ssse3, sse41, sha, fsgsbase, popcnt, lzcnt, bmi1, sse42, cx16, avx, avx2, bmi2, fma,
movbe, f16c, avx512f, avx512bw, avx512cd, avx512dq, avx512vl, aes, pclmul; its levels are:
x86-64-v2, x86-64-v3, x86-64-v4
```

The array must hold strings, each an identifier (`sse41`, not `sse4.1`) or a level
spelling (`x86-64-v2`), listed once.

The x86_64, aarch64 and riscv names, what each implies and the x86-64 levels are read from
masc's extension catalog for the instruction set, and mach adds only which of its
rows the compiler emits against. The spirv names are mach's own until a catalog
names them. A name may cover several rows, as llvm's aarch64 `aes` covers
`FEAT_AES` and `FEAT_PMULL` and `sha2` covers `FEAT_SHA1` and `FEAT_SHA256`: in
`ext` and `#[extension(...)]` it selects every row it covers, and
`$mach.build.ext.<name>` answers for the one row it stands for, the one that
implies the rest (`FEAT_PMULL` for `aes`). The table names each aarch64 row by masc's
name for it. A level is a bundle, never an axis of its own: each
name may imply others, and the selection is closed over that once, when the target
resolves. `sse41` brings `ssse3` (the chain stops there; SSE3 is not a name),
`sse42` brings `sse41`, `avx` brings `sse42`, `avx2`, `fma` and `f16c` bring `avx`,
and every `avx512*` set brings `avx512f`, which brings `avx2`, `fma` and `f16c`. On
aarch64 `FEAT_PMULL` brings `FEAT_AES`. On riscv `d` brings
`f`, `zfh` brings `zfhmin`, `zfhmin` brings `f`, and `f` brings `zicsr`, as masc's
riscv catalog has it, so `ext = ["d"]` on `isa = "rv64i"` selects
`rv64ifd` with Zicsr. The baseline and the list feed one set: `isa = "rv64i"`
with `ext = ["m"]` selects the same machine as `isa = "rv64im"`. A name nothing in
the compiler encodes against yet (`avx`, `avx512f`) is still a declared requirement:
the inline assembler has no rows to admit under it, so today it records only the
promise the binary makes about its hosts, and the promise is the program's to check.

#### Levels

x86-64 also spells the published microarchitecture levels. A level is a manifest
spelling that expands to its member names, so it may stand alone or beside names
(`["x86-64-v2", "sha"]`), and it includes every lower level. It has no bit of its own:
`$mach.build.ext.x86-64-v2` and `#[extension("x86-64-v2")]` do not exist,
programs ask about the members (`.avx2`). `sse2`, `cmpxchg8b` and `lahf-sahf` are the
`x86_64` baseline and are not names. The levels are masc's x86 catalog's, and a test
holds this table to them:

| Level | Members beyond the level before |
|-------|----------------------------------|
| `x86-64-v2` | `ssse3`, `sse41`, `sse42`, `popcnt`, `cx16` |
| `x86-64-v3` | `avx`, `avx2`, `bmi1`, `bmi2`, `fma`, `lzcnt`, `movbe`, `f16c` |
| `x86-64-v4` | `avx512f`, `avx512bw`, `avx512cd`, `avx512dq`, `avx512vl` |

`native` is not a spelling and never a default: the hardware requirement is visible
in the manifest, never inferred from the build machine.

The list is never part of `{target.arch}`. That placeholder is the `arch` value as
written (`riscv64`, `x86_64`), on every isa; the list belongs to the target's identity
and to `{target.name}`.

"Selects" means the extension is assumed of every machine the binary runs on: the
inline assembler admits its rows, `$mach.build.ext.<name>` answers 1, and a
property the extension declares (Zkt's data-independent timing, which the
constant-time multiply rows read) is taken as given. It never means a mode is on. A
row such as a `dit` would admit `msr dit`, not set it.

The `ext` list is the manifest's only lever over the constant-time multiply,
and only on riscv64, where `zkt` is what admits a secret `*`. There is no key that
declares or overrides a timing mode. On aarch64 the condition is PSTATE.DIT, which a
`soft` constant-time function sets around itself where the processor has it and a
`hard` one requires in the target's baseline
([secrecy.md](secrecy.md#soft-and-hard)); a manifest cannot assert it. On x86-64 the multiply rows hold unconditionally
on Intel and AMD, so nothing is there to declare, and Intel's DOITM is a kernel-owned
model-specific register user space cannot set, so mach offers no key for it either. The rows, their conditions and
the DOITM note are in
[secrecy.md](secrecy.md#constant-time-multiply-by-instruction-set).

Some rows are the target's alone. On riscv `i` is the baseline, `c` is a code-size
selection mach never emits, and `f` and `d` select the float register file and the
calling convention's float registers, and `zkt` is a promise about the machine's
execution timing that the constant-time rows read, so none of them may be named in
[`#[extension(...)]`](decorators.md#extnames--an-outlier-function); the
refusal says why. So are spirv `float16`, `int8`, `int16`, `int64` and `float64`,
the device features that let the whole module use a type of that width, and spirv
`zero_init_workgroup`, the device feature
that zero-initializes the workgroup memory of every
[`#[gpu.shared]`](gpu.md#gpuinputn--gpuoutputn--gpubuiltinstr--gpuuniformset-binding--gpustorageset-binding--gpusamplerset-binding--gpupush--gpuspecid--gpushared--shader-interface)
variable in the module, and spirv `storage_read_without_format` and
`storage_write_without_format`, the device features that let a storage image of
`Unknown` format be read and written, spirv `storage_image_multisample`, the
device feature that lets a storage image be multisampled, spirv
`resource_min_lod`, `image_gather_extended` and `maintenance8`, the device features
that let a sample clamp its level of detail, a gather take a run-time offset, and a
fetch or a sample take one too, and spirv `vulkan_memory_model` and
`vulkan_memory_model_device_scope`, the device features that select the Vulkan memory
model for the whole module and let it use the `Device` scope, and spirv
`buffer_device_address`, the device feature that lets a module hold physical
pointers. Every x86_64 and aarch64 row, and riscv `m`, `a`,
`zicond`, `zicsr`, `zifencei`, `zfhmin` and `zfh`, may be.

Selecting an extension is a promise about **every** machine the binary runs on. The
inline assembler admits the extension's mnemonics anywhere in the build, and a host
without the extension faults on the first one it executes. A portable binary keeps the
target at its baseline instead. It confines the extension instructions to
[`#[extension(...)]`](decorators.md#extnames--an-outlier-function) functions
and picks one of those at run time, after detecting the host's features.

A mnemonic that needs an extension the target does not select, outside such a
function, is refused. The refusal names the line to add:

```
error[asm.extension]: inline-asm instruction 'sha256rnds2' needs the `sha` extension, which
this target does not select; add `extensions = ["sha"]` to the target, or mark the
function `#[extension("sha")]` and call it only after detecting the extension at run time
```

The selected set is part of the target's identity: two targets that differ only in
`ext` never share cached products.

The selection also reaches code generation. Every vector operation is legal on every
target and its shape never depends on the extension list; what moves is the lowering.
Each packed row of a target's catalog declares the extension its instruction needs,
and the lowering reads those rows: a cell with a baseline row and a row gated on an
extension lowers to the gated instruction when the selected set holds it (`i32x4 *
i32x4` is `pmulld` under `sse41` on x86_64 and the `pmuludq` pair without). A cell
whose only packed row is gated scalarizes without the extension, and the warning at
each such site names it (see `simd` below). SSE2 is the x86_64 baseline. A build never infers the build machine's features: what the binary
assumes is what the manifest declares.

### Image stack size

`stack_reserve` and `stack_commit` state an image's main stack, as byte counts. What
they mean follows the object format, since the format decides who gives the image its
stack.

```toml
[target.windows]
arch = "x86_64"
sys  = "windows"
abi = "win64"
stack_reserve = 0x800000        # 8 MiB
stack_commit  = 0x1000          # 4 KiB
```

**On a mapped format** the keys are the stack the image asks its loader for. PE keeps
both in its optional header, and Mach-O keeps a reserve in `LC_MAIN.stacksize`.
Omitting them keeps the format's default. On PE the default `stack_reserve` is 8 MiB
(`0x800000`) and the default `stack_commit` is 4 KiB. A reserve is address space, not
committed memory, so raising it costs nothing until the stack is used.

On Mach-O the value reaches only a **position-independent** image. A non-PIE one enters
through `LC_UNIXTHREAD`, which has no stacksize member, so a stack size requested for
such an image is refused at link rather than silently dropped. A commit is refused on
Mach-O, which records none.

**On an image no loader maps**, a `freestanding` ELF image or a flat one, no loader
gives the program a stack, so the link lays one out. `stack_reserve` is the size of a
zero-filled `.stack` section, with no bytes in the file, which the artifact places like
any other section, through [`sections`](#artifact) or a region's
[`run`](#memory-regions). Its top is mink's bound name `__run_stop_.stack`, where
std.runtime's freestanding entry points the stack pointer. The section is laid out
whenever the key is stated, whether or not the program refers to it. Its size is a
multiple of 16, the strictest stack alignment any target's psABI asks, so its top
stays aligned, and another size is refused. The stack is wholly in the image, so `stack_commit` is refused there.

```toml
[target.board]
arch          = "riscv64"
isa           = "rv64imac"
sys           = "freestanding"
abi           = "lp64"
stack_reserve = 0x10000         # 64 KiB
```

There is no default on an image no loader maps, since only the manifest knows how much
memory the machine has to give. An image that refers to the stack's bounds,
`__run_start_.stack` or `__run_stop_.stack`, as every program built on std.runtime's
freestanding entry does, and whose target states no `stack_reserve` is refused, naming
the key:

```
error[target.stack_size]: mach.toml: [target.board] states no `stack_reserve`, and the image
needs a stack: it refers to the bounds of the `.stack` section, which an image no loader maps
holds its stack in. state the stack's size in bytes as `stack_reserve`
```

**On a hosted ELF image**, as on `linux`, the system sizes the main thread's stack from
the process's stack limit (`ulimit -s`), not from the executable, and the format has
nowhere to record a size, so either key is refused by the link, naming the format. Remove
the key. A program that needs a larger stack runs that work on a thread it creates with
the stack size it wants.

The keys live on the target rather than the artifact: every artifact built for one
target shares the value.

### Accepted tuple values

| Axis  | Values |
|-------|--------|
| `arch` | `x86_64`, `aarch64`, `riscv64`, `riscv32`, `spirv`, `wasm32` |
| `sys`  | `linux`, `windows`, `darwin`, `freestanding`, `vulkan`, `wasi` |
| `abi` | `sysv64`, `win64`, `aapcs64`, `lp64`, `lp64f`, `lp64d`, `ilp32`, `ilp32f`, `ilp32d`, `shader`, `basic` |
| `of`  | `elf`, `macho`, `coff`, `flat`, `spirv`, `wasm` |
| `env` | `vulkan1.0`, `vulkan1.1`, `vulkan1.2`, `vulkan1.3`, `preview1` |

Each `arch`, `sys` and `of` value is the name of a row of mink's catalog: `arch` an
architecture, `sys` a system and `of` a format. mach reads the spelling from that row,
and the catalog's rows mach has no target for (`x86`, `uefi`, ...)
are not values here. A spelling mink names otherwise is refused where it is written,
naming the new spelling and carrying the rewrite `mach migrate` applies:

| Key | Retired | Written now |
|-----|---------|-------------|
| `of` | `raw` | `flat` |
| `of` | `spv` | `spirv` |
| `sys` (on `arch = "spirv"`) | `freestanding` | `vulkan` |
| `abi` (on `arch = "spirv"`) | `spirv` | `shader` |
| `arch` | a RISC-V selection such as `rv32imc` | `arch = "riscv32"` and `isa = "rv32imc"` |

v6 wrote a RISC-V selection as `isa = "rv32imc"` with no architecture at all, and
`mach migrate` writes the `arch` that enters it beside it.

```
error[target.retired_spelling]: mach.toml: [target.metal].of = "raw" is a retired spelling; write "flat", mink's name for it
```

`vulkan1.0` through `vulkan1.3` are `vulkan`'s environments and `preview1` is `wasi`'s.

A `[target.*]` naming an `arch`, `sys` or `abi` outside these lists is refused
through the registry's own lookup (`no arch implementation registered for
'mos6502' (registered: ...)`).

`x86_64`/`linux`/`sysv64` is the primary host and target. `aarch64`-linux builds
and runs natively in CI on every PR; `riscv64`-linux runs under qemu and
self-hosts. `windows` is a supported cross-compilation target (PE/COFF,
Win64 ABI). `darwin` is validated end-to-end on both architectures: each
self-hosts to a three-generation fixpoint on a native macOS runner and ships a
release archive. `freestanding` targets a raw flat image with no OS runtime; a
bare-metal platform such as BareMetal (`bmos`) is a `freestanding` target plus a
`platform` tag and `base` override (see [Platform targets](#platform-targets-bare-metal)),
not an os of its own. `spirv` is not a machine at all — it
emits a finished GPU module rather than machine code (see
[Finished-module targets](#finished-module-targets)). `wasi` builds a WebAssembly
module that a host runtime instantiates (see [WebAssembly targets](#webassembly-targets)).

### RISC-V baselines

`arch` names the architecture, and `isa` the instruction set every machine of the
target has, spelled as masc's catalog for the architecture spells a selection. A
target with an operating system that names no `isa` stands on the **floor** mink
states for that system and architecture: `riscv64` on `linux` is `rv64gc`, and
`aarch64` on `linux` is `armv8-a+fp+simd`. A `freestanding` target must name its
`isa`, since the chip decides it and a default would fault on hardware that lacks an
extension; leaving it out is refused (`target.baseline_required`). An `isa` such as
`rv32imc` or `rv64imafd` selects a smaller machine, and `ext` adds names on top of it:

```toml
[target.board]
arch = "riscv32"
isa  = "rv32imc"
sys  = "freestanding"
abi  = "ilp32"
```

masc reads the `isa`, closes it over the catalog's implications (`g` is IMAFD
plus Zicsr and Zifencei, D brings F and F brings Zicsr) and says whether the
architecture enters it. An `isa` masc's catalog refuses, or one of the other
width (`rv64gc` on `riscv32`) or the E base, is refused naming the `isa` and the
architecture. On `x86_64` the `isa` is a comma separated list in masc's x86
catalog, of extension names and levels (`x86-64-v3`, `x86-64-v2,sha`), closed over
the same implications, and a name the catalog does not hold is refused naming it.

mach emits against I, M, A, F, D, C, Zicond, Zicsr, Zifencei, Zfhmin, Zfh and Zkt.
Zicond adds `czero.eqz` and `czero.nez`, which a branch-free select compiles to
where a selection holds it and the xor-and-mask sequence it replaces does
otherwise. Zkt changes no instruction. It states that the listed operations run in
data-independent time, which is what lets a secret multiply compile (see
`secrecy.md`). The `Tag_RISCV_arch` mach writes into its own objects is masc's
spelling of the selection with masc's revision of each extension.

The selected ISA bounds what the compiler generates and what named inline
assembly may use: an instruction needing an extension the selection lacks is
refused with a diagnostic naming that extension. A foreign object's
`Tag_RISCV_arch` must declare only selected extensions at the modeled versions
and the same register width, and its header flags may not claim compressed
code without C; linking never widens the selection. A raw `.word` directive is
the documented unchecked encoding boundary. C is accepted as a capability of the
selected machine, but the emitter writes full-width instructions only.

The ABI still selects the calling convention on its own, and it must fit the
selected machine: `rv32imc` and `rv32imac` have no floating-point registers, so
they take `ilp32` and are refused with `ilp32f` or `ilp32d`; name
`isa = "rv32imafdc"` when RV32 hardware floating point is wanted.

`lp64`, `lp64f`, `lp64d`, `ilp32`, `ilp32f`, and `ilp32d` are the RISC-V psABI
calling-convention family, one `abi` per member. The lp64 three target
`riscv64` and the ilp32 three target `riscv32`; an ilp32 member on a `riscv64`
target or an lp64 member on `riscv32` is refused when that target is selected
(`calling convention 'ilp32' does not target instruction set 'riscv64'`),
since XLEN is part of what the id means. Within each width, the members differ only in how
floating-point arguments travel: `lp64` and `ilp32` are **soft float** — every
float argument rides an integer register — while the `f` and `d` suffixes are
**hard float**, passing `f32` (`f`) or both `f32` and `f64` (`d`) in the
`fa0`-`fa7` register bank. `lp64d` is what a `riscv64-linux-gnu` toolchain
means by "riscv64", and is the convention `mach init` scaffolds for a
`riscv64`/`linux` target; a manifest that wants soft float, or the `f`-only
convention, must name it explicitly, since `abi` has no default of its own
(see [Totality](#totality)). `linux` accepts all three on `riscv64` because
its kernel ABI is integer-only, whereas every operating system accepts only
the conventions it declares per instruction set (`linux` and `darwin` refuse
`win64` on `x86_64`, `windows` refuses `sysv64`) and `freestanding` accepts
every convention the instruction set covers.

`mach info targets` prints every target mink's catalog spells, one per line as
`-t` takes it (`<sys>-<arch>`), then the environments, calling conventions, code
models and executable linkages it accepts as `env=`, `abi=`, `code_model=` and
`linkage=` lists, the default first and marked `*`. A target that runs no
executable, such as `vulkan-spirv`, lists `linkage=-`. `none` is the environment of a host that states none,
such as bare `linux`, and `-` stands for a part the target has no choices of.
It marks `needs --isa` beside each whose system bounds the machine not at all,
such as `freestanding-riscv32`, so a target spelled that way names its
instruction set. The list is the catalog's, so mach holds no table of its own. `mach info` alone prints the tuple the host
resolves to.

A value outside its axis's set is refused when the target resolves, so a typo is
caught rather than silently never matching.

### Object-format override

`of` overrides the object format a target implies. Each os has a default format —
`linux` → `elf`, `windows` → `coff`, `darwin` → `macho`, `freestanding` → `flat`,
`vulkan` → `spirv` — and `of` names a different one from the same closed set: `elf`,
`coff`, `macho`, `flat`, `spirv`. `of` is optional; omit it to
take the default. An os accepts only the formats it can load, so an override the
os cannot enter is refused.

```toml
[target.metal]
arch = "x86_64"
sys  = "freestanding"   # os default object format is "flat"
abi = "sysv64"
of  = "elf"            # override: emit an ELF object instead
```

The default is a function of the whole tuple, not the os alone. An os default
carries relocatable machine text, which a whole-module emitter does not produce,
so a `spirv` target resolves to `spirv`, the format that carries a finished
module, which is also the only format `vulkan` accepts.

An `of` naming a format whose emission shape does not match the instruction set's
is refused at composition (`instruction set 'spirv' emits finished modules, but
object format 'flat' carries linkable objects`), so the override cannot compose a
tuple that would emit nothing.

The page an image is laid out at is a function of the same tuple. A format a
loader maps by page (`elf`, `coff`, `macho`) places every load segment on a page
of its own, so no two segments with different permissions share one: the os's
page where the os declares one (`linux` on `aarch64` lays out at 64 KiB, the
largest page a kernel may use, `darwin` on `aarch64` at 16 KiB, 4 KiB elsewhere),
and the instruction set's hardware page (4 KiB on `x86_64`, `aarch64`, `riscv64`
and `riscv32`) where the os has no loader of its own, which is what `freestanding`
with `of = "elf"` gives a bootloader such as Limine or GRUB. A flat image (`flat`)
and a finished module (`spirv`) have no page and are laid out byte-tight.

### Finished-module targets

A `spirv` target's artifact is a `module`. Each source module is written to
`<out>/obj/<fqn-as-path>.o` like any other target's objects, and each generic instance
the stages reach is compiled in an object of its own beside them, so an object is a
link input that carries the `Linkage` capability and is not a module a Vulkan consumer
takes. mink's SPIR-V link joins them into one module at the artifact's resolved `out`
(or `-o`), giving each id its index and laying out no sections, and drops the
`Linkage` capability and every linkage decoration, so the module it writes is the
deliverable. `--emit obj` stops at the objects:

```toml
[target.gpu]
arch = "spirv"
sys  = "vulkan"
abi = "shader"
# no `of`: the finished-module format resolves on its own
```

```
mach build . --target gpu     # links the module at the artifact's out, from the objects under out/gpu/<profile>/obj
```

With `debug` on, each module carries its debug information inside it, written as core
instructions that need no capability or extension and so fit every `env`: `OpString`
and `OpSource` name the source files, `OpName` names functions, interface variables and
locals, `OpName` and `OpMemberName` name a uniform or storage block's record and its
fields, and `OpLine` attributes each instruction to its source line and column. A
required shader artifact built for a consumer's debug profile therefore builds, and
validation layers and capture tools report names and source lines.

`env` is a general target key whose values are owned by the target's isa; a
`spirv` target uses it to declare the environment its modules are consumed in.
The environment fixes the SPIR-V version word and the capability ceiling:
the compiler derives the minimal capability set a module needs and refuses a
module that needs more than the ceiling, naming the capability and the
environment. The ceiling only admits a capability. One that Vulkan leaves to an
optional device feature also needs the target to select that feature, as below.
Without `env` a module is written as SPIR-V 1.6 with no ceiling, and holds every
feature.

The environment also selects the `zero_init_workgroup` extension from `vulkan1.3`,
where `shaderZeroInitializeWorkgroupMemory` is core, and a module without `env` has
it too. A target for an earlier version selects it with `ext` when its consumer
enables `VK_KHR_zero_initialize_workgroup_memory`.

The environment also selects `storage_read_without_format` and
`storage_write_without_format` from `vulkan1.3`, which accepts the
`StorageImageReadWithoutFormat` and `StorageImageWriteWithoutFormat` capabilities
with no feature enabled, and a module without `env` has them too. A module reading
or writing a storage image of `Unknown` format for an earlier version is refused
unless the target selects the matching extension, which it does when its consumer
enables `shaderStorageImageReadWithoutFormat` or
`shaderStorageImageWriteWithoutFormat`.

The memory model follows the same selection. A module is written under the GLSL450
memory model unless its target selects `vulkan_memory_model`, Vulkan's
`vulkanMemoryModel` feature, and then under the Vulkan memory model, declaring the
`VulkanMemoryModel` capability and, below SPIR-V 1.5, the
`SPV_KHR_vulkan_memory_model` extension. `vulkan1.3` requires the feature of every
device, so it selects the extension, and a module without `env` has it too. A target
for `vulkan1.1` or `vulkan1.2` selects it with `ext` when its consumer enables
the feature, and one for `vulkan1.0` is refused, since the model needs SPIR-V 1.3.
Under the Vulkan model a memory scope of `Device` needs
`vulkanMemoryModelDeviceScope` as well, which `vulkan1.3` also requires: an
instruction taking that scope is refused unless the target selects
`vulkan_memory_model_device_scope`, which brings `vulkan_memory_model` with it. Under
GLSL450 the `QueueFamily` scope and the `MakeAvailable`, `MakeVisible` and `Volatile`
memory semantics, which only the Vulkan model defines, are refused. What
`"coherent"` and `#[gpu.shared]` mean under each model is in
[gpu.md](gpu.md#gpuinputn--gpuoutputn--gpubuiltinstr--gpuuniformset-binding--gpustorageset-binding--gpusamplerset-binding--gpupush--gpuspecid--gpushared--shader-interface).

Physical pointers follow it too. A module holding one, a pointer stored in memory or
made from an address ([types.md](types.md#pointers-on-spir-v)), is refused unless the
target selects `buffer_device_address`, Vulkan's `bufferDeviceAddress` feature, and
then declares the `PhysicalStorageBufferAddresses` capability, the
`PhysicalStorageBuffer64` addressing model and, below SPIR-V 1.5, the
`SPV_KHR_physical_storage_buffer` extension. `vulkan1.3` requires the feature of every
device, so it selects the extension, and a module without `env` has it too. A target
for an earlier version selects it with `ext` when its consumer enables the
feature.

No environment selects `storage_image_multisample`, Vulkan's
`shaderStorageImageMultisample`, which every version leaves optional. A module
declaring a multisampled storage image is refused unless the target selects it,
and with it the module declares `StorageImageMultisample`, and `ImageMSArray` as
well for an arrayed one. A multisampled sampled image needs no feature.

No environment selects `resource_min_lod` either, Vulkan's `shaderResourceMinLod`. A
sample passing the `MinLod` image operand is refused unless the target selects it,
and with it the module declares `MinLod`. Nor does any select `image_gather_extended`,
`shaderImageGatherExtended`, which a run-time `Offset` image operand needs and with
which the module declares `ImageGatherExtended`, or `maintenance8`, under which Vulkan
admits that operand on a fetch or a sample as well as a gather.

No environment selects a type's feature, since every Vulkan version leaves them
optional: `int8`, `int16`, `int64`, `float16` and `float64` are Vulkan's
`shaderInt8`, `shaderInt16`, `shaderInt64`, `shaderFloat16` and `shaderFloat64`,
and the target selects one with `ext` when its consumer enables it. A module
without `env` has all five. The ceiling below still bounds them, so `int8` or
`float16` under `vulkan1.0` is refused for the environment, not the feature.

Under `float16` an `f16` is the native `OpTypeFloat 16` wherever it lives, computed,
negated and converted by the core float instructions, so an `f16` shader needs
`float16` alone. A local, a parameter or a result is that type, and so is an `f16` in
memory the host or the workgroup shares, a stage input or output, a storage buffer, a
uniform or push block, a record a physical pointer reaches or a `#[gpu.shared]` variable,
so an atomic can operate on it and a whole record moves between that memory and a
local as it is. Reading an `f16`'s bits with `:~` into a `u16` or `i16` local, or
back, needs no `int16` either. Without it an `f16` is the software expansion on its 16 bits, which
computes in binary32 on 32-bit integers, so it needs neither `int64` nor
`float64` of its own. An `f64` it converts to or from needs `float64` as any
`f64` does. `%` needs no `int64` at any float width. A stage input or output is
`OpTypeFloat 16` with or without `float16`, so a pipeline interpolates an `f16`
varying as it does an `f32` one, and only an integer or 64-bit fragment input is
`Flat`.

Under `int8` and `int16` an integer of that width computes at its own width.
Without the feature, an integer of that width is carried, wherever it lives in a
function, in a 32-bit integer, which needs no capability: a `u8`, `i8`, `u16` or
`i16` local, and a `bool`, is wrapped and extended at its own width where the
program can tell. A vector is carried lane by lane the same way, so a `u8x4`,
`i16x4` or `u16x8` local, and without `float16` an `f16x4`, needs no feature either,
and its lanes are wrapped and extended at their own width where the program can
tell, a reinterpret with `:~` included. A member of an aggregate keeps its declared
width, so an 8-bit or 16-bit one, or a vector of them, in a local or in `#[gpu.shared]`
memory needs its feature, and the module is refused, naming it, without. Memory the
host shares keeps its width too, under the storage feature below rather than `int8`
or `int16`, and a load from it or a store to it converts to and from the wider integer
the function computes in.
Nothing carries a 64-bit type, so a module holding a `u64`, `i64` or `f64` anywhere
needs `int64` or `float64`.

An 8- or 16-bit scalar, an `f16` included, in memory the host shares needs Vulkan's
storage feature for its width and memory, which no environment selects: a target
names each one its consumer enables, and a target naming no `env` holds them all.
Each enables the capability of its name, declared with its SPIR-V extension below
the version that took it into the core.

| Memory | 16-bit | 8-bit |
|---|---|---|
| `#[gpu.storage(...)]`, or a record a physical pointer reaches | `storage_buffer_16bit_access` (`StorageBuffer16BitAccess`) | `storage_buffer_8bit_access` (`StorageBuffer8BitAccess`) |
| `#[gpu.uniform(...)]` | `uniform_and_storage_buffer_16bit_access` (`UniformAndStorageBuffer16BitAccess`) | `uniform_and_storage_buffer_8bit_access` (`UniformAndStorageBuffer8BitAccess`) |
| `#[gpu.push]` | `storage_push_constant16` (`StoragePushConstant16`) | `storage_push_constant8` (`StoragePushConstant8`) |
| `#[gpu.input(n)]`, `#[gpu.output(n)]` | `storage_input_output16` (`StorageInputOutput16`) | refused |

A stage output starts at its initializer, and at zero without one. A constant of a
16-bit type needs the type's own feature, `int16` or `float16`, so an output holding a
16-bit scalar without it carries no zero constant: each stage that reaches it stores
the zero first, converted from a 32-bit one, and the output needs
`storage_input_output16` alone. One initialized to anything but zero starts at that
constant, which needs the type's feature as well. The 16-bit capabilities need
`SPV_KHR_16bit_storage` below SPIR-V 1.3 and the 8-bit ones `SPV_KHR_8bit_storage`
below SPIR-V 1.5. Vulkan defines no 8-bit stage input or
output, so one is refused, and an 8-bit member of a `#[gpu.storage(...)]` buffer is
refused under `vulkan1.0`, whose storage buffer is a `BufferBlock` in the `Uniform`
class, which the 8-bit feature does not reach.

```toml
[target.gpu]
arch        = "spirv"
sys         = "vulkan"
abi        = "shader"
env        = "vulkan1.0"
ext = ["int16", "int64", "float64"]
```

| `env` | SPIR-V | capabilities the ceiling admits |
|---|---|---|
| `vulkan1.0` | 1.0 | Int16, Int64, Float64, Sampled1D, SampledCubeArray, Image1D, ImageCubeArray, SampledBuffer, ImageBuffer, StorageImageExtendedFormats |
| `vulkan1.1` | 1.3 | same as `vulkan1.0` |
| `vulkan1.2` | 1.5 | the above plus Int8, Float16 |
| `vulkan1.3` | 1.6 | same as `vulkan1.2` |

The ceiling is what a conforming implementation of that Vulkan version can
enable through core device features alone, with no device extension: from the Vulkan
specification's "Vulkan Environment for SPIR-V" appendix, the capabilities table
maps `Int64`, `Int16`, `Float64` and `SampledCubeArray` to the `shaderInt64`,
`shaderInt16`, `shaderFloat64` and `imageCubeArray` features of Vulkan 1.0,
`ImageCubeArray` to `imageCubeArray` as well, `Sampled1D`, `Image1D`,
`SampledBuffer`, `ImageBuffer` and `StorageImageExtendedFormats` to core, and `Int8` and `Float16` to `shaderInt8` and
`shaderFloat16`, which became core features in Vulkan 1.2 (promoted from
`VK_KHR_shader_float16_int8`). The SPIR-V version per Vulkan version is the
appendix's required version: 1.0, 1.3, 1.5 and 1.6.

```toml
[target.gpu]
arch = "spirv"
sys  = "vulkan"
abi = "shader"
env = "vulkan1.2"
```

The artifact's `out` template and `-o` name that delivered module, so
`{artifact.suffix}` gives it `.spv`, and `{artifact.<id>.out}` names it for a
consumer that embeds it (see [Artifact requirements](#artifact-requirements)). A
`static` or `shared` artifact kind, and `mach test`, are refused by name, since
there is no archive, shared object, or executable form for a module.

### WebAssembly targets

A `wasi` target builds a wasm32 module that a host runtime instantiates, so its
programs run under a command the manifest declares as the target's `runner`, since
a wasm module is not a program the host runs itself. mach knows no runtime by name. The one `sys` is
`wasi` under the one environment, `preview1`: the module imports its system
interface from `wasi_snapshot_preview1`, starts at `_start`, and runs one thread.

```toml
[target.wasi-wasm32]
arch   = "wasm32"
sys    = "wasi"
env    = "preview1"
abi    = "basic"
runner = ["wasmtime", "{artifact.out}"]
```

- **The tuple.** `env` is required (see [`[target.<name>]`](#targetname)). `abi =
  "basic"` is the basic C ABI of the WebAssembly tool conventions, and the format is
  `wasm`. Name `isa` and `ext` as on any target: with no `isa` the target stands on
  `wasm1`, WebAssembly's core 1.0, and `ext` adds features from masc's wasm set (see
  [Instruction-set extensions](#instruction-set-extensions)).
- **Artifacts.** A `bin` artifact is a `.wasm` module and a `static` one a `.a`
  archive of objects whose relocation records mink links. The module has no loader
  to map it, so it is never position-independent and no `shared` artifact exists:
  a `shared` artifact is refused with mink's reason, naming the system.
- **No linker-side image options.** A wasm module has no PE subsystem, so
  `subsystem` and `--set subsystem` are refused as on any format without one. Its data
  starts at the first page of linear memory past the stack, which grows down, so a
  stack overflow traps below the data instead of corrupting it.
- **Inline assembly** reads masc's wasm text under a `mvp` or `wasm1` head, as
  [asm.md](asm.md) states.
- **Function pointers** are table indices, as
  [types.md](types.md#function-pointers-on-webassembly) states.
- **Traps.** The runtime ends the program on a trap and mach installs no handler. An
  integer division or remainder by zero, a signed overflow of one, and an access
  outside linear memory trap, as the IR states for every target. A cast from a float
  to an integer saturates as on every target (see [Float to integer](operators.md#float-to-integer)),
  under `wasm1` alone and with `nontrapping-fptoint` in `ext`.
- **What the target lacks.** The module runs one thread, so std's atomic operations
  (`std.sync.atomic`) build and are the plain access, and its barrier and spin hint do
  nothing. mirl refuses atomic IR in a threadless module with `target.unsupported_op`,
  which std's operations never reach. std gates
  each of its modules on the capabilities its target provides
  (`std.system.capability`), so a module that needs threads or sockets is refused
  at build with an error that names the module and the capability. A std primitive
  the preview can compile but not do, such as spawning a process, returns
  `ENOTSUP`, which std reports as its unsupported error kind. std's manifest declares
  no `[link.*]` entry for the system interface, since the environment names the
  module every import resolves from.

### Platform targets (bare metal)

A bare-metal platform — such as [BareMetal](https://github.com/ReturnInfinity/BareMetal)
(`bmos`), Return Infinity's x86-64 exokernel — is not its own `sys`. It is
`sys = "freestanding"` plus two optional keys: a `base` load-address override and an
open `platform` tag a support library keys its backend on (surfaced to comptime as
`$mach.build.platform`). A BareMetal target:

```toml
[target.bmos]
arch      = "x86_64"
sys       = "freestanding"
abi      = "sysv64"
base     = 0xFFFF800000000000   # BareMetal copies the flat image here and calls it
platform = "bmos"               # selects the mach-bmos backend
# no `of`: freestanding's default object format is "flat"
```

- The artifact is a **flat binary** — no header, no sections, no entry record.
  The loader copies the file's bytes verbatim to `base`.
- The load address is set by **`base`** and the loader relocates nothing, so the
  image is never position-independent.
- Execution begins at the **first byte of the image**, which the loader reaches
  with a `call`. The entry function is marked `#[symbol("_start")]`, and it must
  be the only function or the first one emitted, since a flat image cannot say
  where else to enter. An entry anywhere but the base is refused at link.
- A program **exits by returning**: the entry function's `ret` goes back to its
  caller. There is no exit syscall.

The compiler encodes no BareMetal knowledge — `base` places the image and
`platform` is an opaque string. The kernel-call machinery lives in the `mach-bmos`
platform package, which gates on `$mach.build.platform == "bmos"`; because that is
a library, a non-x86-64 bmos build fails at the package's own `$mach.build.arch`
gate rather than in the compiler.

One fact the kernel leaves to the program: **the stack is not guaranteed 16-byte
aligned at entry**, so a startup shim must align it before calling anything that
may use SSE. BareMetal's own `crt0.c` does exactly this.

Zero-initialized data needs no such step. A flat image spans its whole memory
extent, so `.bss` is stored as the zero bytes it is and arrives zeroed with the
rest of the image — an image costs its bss size in file bytes, and nothing
has to zero anything at startup.

## `[profile.<name>]`

A profile is one explicit compilation policy: a build variant. Whether the
optimization passes run, which passes are added or left out, which departures
from exact semantics are permitted, the debug-emission toggle and the SIMD lever
live here because they are variant concerns, and every one of them is stated. Every manifest
declares at least one profile; nothing is synthesized. Values that are
*derived* rather than declared live elsewhere: a target's object format and
naming come from `[target.*]` facts, and an absent optional feature such as a
`[link.*]` filter axis is spelled `"*"` where it applies, not defaulted here.

| Key     | Type    | Meaning |
|---------|---------|---------|
| `optimize` | bool | Whether the build asks mirl for its release schedule of passes. `false` asks for the debug schedule, which keeps the passes every build needs. The legalization passes, which make the program something the target can execute, run at every setting. A non-boolean is a manifest error. |
| `relax` | array of strings | **Optional.** The departures from exact semantics the build permits. The one there is, `"float-reassoc"`, is described under [Float reassociation](#float-reassociation). Absent, the build keeps exact semantics. |
| `linker_relax` | bool | **Optional.** Whether the linker may relax the code, as RISC-V linker relaxation shortens a call or an address pair once the link has placed its target. The build hands it to mirl's `relax` build option on every target, and a target whose linker relaxes nothing takes it without effect. `false` keeps every instruction sequence at the length the compiler laid it out. Absent, it is `true`. It has no command-line form. A non-boolean is a manifest error. |
| `debug` | bool    | Emit debug info for this profile: DWARF in ELF, Mach-O and COFF objects alike, and the core SPIR-V debug instructions on a `spirv` target (see [Finished-module targets](#finished-module-targets)). A PE image carries its DWARF in `.debug_*` sections, which gdb, lldb and the LLVM tools read and Visual Studio and WinDbg do not. Gates emission only, never the optimizer, so a `release` profile can keep symbols with `debug = true`. A non-boolean is a manifest error. |
| `simd`  | string  | SIMD scalarization lever. `"scalarize"` is the one setting a build accepts: mirl legalizes every vector operation, so a target with no packed instruction for an operator gets a defined unrolled scalar expansion. It applies **per operation on every target**, not only to targets with no vector unit: x86-64's SSE2 baseline has no 32-bit lane integer multiply and NEON has no 64-bit one, so a capable target scalarizes too. `"require"` is refused, since mirl reports no operation it scalarizes and a build cannot be held to packed forms. Any other string is a manifest error. |
| `default` | bool | **Optional.** `true` marks the profile a build uses when several are declared and `--profile` is absent. Exactly one profile may carry it. See [Profile requirement and selection](#profile-requirement-and-selection). |
| `allow` | array of strings | **Optional.** The warnings this profile silences, each entry a key or a family of keys from the [warning key table](#silencing-warnings), named once. An unknown key, an entry that covers only errors, a repeated entry or a non-string entry is a manifest error. Absent, nothing is silenced. |
| `opt` | removed | Refused as a removed key: write `optimize = true` or `optimize = false`. |
| `pass`, `skip` | removed | Refused as removed keys: mirl schedules its passes by optimization level, which `optimize` sets. |
| `vectorize` | removed | Refused as a removed key: mirl schedules its passes by optimization level, which `optimize` sets. |
| `float_reassoc` | removed | Refused as a removed key: reassociation is permitted with `relax = ["float-reassoc"]`. |

Three keys (`optimize`, `debug`, `simd`) are required in every profile; the
others are optional, with the meaning of their absence above. A missing key is a
manifest error naming the table and the key:

```
error[manifest.missing_required]: mach.toml: [profile.debug] is missing required key 'simd'; a profile declares optimize, debug and simd
```

A relaxation that does not exist is refused where it is written.

The previous major's keys are refused by name, each with what replaced it, and
with the rewrite `mach migrate` applies as a fix wherever the value has a
mechanical one: `opt = 0` is `optimize = false`, `opt = 1` and `opt = 2` are
`optimize = true`, `vectorize = true` is what `optimize` schedules and is removed,
`vectorize = false` is an opt-out no profile key states and is removed too, with
`#[vectorize("never")]` still opting one function out, `float_reassoc = false` is
removed and `float_reassoc = true` is `relax = ["float-reassoc"]`. `pass` and
`skip` have no rewrite, since no key says which passes run:

```
error[manifest.removed_key]: mach.toml: [profile.release] uses removed key 'opt'; write `optimize = true` or `optimize = false`
  --> mach.toml:12:1
   |
12 | opt = 2
   | ^^^
   |
   = fix: write `optimize = true`
  --> mach.toml:12:1
     -> replace with `optimize = true`
```

### Passes

mirl schedules the middle end's passes by optimization level, and `optimize`
sets the level for a whole build: `true` asks for the release schedule and
`false` for the debug one. No key adds or leaves out a single pass. For a single
function, the `#[vectorize("never")]` decorator is the finer-grained opt-out
(see [decorators.md](decorators.md)).

### Profile requirement and selection

Every manifest declares at least one `[profile.*]` table, a dependency's
included. One that declares none is refused, pointing at the start of the file:

```
error[manifest.missing_required]: mach.toml: missing [profile] table; a build needs an explicit [profile.<name>] declaring optimize, debug and simd
 --> mach.toml:1:1
  |
1 | [project]
  | ^
```

`mach init` writes `debug` (`optimize = false`, `debug = true`, `default = true`) and
`release` (`optimize = true`, `debug = false`) in full, so a scaffold never starts
from that error. More than one profile marked `default = true` is refused in
every manifest.

Which profile a build uses follows one rule, the same one that selects a
target and an artifact:

1. an explicit `-p <name>` wins;
2. otherwise a sole declared profile is chosen;
3. otherwise the one marked `default = true` is chosen.

Table order carries no meaning. A manifest that declares several profiles and
marks none is refused wherever a command must pick one:

```
error[selection.ambiguous]: mach.toml: several profiles are declared and none is marked `default = true`; no profile is selected by table order: mark exactly one [profile.<name>] with `default = true` or select one with -p
```
Emission of the human-readable IR and assembly side-artifacts is **not** a profile
concern: it is controlled only by the `--emit ir` and `--emit asm` flags of `mach build`.

`relax = ["float-reassoc"]` is the one lever here that *adds*, and the only profile
key that can change a program's computed answer. It widens that same pass to float
reductions and does nothing else. A relaxation never rides on `optimize`.

Every lever of a profile is always the **consumer's**.
As [Dependency manifests](#dependency-manifests) sets out, a dependency's `[profile.*]`
is parsed by the same schema and never read to build the consumer, so a library's
values are inert — the effective levers come from the consumer's resolved profile.
Libraries set nothing SIMD-specific and inherit the consumer's choice; there is no
ecosystem fork and no dual API.

### Silencing warnings

Every diagnostic kind has a dotted key, named by the subject it concerns, and
every diagnostic that has one prints it:

```
warning[vector.scalarize]: vector divide on 4 lanes of 32-bit integers in 'app.main.kernel' scalarizes on x86_64: no packed form for it at any extension
```

`allow` names warning keys the build does not report. A silenced warning is
dropped before it is printed or counted, so the summary's warning count leaves
it out as well. Nothing else changes: the same code is built, and an error is
never silenced.

```toml
[profile.release]
optimize = true
debug = false
simd = "scalarize"
allow = ["import.unused", "target"]
```

A key's leading components name a family: `"target"` covers
`target.skipped`, and `"vector"` covers
`vector.scalarize`. A family covers whole components only, so `"vec"` is not a
key. To acknowledge one warning where it is raised instead of across the whole
build, put [`#[expect]`](decorators.md#expectkey--acknowledge-a-warning) on the
declaration that raises it.

Every diagnostic kind has one row in one table in the compiler
(`src/lang/diagnostic/kind.mach`). Each warning names its row where it is
raised, and `allow`, `#[expect]` and the printed key read the same rows, so a
key here is exactly the kind the warning carries. The list is closed: a key no
row declares is refused, naming the warning keys there are. A key is never
reused for a different kind: see [the registry](diagnostics.md#the-registry).

| Key | Warns when | Decided by source |
|---|---|---|
| `import.unused` | a symbol import names something the module never uses | yes |
| `decl.deprecated` | code outside a `#[deprecated]` declaration's module uses it | yes |
| `doc.lint` | a doc comment's component list names no parameter, field, generic or `ret` of its declaration, leaves a component undescribed, or lists them out of declaration order | yes |
| `float.inexact` | a float literal is not exact at its type and its digits are not the shortest spelling of the value stored | yes |
| `fwd.instances` | a shared library `fwd`s a generic, comptime-parameter or pack declaration, which exports no symbol | no |
| `debug.dropped` | the linker leaves out an object's debug info that it cannot merge | no |
| `target.skipped` | multi-target analysis skips a declared target this build does not support | no |
| `vector.scalarize` | a vector operation falls back to scalar code on the target (see `simd`); portable code silences it | no |
| `expect.unfulfilled` | an `#[expect]` names a key decided by source and no such warning is raised inside its declaration | no |
| `secret.target_unenforceable` | an `#[constant_time]` function compiles for a target whose instruction set masc says cannot guarantee constant-time execution (wasm) | no |

A kind decided by source warns or not from the source text alone, whatever the
target, goal or profile. Only such a key is reported when an `#[expect]`
naming it goes unfulfilled; the others depend on what the build compiles and
for which target, so an expectation of one can be quiet in a given build.

Only warnings can be silenced. The table also keys error kinds, which print
their key as well, and naming one in `allow` is refused rather than read as
unknown:

| Key | Error |
|---|---|
| `secret.branch` | a secret value decides a branch or loop condition |

```
error[allow.error_key]: mach.toml: [profile.release].allow entry "secret.branch" names an error; only a warning can be silenced
```

Like the SIMD levers, `allow` is the consumer's: a dependency's profiles are
never read to build it, so a library cannot silence the consumer's warnings.

### Float reassociation

`relax = ["float-reassoc"]` grants the optimizer exactly one liberty: it may treat
floating-point `+` and `*` as **associative**, and regroup a reduction accordingly.
Nothing else changes. It does not license reciprocal substitution for division,
assumptions that operands are finite or non-NaN, contraction into a fused
multiply-add, or flushing subnormals to zero. Each of those would be its own key, with
its own argument.

What it unlocks is the reduction vectorizer. `s = s + a[i]` is a serial dependence
chain: every iteration must wait for the previous one's rounded result, so it cannot be
done four lanes at a time without regrouping the additions. With the key set, the loop
becomes lane-count independent partial accumulators plus a horizontal combine at the
end — which computes a *differently grouped*, and therefore differently rounded, sum.
Element-wise float loops (`a[i] = b[i] * c[i]`) never needed the key and are
unaffected: each lane performs exactly the operation the scalar loop performed.

The reductions it admits are sum (`s = s + x`), product (`s = s * x`), and the dot /
matmul inner loop (`s = s + a[i]*b[i]`). Subtraction and division reductions are not
associative in real arithmetic either, so they are refused with the key set exactly as
without it.

**The accuracy cost, measured.** Summing one million `f64` values of `0.1`, scored
against a compensated (Kahan) reference on x86-64:

| ordering | result | relative error |
|---|---|---|
| strict, sequential | `100000.00000133288` | 1.33e-11 |
| reassociated, 2 lanes | `99999.9999991058` | 8.94e-12 |

The two answers differ from **each other** by 2.2e-11 relative — about 153,000 `f64`
ULPs at that magnitude. The same experiment in `f32` (4 lanes) differs by 1.2e-2
relative: strict gives `100958.34`, reassociated `99759.85`, against a true value of
`100000.0`.

Note the direction. In both cases the *reassociated* answer is the more accurate one,
because splitting into per-lane partials keeps each running total smaller and so grinds
off fewer low bits — the same reason pairwise summation beats sequential summation. But
that is a property of this input, not a guarantee. The honest statement is that the
result **changes**, by roughly the accumulated rounding error of the sum, in a direction
that depends on the data. Code whose correctness depends on the exact bit pattern of a
float reduction — a checksum, a reproducibility requirement, a comparison against a
reference implementation — must not relax it.

Two exactness properties are preserved rather than traded away. The idle lanes are
seeded with the op's **exact** IEEE identity — `-0.0` for addition (`x + (-0.0)` is `x`
for every `x`, where `+0.0` would turn a negative-zero sum positive) and `1.0` for
multiplication — so no signed-zero or NaN behaviour changes. And a trip count below the
lane count never enters the vector loop at all, so short reductions are bit-identical
regardless of the relaxation.

The relaxation is profile-wide. For a single function, `#[vectorize("never")]` opts out of vectorization
entirely and takes precedence over it, so a routine that must stay IEEE-strict inside
an otherwise-reassociating build has a spelling. There is no per-function opt-*in*:
whether a reduction may be reassociated is the caller's tolerance to decide, not the
callee author's.

Integer reductions are untouched by this relaxation. They vectorize unconditionally and are
bit-identical to the scalar reference, because integer add / xor / or / and reassociate
exactly.

### Levers on the command line

`-p <name>` picks the profile, and every lever has its command-line form over
it for one invocation. The two levers are `--key` and `--no-key` pairs:

| Key | Flags |
|---|---|
| `optimize` | `--optimize` (`-O`), `--no-optimize` |
| `debug` | `--debug`, `--no-debug` |

The levers apply in one order: the built-in defaults, then the selected profile,
then the command line. Both flags of a pair are refused. The previous major's
`-g`, `-O0`, `-O2`, `--emit-asm`, `--emit-ir` and `--jobs` are refused naming
`--debug`, `--no-optimize`, `-O`, `--emit asm`, `--emit ir` and `--threads`, and
this major's short `-d`, `-e` and `-s` naming `--debug`, `--emit` and `--source`.
`-k` is the short form of `--kind`. The previous major's `--pie` is refused naming
`--linkage pie` and the `linkage` key.

A `mach build` whose flags change the selected profile writes a differently
configured build, so it must say where: it names the artifact with `-o`, or a
work directory with `-w` that every artifact it writes sits under. Without
either it is refused:

```
error[cli.flag_conflict]: these options would write a differently configured build over the profile's existing artifacts; name an artifact path with -o or a work directory with -w
```

The object cache is keyed by the whole configuration, so only published
artifacts can collide. `mach test` publishes none and takes the levers freely,
and `mach check` and `mach run` build nothing.

### Overriding a key for one build

`mach build` and `mach test` take `--set <key>=<value>`, once per key, with the
key spelled as this manifest spells it. It writes the value over every table of
the key's section for one invocation, a later `--set` of the same key winning.
`--set` writes `platform`, `base`, `stack_reserve` and `stack_commit` over every
target, the `native` one included, and `subsystem` over every artifact.

The value is read as the key is read here: a string-shaped key takes the value
as written, and any other reads it as a TOML value, so `--set base=0x400000`
is an integer. It is then checked by the same rules and refused with the same
reasons as the key in `mach.toml`. A key no table holds is refused naming it,
and so is a key `--set` does not write:

```
error[cli.flag_value]: --set does not apply to 'optimize'; it sets platform, base, stack_reserve, stack_commit, subsystem
error[manifest.value_type]: --set base: must be an integer
```

A value `--set` wrote carries where it was written, so every later refusal of
it, a target's system refusing a subsystem included, names `--set <key>` and
points at the `--set` text rather than at a line of `mach.toml`:

```
error[catalog.unsupported]: --set subsystem: Subsystem windows is unsupported by linux (target 'linux-x86_64' runs on linux, whose images state no subsystem)
 --> --set:1:13
  |
1 | subsystem = windows
  |             ^^^^^^^
```

The previous major's `--subsystem`, `--base`, `--stack-reserve`,
`--stack-commit` and `--platform` are refused naming the `--set` that replaces
each.

## `[artifact.<name>]`

Every artifact is declared explicitly and named by its table key. `$bin.name`
reads the selected artifact's name.

| Key       | Required | Meaning |
|-----------|----------|---------|
| `kind`    | yes | What the artifact is, by mink's product name: `"bin"` (an executable), `"static"` (a static library), `"shared"` (a shared library), `"object"` (each module's object, for a later link) or `"module"` (a module a runtime instantiates, such as a SPIR-V module). Any other value is refused listing mink's products (see below). |
| `linkage` | absent: the target's default | How a `bin` is linked, by mink's linkage name: `"pie"`, `"dynamic"` or `"static"`. A name mink has no linkage of is refused listing them, and the key is refused on any other `kind`, naming it. A linkage the target's loader does not run is refused when the target resolves, pointing at the key and listing those it runs, its default first, as `mach info targets` shows them. |
| `entry`   | yes | Entry source, relative to the project `src` dir (e.g. `main.mach` for `src/main.mach`). The entry module's FQN is `<id>.<entry without .mach>`, `/` turned into `.`. |
| `out`     | every kind but `object` | This artifact's output path, relative to the project root like every other path. An `object` artifact states none, since its objects stay under `obj/` for a later link: stating it is refused, and `mach migrate` removes it. Write `{project.work}/bin/demo` to place it under the build output, or any other project path to place it there. v6 rooted it under the expanded `[project].out` without saying so, so `mach migrate` writes `{project.work}/` before each `out` of a manifest that still writes `[project].out`. Use `{artifact.suffix}` for the target extension, or write a literal filename. See [Artifact filenames and identity](#artifact-filenames-and-identity). |
| `targets` | yes | Array of declared target names this artifact builds for; `["*"]` means every declared target. |
| `link`    | absent: links nothing | Array of `[link.X]` names this artifact links (see below). A name with no table is a manifest error naming the artifact and the declared tables (`[artifact.p1].link names no [link.*] table: 'nosuch' (declared: [link.kernel32])`). |
| `need`    | absent: needs nothing | Array of category-qualified requirements such as `step.generate`, `artifact.support`, and `artifact.shader-*`. Each glob matches only its named category. See [Artifact requirements](#artifact-requirements). |
| `subsystem` | absent: `"console"` | A PE subsystem as mink spells it, such as `"console"` or `"windows"` — the environment a windows executable declares it runs under; refused on a target whose image format has no subsystem (see below). |
| `icon` | absent: no icon | Project-root-relative `.ico` path embedded in a Windows executable's PE resources. Non-empty path string; `bin` artifacts only. |
| `manifest` | absent: no application manifest | Project-root-relative application-manifest path embedded byte-for-byte in a Windows executable's PE resources. Non-empty path string; `bin` artifacts only. |
| `default` | absent: not marked | Selection only. `true` puts the artifact in the [default selection](build.md#selection-and-the-build-matrix): with no `-a`, `mach build` and `mach check` take the marked artifacts among those supporting the selected target (every one of them when none is marked), and a command that needs one artifact (`mach test`, `mach run`, the editor's union build) takes the marked one. A command that needs one artifact refuses two marked candidates; an explicit `-a` always wins, and a sole candidate needs no marker. |
| `export` | absent: not exported | `true` marks the library whose requirements travel to consumers (see [Dependency requirements travel](#dependency-requirements-travel)). It describes the prebuilt library only and never affects how a `use` path resolves. It applies to `static` and `shared` artifacts only, and a project exports at most one. |
| `sections` | absent: the format's order | Array of output section names the link places ahead of the rest, in the order written, such as `[".boot", ".text", ".rodata", ".data", ".bss"]`. A named section the image does not hold is refused by the link, naming it. |
| `size` | absent: no ceiling | The most bytes the image loads from its base, a positive integer. A flat image is padded to exactly `size`, and on any other format it is a ceiling. An image over it is refused by the link, naming the section that overflows. |
| `region` | absent: no region | Named memory regions the link lays sections into, each an `[artifact.<name>.region.<region>]` table. See [Memory regions](#memory-regions). |

`entry` is the build cell's source root. The build follows its active `use` and
`fwd` edges transitively and compiles that reachable module set; another file under
`src` is not part of the cell merely because it shares the project directory. This
is what lets one project declare host and accelerator artifacts with disjoint target
sets. `mach build`, `mach check` and `mach test` all operate on the selected
artifact's closure: a test build compiles and tests exactly the modules the artifact
under test reaches, so a module no selected artifact reaches is not loaded under
any of them (see [test.md](test.md#which-tests-run)).

- **`bin`** links an executable at the resolved `out` path, under its `linkage`,
  the target's default when absent. A target whose loader runs no executable,
  such as a finished-module `spirv` target, refuses it in mink's words.
- **`module`** is a module a runtime instantiates. On a finished-module target
  such as `spirv` it is the module mink's link joins from the build's objects,
  written at `out`. A format
  mink builds no module in, such as ELF, refuses it.
- **`object`** is each module's object, written under `obj/` for a later link,
  as `--emit obj` writes them. It takes no `out`, and `{artifact.<id>.out}` names
  none. An entry build's `-k object -o <path>` writes its object there when the
  build writes exactly one.
- **`static`** materialises a real `ar` archive at the resolved `out` path — the
  per-module objects with an archive symbol index, the deliverable a consumer links
  as a `.a`. The archive carries its link claims: every object mach emits states
  each `[link.<name>]` requirement of its build, its own and those its
  dependencies export, in a section the link leaves out of the image. A link that
  pulls a member finds what its claims name, a library by its `kind` with no
  fallback and a local file by its absolute path, so a consumer that takes the
  archive by path restates none of them. A file the claims name that the
  consumer's own entries name too is linked once (see
  [One file is one input](#one-file-is-one-input)).
- **`shared`** links a dynamic library at the resolved `out`, as mink's shared
  output kind: a `.so` on ELF, a `.dll` on PE and a `.dylib` on Mach-O, named as
  mink's system names the file. A system whose loader maps no library into
  another image, such as `freestanding` or `wasi`, builds none, and the artifact
  is refused with mink's reason when its name is formed.
  - **Exports.** The library exports exactly the declarations marked
    [`#[export]`](decorators.md#export--a-shared-librarys-export), in any project
    of the build, each under its symbol name. `pub` alone exports nothing, and
    `#[symbol("name")]` sets the name an export carries without exporting it.
  - **Internals.** Every other definition still links inside the library but is
    seen from no other image.
  - **Code model.** The library is built under its target's code model, and mirl
    builds its code for a shared object, the output mink links it as. Each
    system's row says what that means there: on ELF an export of default
    visibility may be preempted by another image, on Mach-O only a weak one may,
    and a PE DLL reaches its own exports directly.
  - **Thread-locals.** A `#[thread]` var in a shared library is reached through
    the initial-exec model, at an offset its loader fixes when the process
    starts. General-dynamic access is never lowered.
  - **Linking against one.** An executable names the library through a
    `[link.X]` entry, a `local` path to the built file, and `need`s the artifact
    that builds it. On a host whose loader neither runs executables position
    independent by default nor places every image, as `linux`, an executable
    that links a shared library is linked as mink's dynamic kind, so its loader
    binds the library at start.

### Memory regions

An artifact may lay its image out as a linker script's `MEMORY` and `AT>` do.
Each `[artifact.<name>.region.<region>]` table states one region, and each
section runs in at most one region and loads in at most one other:

```toml
[artifact.firmware]
kind     = "bin"
entry    = "bin/main.mach"
out      = "{project.work}/firmware{artifact.suffix}"
targets  = ["*"]
sections = [".text", ".rodata", ".data", ".bss"]

[artifact.firmware.region.flash]
origin = 0x08000000
length = 0x40000
access = "rx"
run    = [".text", ".rodata"]
load   = [".data"]

[artifact.firmware.region.ram]
origin = 0x20000000
length = 0x8000
access = "rw"
run    = [".data", ".bss", ".stack"]
```

The `.stack` section is the one the target's `stack_reserve` lays out (see
[Image stack size](#image-stack-size)), so its top, `__run_stop_.stack`, is the top of
the stack the program starts on.

| Key | Required | Meaning |
|-----|----------|---------|
| `origin` | yes | The address the region starts at. |
| `length` | yes | The bytes it spans. |
| `access` | yes | What it allows, the letters `r`, `w` and `x`, each at most once. |
| `run` | absent: none | The output sections that run in the region. |
| `load` | absent: none | The output sections whose bytes load in the region and run in another, as `.data` loaded in flash and run in RAM. |

A section that runs in two regions or loads in two is refused, and so is one that
loads in a region and runs in none, or loads in the region it runs in. A name in
`sections`, `run` or `load` is an output section, as mink's layout rules name the
output section each object's section is laid into, so `.text` takes every
`.text.*` section. A name no object's section is laid into lays no bytes, so one
layout serves a program with no `.bss` as well as one with it, and its bounds
name the empty range where it would land.

A loaded output section no region names, such as an `.eh_frame` the link writes
of the objects' frame records or a `.rodata` left out of `run`, runs in the
region whose `access` admits it and allows least beyond what it needs, laid
after the sections the regions name. Of regions that tie, the one stated first
takes it, which is why the example states `flash` before `ram`: a read-only
section is admitted by both, each allowing one letter more. A section no region
admits is refused, naming it. mach
hands the regions, the section order, the size ceiling and every `#[at]`
placement to mink as one link plan, and reports mink's refusals, an overflow or
an overlap, as mink names them. mink names the bounds of each section by
its load and run addresses, `__load_start_<section>`, `__load_stop_<section>`,
`__run_start_<section>` and `__run_stop_<section>`, so startup code, such as
copying `.data` out of its load region, is a library's and never the compiler's.

Per-target extension or per-target entry is not a per-cell exception table — it is a
second artifact stanza, so the condition stays visible like everything else.

### Artifact filenames and identity

Use `{artifact.suffix}` in an artifact's `out` to select its target filename
extension. Literal paths stay literal. No prefix is inserted, so a library may
spell its desired `lib` prefix directly.

```toml
[artifact.app]
kind = "bin"
entry = "main.mach"
out = "{project.work}/bin/app{artifact.suffix}"
targets = ["*"]
link = []
need = []
```

This produces `app.exe` on Windows and `app` on Linux and Darwin. The artifact
name and `$bin.name` remain `app` on every target. `mach init` generates one
artifact using this form. Build, run, clean, required-artifact paths and plan
inspection use the same expansion.

| Target output format | `bin` suffix | `static` suffix | `shared` suffix |
| --- | --- | --- | --- |
| ELF on Linux or freestanding | empty | `.a` | `.so` (refused on freestanding) |
| Mach-O on Darwin | empty | `.a` | `.dylib` (not written) |
| COFF/PE on Windows | `.exe` | `.lib` | `.dll` (not written) |
| Raw image | empty | unsupported | unsupported |
| SPIR-V module | refused, its artifact is a `module` (`.spv`) | unsupported | unsupported |
| wasm module on wasi | `.wasm` | `.a` | refused |

The selected object format supplies the naming rules, including explicit target
format overrides. Unsupported library forms are errors. A SPIR-V `module` artifact is
linked from the build's objects and delivered at `out`, with the objects still written
under `obj/` (see [Finished-module targets](#finished-module-targets)).

`{artifact.suffix}` is available only in an artifact output template. It does not
expand in project output roots, link paths, step arguments or source embeds.
Output collisions are checked after expansion among artifacts selected for the
target. An explicit literal such as `bin/app.exe` can therefore collide with
`bin/app{artifact.suffix}` on Windows.

### `subsystem` — the PE subsystem

```toml
[artifact.game]
kind = "bin"
entry = "main.mach"
out = "{project.work}/bin/game.exe"
targets = ["*"]
link = []
need = []
subsystem = "windows"
```

A PE executable records in its optional header which environment it wants, and the
Windows loader honours it: `"console"` gets a console window attached to the
process, `"windows"` does not. The names are mink's, its linkers' subsystem
spellings: `"console"`, `"windows"`, `"native"`, `"posix"`, `"efi_application"`,
`"efi_boot_service_driver"`, `"efi_runtime_driver"`, `"efi_rom"`, `"xbox"` and
`"boot_application"`. mach defaults to `"console"`, so an artifact that omits the
key declares the console subsystem. A graphical application sets `"windows"` to
stop an empty console from opening behind it on launch.

v6 spelled the graphical subsystem `"gui"`. v7 refuses it, naming `"windows"`,
and `mach migrate` rewrites `subsystem = "gui"` to `subsystem = "windows"`.

Only a PE image carries the field. A key written on an artifact that builds
for a target whose format has none (ELF, Mach-O, a flat image, a wasm module) is refused as
unsupported, naming the key, the target and the format:

```
mach.toml: artifact.game.subsystem: Subsystem windows is unsupported by linux (target 'host' runs on linux, whose images state no subsystem)
```

An omitted key is the console default and is never refused, so an artifact
that declares no subsystem still builds everywhere. An artifact that needs the
key and also targets a non-windows cell declares two artifacts, one per format,
the way `[link.X]` entries carry `sys`/`arch`/`abi` axes: the manifest never
carries a declaration a build silently ignores.

`--set subsystem=<name>` writes the key over every artifact for one invocation, held to every
rule the key keeps here, and is refused the same way on a target whose format has no subsystem
(see [Overriding a key for one build](#overriding-a-key-for-one-build)).

### `icon` / `manifest` — Windows executable resources

```toml
[artifact.game]
kind = "bin"
entry = "main.mach"
out = "{project.work}/bin/game.exe"
targets = ["*"]
link = []
need = []
icon = "assets/game.ico"
manifest = "assets/game.manifest"
```

On a Windows target, either key adds a `.rsrc` section. `icon` must name a valid
ICO container; mach emits each contained image as `RT_ICON` and an
`RT_GROUP_ICON` that indexes them. `manifest` is emitted unchanged as
`RT_MANIFEST`. A `VS_VERSIONINFO` (`RT_VERSION`) accompanies the declared
resources with these schema-derived values:

| Version field | Value |
|---------------|-------|
| `FileVersion`, `ProductVersion` | `[project].version` |
| `InternalName`, `ProductName` | the `[artifact.<name>]` table key |
| `OriginalFilename` | basename of the resolved executable output, retaining an extension such as `.exe` |

There is no `FileDescription`: the live manifest schema has no accepted
description field for an artifact. Strings are converted from strict UTF-8 to
UTF-16, including surrogate pairs; malformed text, malformed/empty resources,
and values that exceed PE's 16/32-bit fields fail the build instead of being
truncated.

Paths use the same portable `/` spelling as other manifest paths and are resolved
against the project root. A generating `[step.X]` must appear in `need` and write
the named path before linking. Resource paths and contents participate in the
link fingerprint, so changing an asset at the same path relinks a warm build.

The keys remain valid in a multi-target artifact, but are completely inert off
Windows: mach does not resolve or read either path and ELF, Mach-O, and raw output
remain unchanged. `static` and `shared` artifacts reject these executable-only
keys.

## `[link.<name>]` — link requirements

A `[link.X]` is a named external link requirement. Artifacts reference entries by
name in their `link = [...]`; an entry whose filters do not match the build cell is
skipped. An entry with `export = true` also applies to any project that links this
project's modules, so a platform link requirement lives once — in the manifest that
needs it — and cascades to consumers. A standalone build and a consumed build use
the same entries, so nothing behaves differently as a dependency.

| Key       | Required | Meaning |
|-----------|----------|---------|
| `source`  | yes | `"system"` (a system library resolved by name), `"framework"` (a macOS framework), or `"local"` (a file on disk). |
| `name`    | shape | Library/framework name — required for `source = "system"`/`"framework"`, forbidden for `"local"`. |
| `path`    | shape | File path — required for `source = "local"`, forbidden otherwise. A template (see below). |
| `symbols` | absent: none claimed | Array of symbol names this dependency provides, attributing imports that have no `ext` declaration to decorate (see below). Written as **source-level** names; the target's C symbol prefix is applied by Mach. |
| `sys`      | yes | Filter axis: an array of canonical `sys` values or `"*"` (any); `[]` matches none. |
| `arch`     | yes | Filter axis over `arch`, same forms. |
| `abi`     | yes | Filter axis over `abi`, same forms. |
| `export`  | absent: not exported | `true` cascades this entry to consumers; `false` keeps it to this project's own builds. |
| `include` | absent: `"always"` | `"always"` names the dynamic library in the linked image whether or not anything imports from it; `"referenced"` names it only when a live import references it, so an unused provider leaves no load command behind. Any other value is a manifest error (`[link.k].include must be "always" or "referenced"`). |
| `current_version`, `compatibility_version` | absent: `0.0.0` | The versions a Mach-O load command records for this library, each `X`, `X.Y` or `X.Y.Z` with `X` below 65536 and `Y` and `Z` below 256. Both or neither, and only on a `source = "system"` entry of the shared kind. dyld compares the compatibility version against the loaded library's current version. Other formats ignore them. |
| `kind`    | absent: `"shared"` | What a `source = "system"` library named by a bare name must be: `"shared"` or `"static"`, with no fallback in either direction (see below). Any other value is a manifest error (`[link.k].kind must be "shared" or "static"`). Refused on any other source, and on a `name` that names a file, whose contents state its kind. |
| `library` | removed | Refused as a removed key. The table key is the library's identity, and `#[library]` names the entry by that key. |
| `os`, `isa` | removed | v6's names of `sys` and `arch`, refused with the rename `mach migrate` writes. |

The `sys`/`arch`/`abi` axes select the build cells an entry applies to. Each is an
array, like an artifact's `targets`: canonical values, `"*"` for any, and `[]`
matching nothing (an entry deliberately switched off). A bare string is refused. The three axes are required: an axis left out would mean every
target, which is an assumption to state, so a `kernel32` entry with no `sys` is
refused rather than linked on Linux. A non-canonical spelling is a manifest
error. An entry applies
to a cell when all three axes match.

A `local` entry's `path` must, at build time, either match a `[step.X]`'s `out`
(which demands that step) or already exist on disk — anything else is an up-front
error, so a typo never silently drops an input. A `local` path naming a shared
library is validated for the selected target before it is recorded: an ELF
`.so` that is not a loadable shared object for the target's architecture (a
linker script, a foreign-architecture file) is refused (`'<file>' is not a
loadable ELF shared object for the selected architecture`).

### One file is one input

A link reads each file once, however many times and however it is named: by
an artifact's `link` list, by two entries, through `{project.work}`, and by the
claims of the objects it links, which name a `local` file by its absolute path
(every object mach emits carries a claim per entry, so an artifact's own
`local` entry always reaches its link twice). mink knows a file by its
absolute path, lexically cleaned, and a file named again adds nothing. When the
namings disagree on `include`, the library is named in the image if any of them
says `"always"`, and otherwise as the first naming says. A symbolic link is not
followed: a file named through a symbolic link and by its own path, or by two
hard links, is two inputs, and an object read twice that way is refused as a
symbol defined in more than one input.

An entry's table key is its identity: `[link.vulkan]` answers to
`#[library("vulkan")]` and to nothing else. A loader name such as
`kernel32.dll` does not bind, and an import attributed to one is refused with a
message naming the entry whose key to write. A `library` key is refused by
name, since no entry answers to a name other than its key. When
mutually exclusive platform entries provide the same API, the binding picks the
key with a constant that `$if` selects, so one binding module serves every
platform (see the `mach-glfw` example below). Selecting two dependencies whose
entries share a key but resolve to different loader names in one build is an
error, as is a key that equals a different entry's loader name, so attribution
never depends on requirement order.

A `#[library]` resolves against the **effective** link set: the artifact's own
referenced entries, plus every entry a dependency exports. A binding project
therefore names its libraries once and a consumer writing its own `ext fun`
against them adds nothing but a `dep` entry.

A key may belong to an entry that resolves to a **static** input, and
that is not something an import can bind to: a static input defines symbols
rather than importing them, so a pin naming one means the symbol must come out of
that object or archive. When it does, the pin is inert and the link is normal.
When it does not, the symbol is undefined, and mach says exactly that — naming
the entry, its kind, and the undefined symbol, rather than claiming the library
is missing from the link.

`symbols` names the symbols the dependency provides. On a two-level-namespace
format (PE, Mach-O) every import must identify its provider, and `#[library]` can
only attribute a symbol your Mach source declares. A **vendored static archive**
leaves its own undefined references — the Win32 calls inside a `glfw3.a`, say —
with no declaration to decorate, so the entry that provides them claims them:

```toml
[link.kernel32]
source  = "system"
name    = "kernel32.dll"
symbols = ["Sleep", "CreateFileW", "CloseHandle"]
sys      = ["windows"]
arch     = ["*"]
abi     = ["*"]
export  = true
```

Each name is written the way you would write it in **source**, without the
target's C symbol prefix. Mach applies that prefix itself, exactly as it does for
an `ext fun` declaration, so `symbols = ["Sleep"]` attributes the `Sleep` a Linux
or Windows object names and the `_Sleep` a Mach-O object names, and one manifest
is correct on every target. The prefix is only ever added, never stripped: `_exit`
is a real C symbol whose Mach-O object spelling is `__exit`, so "already
prefixed" is not something a spelling can be checked for. Writing the mangled
form yourself therefore does not work — on darwin `symbols = ["_Sleep"]` claims
`__Sleep`, which nothing imports.

Claims travel with the entry, so `export = true` cascades them to consumers and a
C-binding project declares them once.

A symbol may be claimed only once per link: two selected entries claiming it, or a
claim contradicting a `#[library]` decorator, is an error naming both claimants
rather than an order-dependent win — repeating the *same* claim is fine. Listing a
symbol twice within one entry is rejected.
On ELF the key is accepted and validated but changes no emitted bytes, since that
loader resolves imports by global search.

### Libraries linked from their claims

On PE and Mach-O a `system` entry of the shared kind is linked from its claim,
and no file is searched for or read. Every object mach emits carries a claim
record (`.mink.claims`) per entry it links: the library's loader name as the
entry writes it, its versions, and each symbol the object takes from it, by its
link name and whether it is code or data. Those symbols are the entry's
`symbols` and every `#[library]` import of the entry the object references. mink
builds the library's interface from the record alone: on PE an import directory
entry for the DLL with each symbol imported by name, and on Mach-O a load command
with the install name and the entry's versions, each symbol bound to that
library's ordinal. A Windows or macOS program therefore cross-links from any host
with no import library, SDK or `-L`. A symbol two records give to two different
libraries is refused, naming the symbol and both libraries.

Everything else is a file: a `static` entry, a `local` entry, a `framework`, and
every entry on ELF and wasm. ELF stays file based because only the `.so` states
what an import needs there, its symbol versions and the size of data a copy
relocation moves. A shared library whose symbols the build does not know, one
whose imports come from C objects with no claim, is linked from its import
library with a `local` entry.

A `system` entry that is not linked from its claim, and a `framework` entry, is
found by name through mink's library search as the link runs, in each `-L` directory, then the working directory, then the
system's directories the target's host row states. A `local` entry names its
file. Whether an input links **statically** or **dynamically** follows the file
found — a loose `.o`/`.obj` or static `.a`/`.lib` links statically, and an ELF
`.so`, Mach-O `.dylib` or PE `.dll` is recorded by its format's canonical loader
name.

A `system` entry whose `name` is a bare library name is looked for as one kind
alone, `kind = "shared"` unless the entry says `kind = "static"`. There is no
fallback in either direction: a file of the other kind is skipped and the search
goes on, and a search that takes nothing is refused, naming the library looked
for, the kind asked for, and any file of the other kind it passed over. The kind
is read from what a file holds, never from its suffix. A linker script is shared
when any file it names is shared, and static when it names only static files. A
Windows `.lib` that holds any import member counts as shared, so a mixed `.lib`,
holding both objects and import members, answers `kind = "shared"` and is refused
for `kind = "static"`.

`kind = "shared"` needs an image that imports, so a target that builds no such
executable refuses it at once. An executable is linked under its artifact's
`linkage`, the target's default when absent, whatever its inputs are, so one that
binds a shared library is linked `pie` or `dynamic`. A
static library that needs a shared one its build also names may still fail to
link: `libm.a` beside a shared `libc` is refused by the link, so name both of the
same kind.

A `name` that names a file, such as `kernel32.dll` or
`/usr/lib/libSystem.B.dylib`, is taken as written, as the loader name its claim
records, and does not take the key:
stating `kind` on it is refused, as is stating it on a `framework` or `local`
entry. Two entries naming one library as different kinds are refused, as is a
`-l` of a library an entry names `kind = "static"`.

See
[ext-fun.md](ext-fun.md#linking-external-objects) for the `ext fun` workflow
that consumes these inputs.

## `[step.<name>]` — build steps

A step is a command, make-recipe style, that produces files a build consumes
(typically a `local` link input, e.g. a vendored-C object). `<name>` must be a
plain identifier — it keys the step's stamp file.

| Key    | Required | Meaning |
|--------|----------|---------|
| `argv` | yes | Nonempty array of strings, spawned directly with no shell. `argv[0]` names the executable, by path or resolved on the planner `PATH`. Templates expand in every element. To use a shell, spell it: `["sh", "-c", "…"]`. |
| `env`  | absent: nothing added | Table of string values added to the step process's environment. |
| `env_in` | absent: none | Declared input variable list: names of inherited environment variables the step reads, such as `CC`. Each name is nonempty, holds no `=`, and is distinct from every other under the rules for `env` names. |
| `in`   | yes | Declared input file list. Accepts globs (`*`, `**`), expanded sorted for a stable fingerprint; a glob that matches nothing is a hard error. |
| `out`  | yes | Declared output file list. Concrete paths only — a glob here is an error, since the demand match and cache key expand `out` verbatim. |
| `need` | absent: needs nothing | Array of `step.<name>` requirements or `step.<pattern>` globs this step must run after. Steps may require only steps. Cycles are manifest errors. |
| `timeout` | absent: no limit | Duration string (`"30ms"`, `"30s"`, `"5m"`, `"1h"`) after which the step's process group is terminated and the build fails. Omit for an unbounded step. |
| `cmd` | removed | Refused as a removed key. Write the command as a nonempty `argv` array. |
| `shell` | removed | Refused as a removed key. Put the shell executable and its arguments in `argv`. |

Steps carry **no filters** and **never run automatically**. A step runs only when
**demanded**:

- by a selected `[link.X]` whose `local` `path` matches the step's `out`;
- by another step's `need`;
- by an artifact's `need` (for outputs that are not link inputs), by name or
  through a glob;
- in a dependency, by the `need` of its `export = true` library artifact, which
  travels to every consumer (see
  [Dependency requirements travel](#dependency-requirements-travel)).

Because a step has no filter of its own, the condition for running it lives in the
link entry that demands it: on a build cell where that entry filters out, the step
is never demanded and never runs.

A step is cached by content: its declared inputs, resolved executable, expanded
arguments, and declared environment contribute to its fingerprint. The declared
environment is the `env` table as expanded, the three `MACH_TARGET_*` values, and
the value of each inherited variable `env_in` names, as the child receives it; a
named variable that is unset keys differently from one set to the empty string.
The child still inherits the whole environment, but an inherited variable the step
does not name in `env_in` is not an input, exactly as a file missing from `in` is
not: changing it does not invalidate the step. `PATH` reaches the fingerprint only
through the executable it resolves `argv[0]` to.

The stamp of the last successful run records the fingerprint and the content
digest of each declared output (a directory output by every path, kind and file
beneath it). A step is skipped when its fingerprint is unchanged and every declared
output still holds the bytes the stamp records; an output that is missing,
modified or replaced since the step ran runs the step again. A step that runs
again invalidates only what consumes its outputs, and only when their bytes
changed: a module that embeds an output is keyed by that file's digest, a link
that takes an output as a `local` input is keyed by that file's digest, and a
generated source is keyed like any other source. No step's fingerprint is part
of any module's key, so a step that runs again and writes the same bytes
rebuilds nothing.

The stamp is kept in `{project.work}/.cache/steps/`, and a step's outputs under
`{project.work}` are written into scratch space in `{project.work}/.stage/<name>/`
and published only once the step succeeds. Both belong
to the compiler: a declared `out` inside `{project.work}/.cache/` or
`{project.work}/.stage/` fails at manifest load, naming the step and the path.
`mach clean` removes both, so the next build runs every demanded step again.

**Bounding a step.** `timeout` gives the step a deadline measured from
the moment it is spawned. When the deadline passes, the step's whole process
group is signalled — a compiler or archiver the step's shell invoked dies with
it rather than outliving the build — the child is reaped, and the build fails
naming the step and the bound. Omitting the key leaves the step unbounded,
which is the default and the behaviour of every step that does not set it. The
value is a string holding a positive integer and a unit, `ms`, `s`, `m` or `h`,
the same grammar as `mach test --timeout`. A bare number, zero, a fraction and
any other unit are rejected at manifest load.
The bound is not part of the step's cache key: changing it does not invalidate
a cached step, because it cannot change what the step produces.

A source file's `#[embed(...)]` decorator (see
[decorators.md](decorators.md#embedstr--compile-time-file-embedding))
is a build input under the same content-based principle, by a different
mechanism: it has no `[step]` stanza of its own. The embedded file's content
digest is published into a `Q_EMBED_FILE` query input that the embedding
module's sema depends on, so an edited asset invalidates that module's cached
sema and an untouched asset is a cache hit — the same guarantee `in` gives a
step, keyed to one file instead of a step's whole input list, and by digest
rather than timestamp either way.

**Output homing.** `{project.work}` resolves to the **root** project's expanded
`out` in every manifest of the closure. A dependency's step outputs land in the
consumer's output tree — exactly as a dependency's compiled modules do — and the
dependency's own checkout is never written to. For an exported dependency step,
the command receives `{project.work}` as an absolute path rooted at the consumer;
it does not depend on the directory from which the consumer was invoked. An ordinary relative
consumer root (including `./` segments) and its normalized absolute spelling produce
the same expanded command and cache key.

**The module object tree is reserved.** `{project.work}/obj/<project.id>/` belongs
to the compiler: every module of the project compiles to one object in it, named
after the module's path (`src/window.mach` in project `glfw` becomes
`{project.work}/obj/glfw/window.o`). A step that writes there collides with those
objects by name, and because the link takes whichever file survived, the result is
a binary that is subtly wrong rather than a build that fails.

**The object tree is the object cache.** `mach build` and `mach test` read it by
default: a module whose object in `obj/` was built under the same key is reused
as it is: the module is neither
lowered nor generated again, and when nothing the build still compiles imports
it, it is not resolved or type-checked either. Each module has its own key: the
compiler identity, the build configuration, the module's own source and embedded
files, and the surface of every module it imports, directly or not. A module's
surface is its source without the bodies of its tests and of its functions that
are neither generic nor take a comptime parameter, since no importer compiles
those; a release build inlines function bodies across modules, so there the
whole source is the surface. Editing such a body rebuilds that module alone, and
editing a declaration rebuilds the module and the modules that import it. With
debug information the surface also covers where each retained declaration sits,
so an edit that moves one to another line rebuilds its importers. A reused
module reports again the warnings it reported when it was compiled, so a warm
build prints what a cold one prints. Each object
carries its key in a section no link loads: `.mach.cache` on
ELF (not allocated) and COFF (`IMAGE_SCN_LNK_INFO | IMAGE_SCN_LNK_REMOVE`), and
`__MACH,__mach_cache` on Mach-O (debug-attributed). The parser consumes it, so an
object links exactly as it would without it. The section also carries the image
the compiler generated for the module, and a reused module links that image
rather than what the object format spells, since COFF and Mach-O cannot spell
everything a link reads, such as symbol sizes. A warm build therefore links the
binary a cold one links, byte for byte. Each object's SHA-256 as written is kept
in `{project.work}/.cache/objects` under its path in `obj/`, and a build reuses an
object only when the bytes it reads hash to that digest: an object that is missing,
has no recorded digest, holds other bytes than were written (one flipped bit
included), has no key, a damaged one or another key is rebuilt, never linked
stale. `obj/` holds one object per module, the latest, and each object is written
to a sibling temporary and renamed into place, so an interrupted build leaves the
previous object or the new one, never a torn file, and the digests are stored only
after the objects they describe are in place. No timestamp decides anything: every
source file is read and hashed on every build, and a file's modification time,
size and identity are never consulted. The surface digests of a source text are
remembered in `{project.work}/.cache/digests` under the SHA-256 of the text, so a
text the build has seen is not walked again for its key. Every file under
`.cache/` reads as empty when it is missing, truncated, damaged or written by
another compiler, which costs only rework: the build recovers without anything
being deleted by hand.

**A link whose inputs did not change is not run again.** After each link the
build records, in `{project.work}/.cache/link` under the output's canonical
path, the link's key, the SHA-256 of the bytes it wrote and the other files it
read. The key covers the whole configuration (the compiler binary's digest among
it, and so the linker's), the link configuration (the artifact's kind, its
output path and product name, its resources by content, every link input given
as a file by its digest, the library search directories and the layout), the
key of every object the link takes, in link order, and every other file the
link read, such as a `local` file the claims of a static archive name or a file
a linker script names, by its path and the digest of its bytes. A build that
compiled nothing, whose key equals the record of its output, with every file
read and hashed again, and whose output file still hashes to the recorded
digest leaves the output as it is: same bytes, same modification time, same
file. Every file's bytes are read and hashed every time; sizes and times decide
nothing. A link that reads a library found by name in a search directory always
runs, since the key names such a library and not its contents: a `system` or
`framework` entry linked from a file, `-l`, or a claim of an archive's member
naming a library found that way. Anything else, a missing or changed output,
another key or a damaged record, links as before.

`--no-cache` forces an uncached
build: it reuses no object and writes each one without a key, and `mach clean`
removes the tree with the rest of the output.

A step output is therefore rejected in that subtree. A declared `out` inside it
fails at manifest load, naming the step and the path, before any step runs. A step
that writes an object there without declaring it is caught after it runs, with the
same message — this covers the common case of a vendored `make` dropping every
object it built into the output directory.

Pick any other subtree of `{project.work}`. The conventional choice for a vendored
library is a directory named after the library rather than after the project, e.g.
`{project.work}/obj/miniaudio/` for a project whose own id is `audio`; note that
this only stays clear of the reserved tree while the two names differ, so prefer a
distinct sibling such as `{project.work}/vendor/<library>/`.

**Target environment.** Every step process additionally receives the active
build cell's target tuple as `MACH_TARGET_ARCH`, `MACH_TARGET_SYS`, and
`MACH_TARGET_ABI`, so the script `argv` invokes can branch on the target
without threading it through the template — e.g. `cc --target=$MACH_TARGET_ARCH-…`.
The step inherits the planner's environment, then applies its declared `env`
values, then assigns those three target variables. Names use host identity:
case-sensitive on Unix and ordinal case-insensitive on Windows, including Unicode
names. A declaration cannot contain names differing only in ASCII case on any
host, or names that alias under Windows Unicode comparison on Windows. Each name
has one value. A `MACH_TARGET_*` value inherited from an enclosing
build or declared by the step is overwritten by the cell's own. The same three values are
available in the `argv` templates as the `{target.*}` keys (see
[Path templates](#path-templates)).

`cmd` was the pre-4.x spelling of a step's command and is a removed key:
`[step.s] uses removed key 'cmd'; use a nonempty 'argv' array`.

## `[dep.<id>]`

A dependency is named by its **project id**, and that one name is used in three
places: the manifest key `[dep.<id>]`, the directory `dep/<id>/`, and the head
segment of every module path the dependency exposes (`use <id>.x;`). The
compiler checks all three agree: `dep/<id>/mach.toml` must declare
`id = "<id>"`. A dependency whose id is the declaring project's own is refused
where it is declared, since one head segment cannot name two projects.

```toml
[dep.std]
git = "https://github.com/briar-systems/mach-std"
version = "^7.4"
```

A stanza declares exactly one source. Every key's presence is decided by the
others:

| Key    | Meaning |
|--------|---------|
| `git`  | Git URL. The dependency is a git **submodule** at `dep/<id>/`, pinned by the gitlink the root repository commits. Requires `ref` or `version`. |
| `ref`  | Selector for `git`: `branch/<name>`, `tag/<name>`, or `commit/<full-object-id>`. Any other spelling is rejected (`[dep.std].ref must be branch/<name>, tag/<name>, or commit/<full-object-id>`). |
| `version` | A [version range](#version-ranges) over the dependency's releases (see [Releases and resolution](dependencies.md#releases-and-resolution)). Valid only with `git`; a path dependency has no releases. |
| `path` | Local project tree, never fetched. A relative `path` is resolved relative to this manifest's directory. `mach dep add <path> <id> --path` copies its files into `dep/<id>/` without the source's own `dep/` or Git metadata. No repository or index is required for a path dependency, and copied files are not automatically staged. Forbids `ref` and `version`. |

`git` and `path` are mutually exclusive and exactly one is required. A `git`
dependency also names exactly one selector, `ref` or `version`.

`ref` and `version` are mutually exclusive (`[dep.std] names both 'ref' and
'version'; keep one`). `ref` selects one exact commit or tag, or follows a
branch. `version` selects among releases.

How a `version` is resolved to a release, how pins are recorded, and what
`mach dep verify` and `update` check is described in
[dependencies.md](dependencies.md).

## Path templates

Paths, step `argv` entries and target `runner` entries expand over a closed, final
set of nine variables:

- `{project.work}` — the **root** project's expanded `[project].work`, in every
  manifest of the closure. In a dependency's artifact `out` it is that
  dependency's home under it, `dep/<id>`.
- `{target.name}` — the resolved target name (never the literal `native`).
- `{target.arch}` — the resolved target's `arch` (e.g. `x86_64`).
- `{target.sys}` — the resolved target's `sys` (e.g. `linux`).
- `{target.abi}` — the resolved target's `abi` (e.g. `sysv64`).
- `{profile.name}` — the selected profile name.
- `{artifact.suffix}` — the conventional filename suffix of the artifact being
  named, for its kind on the selected target (`.exe`, `.a`, `.so`, ...).
  Available only in an artifact's own `out`. See
  [Artifact filenames and identity](#artifact-filenames-and-identity).
- `{artifact.<id>.out}` — the output path of a required artifact, relative to the
  root project's directory exactly as `{project.work}` is. In a dependency's module
  it names a requirement of that dependency's export library artifact, homed under
  `dep/<id>`. See [Artifact requirements](#artifact-requirements).
- `{artifact.out}` — the path of the program a target's `runner` launches.
  Available only in a `runner`. See [Execution command](#execution-command).

The three `{target.*}` tuple keys are also exported to every step process as
`MACH_TARGET_ARCH`/`MACH_TARGET_SYS`/`MACH_TARGET_ABI` (see [build steps](#stepname--build-steps)).

Every output path is explicit and none is rooted for you: an artifact's `out`, a
step's `out` list and a local link's `path` are relative to the project root and
name `{project.work}` where they mean the build output, so an artifact may be
placed anywhere in the project. Naming `{project.work}` is what homes a
dependency's build products into the *consumer's* output tree rather than the
dependency's checkout, and a dependency's artifact `out` must begin with it.

There are no `{name}`/`{ext}` or bare `{target}`/`{profile}` aliases. Where a
template is written decides what it may name, and every template is checked when
the manifest is read, the root's and each dependency's alike: an unknown `{...}`
reference, an unterminated `{`, or a variable its place does not admit is
refused there, pointing at the template.
`{project.work}` is not available inside `[project].work` itself (it would be
self-referential), and `{artifact.<id>.out}` is not available inside an artifact's
own `out` for the same reason. `{artifact.suffix}` is available nowhere but an
artifact's own `out`. A target's `runner` admits `{project.work}`, joined to the project root there,
the `{target.*}` and `{profile.name}` variables, and `{artifact.out}`, and nothing
else. In a step's `argv` and `env` values, and in a `runner`, a brace group outside
the `project`, `target`, `profile` and `artifact` namespaces is text, so
`{"x":1}` passes through as written. Two artifacts selected for one target that resolve to the
same `out` path collide and fail at build start.

## Artifact requirements

An artifact's `need` names what must exist before it is built. Every entry
identifies its category: `step.generate` selects `[step.generate]`, while
`artifact.support` selects `[artifact.support]`. A `*` glob applies only within
that category, so `artifact.shader-*` never selects a similarly named step.
A step and an artifact may share a name. List both qualified names to require both.

A required artifact is built before its consumer, for the consumer's profile and
for the required artifact's own targets: the consumer's target when the
requirement declares it, and otherwise every target the requirement names.
A requirement reached from several consumers is built once.

Inside the consumer, `{artifact.<id>.out}` expands to that artifact's output path.
It is an error to name an artifact the consumer does not require, or one that
builds for several targets here and therefore has no single output.

```toml
[artifact.shader-blur]
kind    = "module"
entry   = "shaders/blur.mach"
out     = "{project.work}/shaders/blur.spv"
targets = ["vulkan"]
link    = []
need    = []

[artifact.app]
kind    = "bin"
entry   = "main.mach"
out     = "{project.work}/bin/app"
targets = ["linux-x86_64"]
link    = []
need    = ["artifact.shader-*"]
```

```mach fragment
#[embed("{artifact.shader-blur.out}")]
val BLUR: [_]u8;
```

An `#[embed]` argument holding a template resolves against the **project root**, as
the template's own value does; a literal `#[embed]` path keeps resolving against the
declaring file's directory. Only `{artifact.<id>.out}` may appear in an `#[embed]`
path.

Requirements are written within one manifest: a `need` entry never names another
project's artifact or step, and a consumer cannot add to a dependency's `need`.
A dependency's own requirements reach the consumer by travelling, below.

These are errors:

- a missing or malformed category prefix, including bare names;
- a name or glob matching no declaration in its named category;
- an explicit self-requirement, or a glob matching only the declaring item;
- an artifact requirement in a step's `need` list;
- artifact cycles and step cycles, including cycles formed by globs. A cycle is
  refused at its first `need` entry with the others named as related, and the
  message spells the chain, as in `'need' cycle: step.a -> step.b -> step.a`.

Globs exclude the declaring item. Matching declarations retain manifest order,
and transitive prerequisites run before their consumers. Both root and dependency
manifests receive these checks during parsing, before planning can execute a step.

A required artifact that fails to build fails its consumer, naming the requirement,
and the consumer is not attempted.

`mach check` builds nothing, and that includes required artifacts. An `#[embed]` is
read in the frontend, so a consumer that embeds a requirement's output cannot be
checked on a tree that has never been built: the check reports the output it cannot
read, names the artifact whose output it is, and says to build first. One `mach build`
produces the outputs and every later check of unchanged requirements is clean, so a
pipeline that checks before it builds should build first.

### Dependency requirements travel

A library that embeds what it builds cannot be consumed if its requirements stop
at its own manifest, and a consumer has no way to declare them. So a dependency's
**`export = true` library artifact** carries its requirements as part of its
export surface: the artifacts and steps its `need` names are built for any project
whose dependency closure holds that dependency, before the cells that compile
against the closure. Only that artifact's `need` travels, and a project exports
at most one library. `default` plays no part here: it only selects what a
command builds when no `-a` names an artifact.

```toml
# the dependency's mach.toml
[artifact.shlib]
export  = true
kind    = "static"
entry   = "lib.mach"
out     = "{project.work}/lib/shlib{artifact.suffix}"
targets = ["linux-x86_64"]
link    = []
need    = ["artifact.shader-frag"]

[artifact.shader-frag]
kind    = "module"
entry   = "shaders/frag.mach"
out     = "{project.work}/spv/frag{artifact.suffix}"
targets = ["spirv"]
link    = []
need    = []
```

```mach fragment
# the dependency's src/lib.mach, compiled by every consumer
#[embed("{artifact.shader-frag.out}")]
val FRAG: [_]u8;
```

The rules:

- **The consumer's profile, the dependency's targets.** A travelling requirement
  is built with the profile the consumer resolved, for every target it names in
  the dependency's own manifest. The consumer's target never selects among them,
  because target names of two manifests are unrelated; a requirement naming
  several targets has no single `{artifact.<id>.out}`, exactly as within one
  manifest.
- **Homed in the consumer, namespaced by id.** In a dependency's artifact `out`,
  `{project.work}` is `<expanded root [project].work>/dep/<dependency id>`, so two
  dependencies that both declare `shader-quad` produce two files and neither
  writes into its own checkout. A dependency's artifact `out` must begin with
  `{project.work}`; one placed anywhere else is refused, since it would write into
  the consumer's tree. In a dependency's steps `{project.work}` keeps meaning the
  root's out, and the cell's objects sit in the root's `obj/` beside every other
  module's. `mach clean`
  removes that home and those objects with the rest of the output, reading the
  realized dependency manifests for the target names the root never declares.
- **Scope follows the module.** `{artifact.<id>.out}` in a module the dependency
  owns is read in that dependency's manifest and checked against its export
  library artifact; the root's modules keep reading the root's manifest and the
  requirements of the artifact being built. Naming anything else is the ordinary
  refusal, located at the scope it was read in.
- **Its own closure, recursively.** A travelling requirement compiles against the
  dependency's own transitive closure, realized in the root's flat `dep/`, and
  the requirements of *those* dependencies' export library artifacts travel to
  it in turn. A requirement reached through several consumers is built once, and
  a failure names the dependency chain (`dependency app -> boom -> shader: ...`).
- **Steps too.** A step the export library artifact's `need` names runs for the
  consumer, alongside the steps its `export = true` link entries demand.

Nothing here makes an `#[embed]` an edge in the build graph: a missing embedded
file is still a compile error and still triggers nothing. What runs is
the requirement the dependency declared.

## Selection

Which targets, profiles and artifacts a command builds, and how it names its
outputs, is described in [build.md](build.md).

## Worked example: a consumer of C bindings and vendored C

A project that uses a system-lib binding (`glfw`) and a vendored-C library
(`miniz`), building a native binary and cross-compiling to windows. The platform
shim is built by steps and linked through `local` entries; the OS-specific shims
are gated by their link entries' `sys` axis, so the x11 step runs on a linux build
and never on a windows one.

```toml
[project]
id      = "demo"
version = "0.1.0"
mach    = "^5.3"
src     = "src"
work    = "out/{target.name}/{profile.name}"

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
path   = "{project.work}/obj/platform/shim.o"
sys     = ["*"]
arch    = ["*"]
abi    = ["*"]
export = false

[link.shim-x11]
source = "local"
path   = "{project.work}/obj/platform/x11.o"
sys     = ["linux"]
arch    = ["*"]
abi    = ["*"]
export = false

[link.shim-win32]
source = "local"
path   = "{project.work}/obj/platform/win32.o"
sys     = ["windows"]
arch    = ["*"]
abi    = ["*"]
export = false

[link.gl]
source = "system"
name   = "GL"
sys     = ["linux"]
arch    = ["*"]
abi    = ["*"]
export = false

[artifact.demo]
kind    = "bin"
entry   = "main.mach"
out     = "{project.work}/bin/demo"
targets = ["linux", "windows"]
link    = ["shim", "shim-x11", "shim-win32", "gl"]
need    = []

[step.shim]
argv = ["cc", "-c", "-O2", "-fPIC", "-Ivendor/platform", "-o", "{project.work}/obj/platform/shim.o", "vendor/platform/shim.c"]
in   = ["vendor/platform/shim.c"]
out  = ["{project.work}/obj/platform/shim.o"]
need = []

[step.shim-x11]
argv = ["cc", "-c", "-O2", "-fPIC", "-Ivendor/platform", "-o", "{project.work}/obj/platform/x11.o", "vendor/platform/x11.c"]
in   = ["vendor/platform/x11.c"]
out  = ["{project.work}/obj/platform/x11.o"]
need = []

[step.shim-win32]
argv = ["cc", "-c", "-O2", "-fPIC", "-Ivendor/platform", "-o", "{project.work}/obj/platform/win32.o", "vendor/platform/win32.c"]
in   = ["vendor/platform/win32.c"]
out  = ["{project.work}/obj/platform/win32.o"]
need = []

[target.linux]
arch = "x86_64"
sys  = "linux"
abi = "sysv64"

[target.windows]
arch = "x86_64"
sys  = "windows"
abi = "win64"

[profile.debug]
optimize = false
debug = true
simd  = "scalarize"

[profile.release]
optimize = true
debug = false
simd  = "scalarize"
```

The `gl` and `shim-x11` entries carry `sys = "linux"`, so on a windows build cell
they filter out (and `shim-x11`'s step is never demanded); `shim-win32` carries
`sys = "windows"` and applies only there. The unconditional `shim` entry (`sys = "*"`)
applies to both.

## Worked example: a C-binding dependency's export

`mach-glfw` exports its `system`/`framework` link entries — a consumer that imports
its modules inherits every `export = true` entry that matches the build: linux and
darwin pull the `glfw` system library, a windows build pulls `glfw3.dll`, and the
darwin frameworks apply only on darwin.

```toml
[project]
id      = "glfw"
version = "0.3.0"
src     = "src"
work    = "out/{target.name}/{profile.name}"

[link.glfw]
source = "system"
name   = "glfw"
sys     = ["linux", "darwin"]
arch    = ["*"]
abi    = ["*"]
export = true

[link.glfw-win]
source = "system"
name   = "glfw3.dll"
sys     = ["windows"]
arch    = ["*"]
abi    = ["*"]
export = true

[link.Cocoa]
source = "framework"
name   = "Cocoa"
sys     = ["darwin"]
arch    = ["*"]
abi    = ["*"]
export = true
```

The binding module selects the entry's key once with `$if`, so every
declaration carries the same attribution on every target:

```mach
use std.types.string.str;

$if ($mach.build.sys == $mach.sys.windows) {
    val GLFW: str = "glfw-win";
}
$or {
    val GLFW: str = "glfw";
}

#[library(GLFW)]
pub ext fun glfwInit() i32;
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
work    = "out/{target.name}/{profile.name}"

[link.miniz]
source = "local"
path   = "{project.work}/obj/miniz/miniz.o"
sys     = ["*"]
arch    = ["*"]
abi    = ["*"]
export = true

[step.miniz]
argv = ["cc", "-c", "-O2", "-fPIC", "-Ivendor/miniz", "-o", "{project.work}/obj/miniz/miniz.o", "vendor/miniz/miniz.c"]
in   = ["vendor/miniz/*.c", "vendor/miniz/*.h"]
out  = ["{project.work}/obj/miniz/miniz.o"]
need = []
```

## The compiler's own manifest

The compiler builds itself from the `mach.toml` at the root of its repository, which is the one current example of a multi-target, multi-artifact manifest with a dependency.

## See also

- [files.md](files.md) — file layout and `lib.mach` / `main.mach`
- [modules.md](modules.md) — how files map to module paths
- [ext-fun.md](ext-fun.md) — linking against external symbols
