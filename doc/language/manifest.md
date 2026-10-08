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

A manifest refusal points at what it refuses, on the line after the message:
the value it rejects, the key token of a name or key it rejects, or the table a
required key is missing from, in the manifest that states it, a dependency's
own included. [`--diagnostics json`](diagnostics-json.md#failure-records)
carries the same place as the failure's `primary` span.

```
error[manifest.invalid_value]: mach.toml: profile 'debug': simd must be "scalarize" or "require" (got "fast")
 --> mach.toml:15:8
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
project id, the module a bare `use <id>;` binds (see
[modules.md](modules.md#bare-project-id-imports)), its
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
isa = "x86_64"
os  = "linux"
abi = "sysv64"

[profile.debug]                        # a build variant; every manifest declares at least one
optimize = false                       # run the optimization pass set
debug = true                           # emit debug info for this profile
simd  = "scalarize"                    # SIMD lever: "scalarize" | "require"
# pass  = ["inline"]                   # optional: optimization passes added to the set
# skip  = ["vectorize"]                # optional: optimization passes left out of it
# relax = ["float-reassoc"]            # optional: departures from exact semantics
# allow = ["import.unused"]            # optional: warning keys this profile silences

[artifact.demo]                        # a produced artifact
kind    = "bin"                        # "bin" | "static" | "shared"
entry   = "main.mach"                  # entry source, relative to src
out     = "{project.work}/bin/demo{artifact.suffix}"  # output path, under the work directory
targets = ["*"]                        # which declared targets build it ("*" = all)
# link  = ["kernel32"]                 # optional: [link.X] names this artifact links
# need  = ["step.generate"]            # optional: step.X / artifact.X requirements
# subsystem = "gui"                    # optional: windows console/GUI selector
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
| `work`    | string | The work directory template: everything a build writes besides its artifacts, referenced as `{project.work}` by artifact `out`, step `argv`, `in` and `out`, and local link paths. Expanded over `{target.name}`/`{target.isa}`/`{target.os}`/`{target.abi}`/`{profile.name}` (see [Path templates](#path-templates)). `-w` names another work directory for one build or test. |
| `mach`    | string | The compiler versions this project builds with, as a [version range](#version-ranges) (`"^5.3"`). Required in every manifest, a dependency's included. See [Compiler range](#compiler-range). |
| `out`     | removed | Refused as a removed key: the work directory is `work`. Its refusal carries the rename as a fix. |

`[project]` is exactly the five keys with a type, and every one is required. Any other
key, `name` and `description` included, is an unknown-key error
(`mach.toml: unknown key 'name' in [project]`). The previous major's `out` is
refused by name, with the rename to `work` as its fix, and so is `{project.work}`
in any template, with `{project.work}` as its fix. `[profile.<name>]`
likewise carries no `emit_ir` or `emit_asm`: emission is `--emit-ir`/`--emit-asm`
on the command line.

### The work directory

Everything a build and its cache write besides its artifacts lands under the
expanded `work`, in one layout:

| Path | Holds |
| --- | --- |
| `obj/` | one object per module, each carrying its cache key; it is the [object cache](#stepname--build-steps) and is read by developers and tooling too |
| `ir/`, `asm/` | the human-readable views `--emit-ir` and `--emit-asm` write |
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
`dep/` along with every artifact output, for every declared target and profile,
so the build after it is a cold one that reuses nothing. A step output may not
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
| `isa` | yes      | Instruction-set architecture. Read by `$project.target.arch`. |
| `os`  | yes      | Operating system. Read by `$project.target.os`. |
| `abi` | yes      | Application binary interface. Read by `$project.target.abi`. |
| `of`  | absent: the os's format | Object-format override; defers to the os's format when omitted. See [Object-format override](#object-format-override). |
| `base` | absent: the os's base | Load-address override (integer). Overrides the os's default base virtual address; defers to it (`0` for `freestanding`) when omitted. |
| `platform` | absent: no platform | Open platform tag (string), surfaced to comptime as `$mach.build.platform` (empty when unset). A support library keys its backend on it; the compiler treats it as opaque. See [Platform targets](#platform-targets-bare-metal). |
| `stack_reserve` | absent: the format's default | Thread stack reserve in bytes. See [Image stack size](#image-stack-size). |
| `stack_commit` | absent: the format's default | Thread stack commit in bytes. See [Image stack size](#image-stack-size). |
| `default` | removed | Refused as a removed key. A target has no default: with no `-t` a command takes the declared target matching the host. See [`native` target resolution](build.md#native-target-resolution). |
| `extensions` | absent: none selected | Array of instruction-set extension names the target may assume, such as `["sha", "ssse3"]`. Each name must be in the isa's vocabulary. See [Instruction-set extensions](#instruction-set-extensions). |
| `env` | absent: the isa's default | Consumer environment (string). The values are owned by the target's isa: an `env` the isa does not define is a manifest error naming the target and the known values, and an isa that defines none refuses the key outright. Today only `spirv` defines any; see [Finished-module targets](#finished-module-targets). |

### Instruction-set extensions

`extensions` lists the extensions a target may assume beyond its isa's baseline:

```toml
[target.linux-x86_64-sha]
isa        = "x86_64"
extensions = ["sha", "ssse3", "sse41"]
os         = "linux"
abi        = "sysv64"
```

Each isa owns its vocabulary. The names are identifiers, so each one is also a
comptime member, `$mach.build.extensions.<name>` (see [`$mach`](comptime-mach.md)):

| `isa` | Baseline | Extensions |
|-------|----------|------------|
| `x86_64` | SSE2 | `ssse3`, `sse41`, `sse42`, `sha`, `fsgsbase`, `popcnt`, `lzcnt`, `bmi1`, `bmi2`, `cx16`, `avx`, `avx2`, `fma`, `movbe`, `f16c`, `avx512f`, `avx512bw`, `avx512cd`, `avx512dq`, `avx512vl`, `aes`, `pclmul` |
| `aarch64` | AdvSIMD | `sha2`, `sb`, `aes`, `pmull`, `fp16` |
| `riscv64`, `riscv32` | the isa string's selection | `i`, `m`, `a`, `f`, `d`, `c`, `zicond`, `zicsr`, `zifencei`, `zfhmin`, `zfh`, `zkt` |
| `spirv` | | `float16`, `int8`, `int16`, `int64`, `float64`, `zero_init_workgroup`, `storage_read_without_format`, `storage_write_without_format`, `vulkan_memory_model`, `vulkan_memory_model_device_scope`, `buffer_device_address`, `subgroup_arithmetic`, `subgroup_clustered`, `subgroup_vote`, `subgroup_ballot`, `subgroup_shuffle`, `subgroup_shuffle_relative`, `subgroup_quad`, `subgroup_graphics_stages`, `buffer_int64_atomics`, `shared_int64_atomics`, `buffer_float32_atomics`, `buffer_float32_atomic_add`, `buffer_float32_atomic_min_max`, `buffer_float64_atomics`, `buffer_float64_atomic_add`, `buffer_float64_atomic_min_max`, `shared_float32_atomics`, `shared_float32_atomic_add`, `shared_float32_atomic_min_max`, `shared_float64_atomics`, `shared_float64_atomic_add`, `shared_float64_atomic_min_max`, `buffer_float16_atomics`, `buffer_float16_atomic_add`, `buffer_float16_atomic_min_max`, `shared_float16_atomics`, `shared_float16_atomic_add`, `shared_float16_atomic_min_max`, `image_int64_atomics`, `image_float32_atomics`, `image_float32_atomic_add`, `image_float32_atomic_min_max`, `storage_image_multisample`, `resource_min_lod`, `image_gather_extended`, `maintenance8`, `storage_buffer_16bit_access`, `uniform_and_storage_buffer_16bit_access`, `storage_push_constant16`, `storage_input_output16`, `storage_buffer_8bit_access`, `uniform_and_storage_buffer_8bit_access`, `storage_push_constant8` |

On `spirv` each name is a device feature a module may require.
`vulkan1.3` selects `zero_init_workgroup`, `storage_read_without_format`,
`storage_write_without_format`, `vulkan_memory_model`,
`vulkan_memory_model_device_scope` and `buffer_device_address`. The atomic and
sampling features are described with [`op`](gpu.md#optarget-set-name--a-function-that-is-a-target-instruction),
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

A level is a bundle, never an axis of its own: each name may imply others, and the
selection is closed over that once, when the target resolves. `sse41` brings `ssse3`
(the chain stops there; SSE3 is not modelled), `sse42` brings `sse41`, `avx` brings
`sse42`, `avx2`, `fma` and `f16c` bring `avx`, and every `avx512*` set brings
`avx512f`, which brings `avx2`. On aarch64 `pmull` brings `aes`. On riscv `d` brings
`f`, `zfh` brings `zfhmin`, `zfhmin` brings `f`, and `f` brings `zicsr`, as the isa string's own grammar has it, so `extensions = ["d"]` on `rv64i` selects
`rv64ifd` with Zicsr. The isa string and the list feed one set: `isa = "rv64i"` with
`extensions = ["m"]` selects the same machine as `isa = "rv64im"`. A name nothing in
the compiler encodes against yet (`avx`, `avx512f`) is still a declared requirement:
the inline assembler has no rows to admit under it, so today it records only the
promise the binary makes about its hosts, and the promise is the program's to check.

#### Levels

x86-64 also spells the published microarchitecture levels. A level is a manifest
spelling that expands to its member names, so it may stand alone or beside names
(`["x86-64-v2", "sha"]`), and it includes every lower level. It has no bit of its own:
`$mach.build.extensions.x86-64-v2` and `#[extensions("x86-64-v2")]` do not exist,
programs ask about the members (`.avx2`). `sse2`, `cmpxchg8b` and `lahf-sahf` are the
`x86_64` baseline and are not names. The table here is the compiler's, held together by
a test:

| Level | Members beyond the level before |
|-------|----------------------------------|
| `x86-64-v2` | `ssse3`, `sse41`, `sse42`, `popcnt`, `cx16` |
| `x86-64-v3` | `avx`, `avx2`, `bmi1`, `bmi2`, `fma`, `lzcnt`, `movbe`, `f16c` |
| `x86-64-v4` | `avx512f`, `avx512bw`, `avx512cd`, `avx512dq`, `avx512vl` |

`native` is not a spelling and never a default: the hardware requirement is visible
in the manifest, never inferred from the build machine.

The list is never part of `{target.isa}`. That placeholder is the `isa` value as
written (`rv64i`, `x86_64`), on every isa; the list belongs to the target's identity
and to `{target.name}`.

"Selects" means the extension is assumed of every machine the binary runs on: the
inline assembler admits its rows, `$mach.build.extensions.<name>` answers 1, and a
property the extension declares (Zkt's data-independent timing, which the
constant-time multiply rows read) is taken as given. It never means a mode is on. A
row such as a `dit` would admit `msr dit`, not set it.

The `extensions` list is the manifest's only lever over the constant-time multiply,
and only on riscv64, where `zkt` is what admits a secret `*`. There is no key that
declares or overrides a timing mode. On aarch64 the condition is PSTATE.DIT, which
the operating system declares it guarantees (linux and darwin) and the linked
program's start code turns on for a binary that needs it; a manifest cannot assert
it for an OS that declares nothing. On x86-64 the multiply rows hold unconditionally
on Intel and AMD, so nothing is there to declare, and Intel's DOITM is a kernel-owned
model-specific register that hardens memory-side predictors rather than the
multiplier, so mach offers no key for it either. The rows, their conditions and
the DOITM note are in
[secrecy.md](secrecy.md#constant-time-multiply-by-instruction-set).

Some rows are the target's alone. On riscv `i` is the baseline, `c` is a code-size
selection mach never emits, and `f` and `d` select the float register file and the
calling convention's float registers, and `zkt` is a promise about the machine's
execution timing that the constant-time rows read, so none of them may be named in
[`#[extensions(...)]`](decorators.md#extensionsnames--an-outlier-function); the
refusal says why. So are spirv `float16`, `int8`, `int16`, `int64` and `float64`,
the device features that let the whole module use a type of that width, and spirv
`zero_init_workgroup`, the device feature
that zero-initializes the workgroup memory of every
[`#[shared]`](gpu.md#inputn--outputn--builtinstr--uniformset-binding--storageset-binding--samplerset-binding--push--specid--shared--shader-interface)
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
[`#[extensions(...)]`](decorators.md#extensionsnames--an-outlier-function) functions
and picks one of those at run time, after detecting the host's features.

A mnemonic that needs an extension the target does not select, outside such a
function, is refused. The refusal names the line to add:

```
error[asm.extension]: inline-asm instruction 'sha256rnds2' needs the `sha` extension, which
this target does not select; add `extensions = ["sha"]` to the target, or mark the
function `#[extensions(sha)]` and call it only after detecting the extension at run time
```

The selected set is part of the target's identity: two targets that differ only in
`extensions` never share cached products.

The selection also reaches code generation. Every vector operation is legal on every
target and its shape never depends on the extension list; what moves is the lowering.
Each packed row of a target's catalog declares the extension its instruction needs,
and the lowering reads those rows: a cell with a baseline row and a row gated on an
extension lowers to the gated instruction when the selected set holds it (`i32x4 *
i32x4` is `pmulld` under `sse41` on x86_64 and the `pmuludq` pair without). A cell
whose only packed row is gated scalarizes without the extension, and the warning at
each such site names it (see `simd` below), and
[`simd = "require"`](#profilename) judges against the selected set, so a kernel refused on
the baseline is accepted once the target declares the extension it needs. SSE2 is the
x86_64 baseline. A build never infers the build machine's features: what the binary
assumes is what the manifest declares.

### Image stack size

`stack_reserve` and `stack_commit` set the thread stack an image asks its loader for,
as byte counts:

```toml
[target.windows]
isa = "x86_64"
os  = "windows"
abi = "win64"
stack_reserve = 0x800000        # 8 MiB
stack_commit  = 0x1000          # 4 KiB
```

Both are optional. Omitting them keeps the format's conventional default, so an image
built without them is byte-identical to one built before the keys existed. On PE that
default is a 1 MiB reserve and a 4 KiB commit.

A **reserve** is address space, not committed memory, so raising it costs nothing until
the stack is actually used. That is also why these live on the target rather than the
artifact: two artifacts built for one target share the value, and a small tool
inheriting a large program's reserve pays nothing for it.

**Only some object formats carry a stack size.** PE keeps both in its optional header
and Mach-O keeps a reserve in `LC_MAIN.stacksize`. ELF has nowhere to put one — a linux
main thread's stack is the kernel's and `ulimit`'s business — and a raw flat image has
no header at all. Either key on a target whose resolved object format carries no stack
size is refused when the manifest is read, naming the key, the target and the format,
before anything builds:

```
error[target.stack_size]: mach.toml: [target.lin].stack_reserve is not expressible on the `elf` object
format, which carries no stack size in its image headers
```

Every declared target is checked, not only the one being built, so the mistake is found
on the first build rather than whenever someone happens to build that cell.

A function whose own stack frame exceeds the reserve is refused at build time, naming
the function, its frame size and the reserve:

```
error[stack.reserve_exceeded]: `main` needs a 1107824-byte stack frame, which its target's
1048576-byte stack reserve cannot hold; raise `stack_reserve` on the target, or move
the large locals off the stack
```

That is a proof rather than an estimate - one frame against one reserve, with no call
graph and no input dependence - so it is an error and has no false positives. Targets
whose stack is not bounded by the image, such as every ELF target, are not checked.

On Mach-O the value reaches only a **position-independent** image. A non-PIE one enters
through `LC_UNIXTHREAD`, which has no stacksize member, and a stack size requested for
such an image is refused at link rather than silently dropped.

### Accepted tuple values

| Axis  | Values |
|-------|--------|
| `isa` | `x86_64`, `aarch64`, `riscv64`, `riscv32`, `spirv` |
| `os`  | `linux`, `windows`, `darwin`, `freestanding` |
| `abi` | `sysv64`, `win64`, `aapcs64`, `lp64`, `lp64f`, `lp64d`, `ilp32`, `ilp32f`, `ilp32d`, `spirv` |
| `of`  | `elf`, `macho`, `coff`, `raw`, `spv` |
| `env` | `vulkan1.0`, `vulkan1.1`, `vulkan1.2`, `vulkan1.3` |

`isa` also takes a canonical RISC-V extension string such as `rv32imc`, described
below. `env` is defined only by `spirv`.

A `[target.*]` naming an `isa`, `os` or `abi` outside these lists is refused
through the registry's own lookup (`no isa implementation registered for
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
[Finished-module targets](#finished-module-targets)).

`riscv64` and `riscv32` are width-only spellings, and each names a **default
profile**: `riscv64` is `rv64gc` and `riscv32` is `rv32imac`. A canonical
extension string such as `rv32imc` or `rv64imafd` selects a smaller machine.
The retained vocabulary is I, M, A, F, D, C, Zicond, Zicsr, Zifencei and Zkt,
written in lowercase canonical order with multi-letter names after an
underscore, so `rv64gc_zicond`; `g` expands to IMAFD plus Zicsr and Zifencei.
F carries its required Zicsr, and D requires F. Zicond adds `czero.eqz` and
`czero.nez`, which a branch-free select compiles to where a selection holds it
and the xor-and-mask sequence it replaces does otherwise. Zkt changes no
instruction. It states that the listed operations run in data-independent time,
which is what lets a secret multiply compile (see `secrecy.md`). An optional
version must be the one mach models: I 2.1, M 2.0, A 2.1, F and D 2.2, C 2.0,
Zicond 1.0, Zicsr and Zifencei 2.0, Zkt 1.0. Unknown extensions,
other versions, duplicates, noncanonical order and the E base are refused
rather than rounded up to the default machine.

The selected ISA bounds what the compiler generates and what named inline
assembly may use: an instruction needing an extension the selection lacks is
refused with a diagnostic naming that extension. A foreign object's
`Tag_RISCV_arch` must declare only selected extensions at the modeled versions
and the same register width, and its header flags may not claim compressed
code without C; linking never widens the selection. A raw `.word` directive is
the documented unchecked encoding boundary. C is accepted as a capability of the
selected machine, but the emitter writes full-width instructions only.

The ABI still selects the calling convention on its own, and it must fit the
selected machine: `rv32imc` has no floating-point registers, so it takes
`ilp32`, and `riscv32` (rv32imac) is refused with `ilp32f` or `ilp32d`; spell
`rv32imafdc` when RV32 hardware floating point is wanted. `mach init` scaffolds
`riscv32`/`freestanding` with `ilp32` for that reason.

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
every convention the instruction set covers. `riscv32`
currently only reaches a `freestanding` target — `mach info targets` lists
`freestanding-riscv32` rows and no `linux`/`darwin` riscv32 row.

`mach info targets` prints every tuple this binary can actually build, one
`(os, isa, abi, object)` cell per line with every dimension spelled; it is
derived from the same declarations composition reads, so it never advertises a
tuple that would fail to resolve. `mach info` alone prints the tuple the host
resolves to.

A value outside its axis's set is refused when the target resolves, so a typo is
caught rather than silently never matching.

### Object-format override

`of` overrides the object format a target implies. Each os has a default format —
`linux` → `elf`, `windows` → `coff`, `darwin` → `macho`, `freestanding` → `raw` —
and `of` names a different one from the same closed set: `elf`,
`coff`, `macho`, `raw`, `spv`. `of` is optional; omit it to
take the default. An os accepts only the formats it can load, so an override the
os cannot enter is refused.

```toml
[target.metal]
isa = "x86_64"
os  = "freestanding"   # os default object format is "raw"
abi = "sysv64"
of  = "elf"            # override: emit an ELF object instead
```

The default is a function of the whole tuple, not the os alone. An os default
carries relocatable machine text, which a whole-module emitter does not produce,
so a `spirv` target resolves to `spv` — the format that carries a finished
module — regardless of the os it names.

An `of` naming a format whose emission shape does not match the instruction set's
is refused at composition (`instruction set 'spirv' emits finished modules, but
object format 'raw' carries linkable objects`), so the override cannot compose a
tuple that would emit nothing.

The page an image is laid out at is a function of the same tuple. A format a
loader maps by page (`elf`, `coff`, `macho`) places every load segment on a page
of its own, so no two segments with different permissions share one: the os's
page where the os declares one (`linux` on `aarch64` lays out at 64 KiB, the
largest page a kernel may use, `darwin` on `aarch64` at 16 KiB, 4 KiB elsewhere),
and the instruction set's hardware page (4 KiB on `x86_64`, `aarch64`, `riscv64`
and `riscv32`) where the os has no loader of its own, which is what `freestanding`
with `of = "elf"` gives a bootloader such as Limine or GRUB. A flat image (`raw`)
and a finished module (`spv`) have no page and are laid out byte-tight.

### Finished-module targets

A `spirv` target's object output is a complete, self-contained module rather than
a link input. Each module is written to `<out>/obj/<fqn-as-path>.spv` like any
other target's objects, and the entry module already carries every function its
stages reach, so it is the whole deliverable. A `bin` artifact therefore needs no
linker: the build publishes the entry module at the artifact's resolved `out` (or
`-o`), and `--emit obj` stops at the module tree:

```toml
[target.gpu]
isa = "spirv"
os  = "freestanding"
abi = "spirv"
# no `of`: the finished-module format resolves on its own
```

```
mach build . --target gpu     # writes the entry module at the artifact's out, and out/gpu/<profile>/obj/<module>.spv
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
it too. A target for an earlier version selects it with `extensions` when its consumer
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
for `vulkan1.1` or `vulkan1.2` selects it with `extensions` when its consumer enables
the feature, and one for `vulkan1.0` is refused, since the model needs SPIR-V 1.3.
Under the Vulkan model a memory scope of `Device` needs
`vulkanMemoryModelDeviceScope` as well, which `vulkan1.3` also requires: an
instruction taking that scope is refused unless the target selects
`vulkan_memory_model_device_scope`, which brings `vulkan_memory_model` with it. Under
GLSL450 the `QueueFamily` scope and the `MakeAvailable`, `MakeVisible` and `Volatile`
memory semantics, which only the Vulkan model defines, are refused. What
`"coherent"` and `#[shared]` mean under each model is in
[gpu.md](gpu.md#inputn--outputn--builtinstr--uniformset-binding--storageset-binding--samplerset-binding--push--specid--shared--shader-interface).

Physical pointers follow it too. A module holding one, a pointer stored in memory or
made from an address ([types.md](types.md#pointers-on-spir-v)), is refused unless the
target selects `buffer_device_address`, Vulkan's `bufferDeviceAddress` feature, and
then declares the `PhysicalStorageBufferAddresses` capability, the
`PhysicalStorageBuffer64` addressing model and, below SPIR-V 1.5, the
`SPV_KHR_physical_storage_buffer` extension. `vulkan1.3` requires the feature of every
device, so it selects the extension, and a module without `env` has it too. A target
for an earlier version selects it with `extensions` when its consumer enables the
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
and the target selects one with `extensions` when its consumer enables it. A module
without `env` has all five. The ceiling below still bounds them, so `int8` or
`float16` under `vulkan1.0` is refused for the environment, not the feature.

Under `float16` an `f16` is the native `OpTypeFloat 16` wherever it lives, computed,
negated and converted by the core float instructions, so an `f16` shader needs
`float16` alone. A local, a parameter or a result is that type, and so is an `f16` in
memory the host or the workgroup shares, a stage input or output, a storage buffer, a
uniform or push block, a record a physical pointer reaches or a `#[shared]` variable,
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
width, so an 8-bit or 16-bit one, or a vector of them, in a local or in `#[shared]`
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
| `#[storage(...)]`, or a record a physical pointer reaches | `storage_buffer_16bit_access` (`StorageBuffer16BitAccess`) | `storage_buffer_8bit_access` (`StorageBuffer8BitAccess`) |
| `#[uniform(...)]` | `uniform_and_storage_buffer_16bit_access` (`UniformAndStorageBuffer16BitAccess`) | `uniform_and_storage_buffer_8bit_access` (`UniformAndStorageBuffer8BitAccess`) |
| `#[push]` | `storage_push_constant16` (`StoragePushConstant16`) | `storage_push_constant8` (`StoragePushConstant8`) |
| `#[input(n)]`, `#[output(n)]` | `storage_input_output16` (`StorageInputOutput16`) | refused |

A stage output starts at its initializer, and at zero without one. A constant of a
16-bit type needs the type's own feature, `int16` or `float16`, so an output holding a
16-bit scalar without it carries no zero constant: each stage that reaches it stores
the zero first, converted from a 32-bit one, and the output needs
`storage_input_output16` alone. One initialized to anything but zero starts at that
constant, which needs the type's feature as well. The 16-bit capabilities need
`SPV_KHR_16bit_storage` below SPIR-V 1.3 and the 8-bit ones `SPV_KHR_8bit_storage`
below SPIR-V 1.5. Vulkan defines no 8-bit stage input or
output, so one is refused, and an 8-bit member of a `#[storage(...)]` buffer is
refused under `vulkan1.0`, whose storage buffer is a `BufferBlock` in the `Uniform`
class, which the 8-bit feature does not reach.

```toml
[target.gpu]
isa        = "spirv"
os         = "freestanding"
abi        = "spirv"
env        = "vulkan1.0"
extensions = ["int16", "int64", "float64"]
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
isa = "spirv"
os  = "freestanding"
abi = "spirv"
env = "vulkan1.2"
```

The artifact's `out` template and `-o` name that delivered module, so
`{artifact.suffix}` gives it `.spv`, and `{artifact.<id>.out}` names it for a
consumer that embeds it (see [Artifact requirements](#artifact-requirements)). A
`static` or `shared` artifact kind, and `mach test`, are refused by name, since
there is no archive, shared object, or executable form for a module.

### Platform targets (bare metal)

A bare-metal platform — such as [BareMetal](https://github.com/ReturnInfinity/BareMetal)
(`bmos`), Return Infinity's x86-64 exokernel — is not its own `os`. It is
`os = "freestanding"` plus two optional keys: a `base` load-address override and an
open `platform` tag a support library keys its backend on (surfaced to comptime as
`$mach.build.platform`). A BareMetal target:

```toml
[target.bmos]
isa      = "x86_64"
os       = "freestanding"
abi      = "sysv64"
base     = 0xFFFF800000000000   # BareMetal copies the flat image here and calls it
platform = "bmos"               # selects the mach-bmos backend
# no `of`: freestanding's default object format is "raw"
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
| `optimize` | bool | Whether the optimization passes run. `true` runs the default set `mach info passes` lists with `default=optimize` over the passes every build runs (`default=always`); `false` runs only the latter. The legalization passes, which make the program something the target can execute, run at every setting. A non-boolean is a manifest error. |
| `pass` | array of strings | **Optional.** Optimization passes added to the set this profile's `optimize` selects, each a name from [`mach info passes`](#passes), listed once. Absent, nothing is added. |
| `skip` | array of strings | **Optional.** Optimization passes left out of the set, each listed once and never in `pass` too. Absent, nothing is left out. |
| `relax` | array of strings | **Optional.** The departures from exact semantics the build permits. The one there is, `"float-reassoc"`, is described under [Float reassociation](#float-reassociation). Absent, the build keeps exact semantics. |
| `debug` | bool    | Emit debug info for this profile: DWARF in ELF, Mach-O and COFF objects alike, and the core SPIR-V debug instructions on a `spirv` target (see [Finished-module targets](#finished-module-targets)). A PE image carries its DWARF in `.debug_*` sections, which gdb, lldb and the LLVM tools read and Visual Studio and WinDbg do not. Gates emission only, never the optimizer, so a `release` profile can keep symbols with `debug = true`. A non-boolean is a manifest error. |
| `simd`  | string  | SIMD scalarization lever. `"scalarize"` emits a defined unrolled scalar expansion wherever the target has no packed instruction for a vector operator, with one `vector.scalarize` warning at each such operation naming the operation, its lanes, the function, the target and the extension that would pack it (or that none would). `"require"` makes each of those sites a hard error with the same text. It applies **per operation on every target**, not only to targets with no vector unit: x86-64's SSE2 baseline has no 32-bit lane integer multiply and NEON has no 64-bit one, so a capable target scalarizes too. Any other string is a manifest error. |
| `default` | bool | **Optional.** `true` marks the profile a build uses when several are declared and `--profile` is absent. Exactly one profile may carry it. See [Profile requirement and selection](#profile-requirement-and-selection). |
| `allow` | array of strings | **Optional.** The warnings this profile silences, each entry a key or a family of keys from the [warning key table](#silencing-warnings), named once. An unknown key, an entry that covers only errors, a repeated entry or a non-string entry is a manifest error. Absent, nothing is silenced. |
| `opt` | removed | Refused as a removed key: write `optimize = true` or `optimize = false`. |
| `vectorize` | removed | Refused as a removed key: vectorization is the `vectorize` pass of the set `optimize` selects, left out with `skip = ["vectorize"]`. |
| `float_reassoc` | removed | Refused as a removed key: reassociation is permitted with `relax = ["float-reassoc"]`. |

Three keys (`optimize`, `debug`, `simd`) are required in every profile; the
others are optional, with the meaning of their absence above. A missing key is a
manifest error naming the table and the key:

```
error[manifest.missing_required]: mach.toml: [profile.debug] is missing required key 'simd'; a profile declares optimize, debug and simd
```

A pass name that is no pass, or names a legalization pass, which every build
runs and none can add or skip, is refused where it is written, and so is a
relaxation that does not exist:

```
error[manifest.invalid_value]: mach.toml: [profile.release].skip names "vecsplit", a legalization pass, which every build runs and none can add or skip
```

The previous major's keys are refused by name, each with what replaced it, and
with the rewrite `mach migrate` applies as a fix wherever the value has a
mechanical one: `opt = 0` is `optimize = false`, `opt = 1` and `opt = 2` are
`optimize = true`, `vectorize = true` is the default set and is removed,
`vectorize = false` is `skip = ["vectorize"]`, `float_reassoc = false` is
removed and `float_reassoc = true` is `relax = ["float-reassoc"]`:

```
error[manifest.removed_key]: mach.toml: [profile.release] uses removed key 'opt'; write `optimize = true` or `optimize = false`
 --> mach.toml:12:1
 = fix: write `optimize = true`
 --> mach.toml:12:1
   -> replace with `optimize = true`
```

### Passes

The middle end runs one schedule of passes, and `mach info passes` lists every
one of them in the order it first runs, with its class and the setting that
puts it in the default set:

```
mem2reg          class=optimization  default=always
inline           class=optimization  default=optimize
vectorize        class=optimization  default=optimize
vecsplit         class=legalization  default=always
```

An **optimization** pass only improves the code: `optimize` selects it (or every
build runs it, `default=always`), `pass` adds it at either setting and `skip`
leaves it out. A **legalization** pass rewrites what the target cannot execute
and runs in every build; no profile and no flag can add or skip one. Vectorizing
a loop is the `vectorize` optimization pass, which a target without 128-bit
vectors never runs.

### Profile requirement and selection

Every manifest declares at least one `[profile.*]` table, a dependency's
included. One that declares none is refused:

```
error[manifest.missing_required]: mach.toml: missing [profile] table; a build needs an explicit [profile.<name>] declaring optimize, debug and simd
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
concern — it is controlled only by the `--emit-ir` / `--emit-asm` flags of
`mach build`.

Skipping `vectorize` only ever *subtracts*. The pass runs when `optimize` selects it, on targets that report 128-bit vector support (SSE2 on x86-64, NEON on
aarch64, and `OpTypeVector` on spirv) and rewrites counted, unit-stride loops whose dependence analysis proves
independence — element-wise maps behind a runtime alias guard, and associative-exact
integer reductions. A loop it cannot prove safe stays scalar, and a target without
hardware vectors (riscv64) never enters the pass, so `skip = ["vectorize"]` changes
performance and never semantics. For a single function, the `#[scalar]` decorator is
the finer-grained opt-out (see [decorators.md](decorators.md)).

`relax = ["float-reassoc"]` is the one lever here that *adds*, and the only profile
key that can change a program's computed answer. It widens that same pass to float
reductions and does nothing else; skipping `vectorize` switches the pass off wholesale
and so overrides it. A relaxation never rides on `optimize`.

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

A kind decided by source warns or not from the source text alone, whatever the
target, goal or profile. Only such a key is reported when an `#[expect]`
naming it goes unfulfilled; the others depend on what the build compiles and
for which target, so an expectation of one can be quiet in a given build.

Only warnings can be silenced. The table also keys error kinds, which print
their key as well, and naming one in `allow` is refused rather than read as
unknown:

| Key | Error |
|---|---|
| `secret.not_oblivious` | a function performs a constant-time operation on a secret value without `#[oblivious]` |

```
error[allow.error_key]: mach.toml: [profile.release].allow entry "secret.not_oblivious" names an error; only a warning can be silenced
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

The relaxation is profile-wide. For a single function, `#[scalar]` opts out of vectorization
entirely and takes precedence over it, so a routine that must stay IEEE-strict inside
an otherwise-reassociating build has a spelling. There is no per-function opt-*in*:
whether a reduction may be reassociated is the caller's tolerance to decide, not the
callee author's.

Integer reductions are untouched by this relaxation. They vectorize unconditionally and are
bit-identical to the scalar reference, because integer add / xor / or / and reassociate
exactly.

### Levers on the command line

`-p <name>` picks the profile, and every lever has its command-line form over
it for one invocation. A binary fact is a `--key` and `--no-key` pair, a value
is `--key value`, and a set takes repeatable add and remove flags:

| Key | Flags |
|---|---|
| `optimize` | `--optimize` (`-O`), `--no-optimize` |
| `debug` | `--debug` (`-d`), `--no-debug` |
| `simd` | `--simd scalarize`, `--simd require` |
| `pass`, `skip` | `--pass NAME` adds a pass, `--skip NAME` leaves one out |
| `relax` | `--relax NAME` permits, `--no-relax NAME` withdraws |

The levers apply in one order: the built-in defaults, then the selected profile,
then the command line. Both flags of a pair, or `--pass` and `--skip` of the
same pass, are refused. The previous major's `-g`, `-O0` and `-O2` are refused
naming `-d`, `--no-optimize` and `-O`.

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

## `[artifact.<name>]`

Every artifact is declared explicitly and named by its table key. `$bin.name`
reads the selected artifact's name.

| Key       | Required | Meaning |
|-----------|----------|---------|
| `kind`    | yes | `"bin"`, `"static"`, or `"shared"` (see below). |
| `entry`   | yes | Entry source, relative to the project `src` dir (e.g. `main.mach` for `src/main.mach`). The entry module's FQN is `<id>.<entry without .mach>`, `/` turned into `.`. |
| `out`     | yes | This artifact's output path, relative to the project root like every other path. Write `{project.work}/bin/demo` to place it under the build output, or any other project path to place it there. Use `{artifact.suffix}` for the target extension, or write a literal filename. See [Artifact filenames and identity](#artifact-filenames-and-identity). |
| `targets` | yes | Array of declared target names this artifact builds for; `["*"]` means every declared target. |
| `link`    | absent: links nothing | Array of `[link.X]` names this artifact links (see below). A name with no table is a manifest error naming the artifact and the declared tables (`[artifact.p1].link names no [link.*] table: 'nosuch' (declared: [link.kernel32])`). |
| `need`    | absent: needs nothing | Array of category-qualified requirements such as `step.generate`, `artifact.support`, and `artifact.shader-*`. Each glob matches only its named category. See [Artifact requirements](#artifact-requirements). |
| `subsystem` | absent: `"console"` | `"console"` or `"gui"` — the environment a windows executable declares it runs under; refused on a target whose image format has no subsystem (see below). |
| `icon` | absent: no icon | Project-root-relative `.ico` path embedded in a Windows executable's PE resources. Non-empty path string; `bin` artifacts only. |
| `manifest` | absent: no application manifest | Project-root-relative application-manifest path embedded byte-for-byte in a Windows executable's PE resources. Non-empty path string; `bin` artifacts only. |
| `default` | absent: not marked | Selection only. `true` puts the artifact in the [default selection](build.md#selection-and-the-build-matrix): with no `-a`, `mach build` and `mach check` take the marked artifacts among those supporting the selected target (every one of them when none is marked), and a command that needs one artifact (`mach test`, `mach run`, the editor's union build) takes the marked one. A command that needs one artifact refuses two marked candidates; an explicit `-a` always wins, and a sole candidate needs no marker. |
| `export` | absent: not exported | `true` marks the library a bare `use <id>;` binds and whose requirements travel to consumers (see [Dependency requirements travel](#dependency-requirements-travel)). It applies to `static` and `shared` artifacts only, and a project exports at most one. |

`entry` is the build cell's source root. The build follows its active `use` and
`fwd` edges transitively and compiles that reachable module set; another file under
`src` is not part of the cell merely because it shares the project directory. This
is what lets one project declare host and accelerator artifacts with disjoint target
sets. `mach build`, `mach check` and `mach test` all operate on the selected
artifact's closure: a test build compiles and tests exactly the modules the artifact
under test reaches, so a module no selected artifact reaches is not loaded under
any of them (see [test.md](test.md#which-tests-run)).

- **`bin`** links an executable at the resolved `out` path. On a finished-module
  target such as `spirv` it is the entry module, written there unlinked.
- **`static`** materialises a real `ar` archive at the resolved `out` path — the
  per-module objects with an archive symbol index, the deliverable a consumer links
  as a `.a`.
- **`shared`** links a dynamic library at the resolved `out`. Only ELF targets
  write one today: `linux` on `x86_64`, `aarch64` and `riscv64` produce a `.so`
  whose `SONAME` is its file name. mach writes no Mach-O `.dylib` or PE `.dll`,
  so a `darwin` or `windows` target refuses with `link: object format cannot
  write shared libraries`. A `freestanding` target never
  writes one: its default `raw` format refuses with `a flat-image object format
  produces only executables`, and setting `of = "elf"` moves the refusal to the
  link, `link: a shared library needs a loader to map it, and os =
  "freestanding" has none`, because a shared library only exists to be mapped by
  a loader the os provides.
  - **Exports.** The library exports the root project's `pub` functions and
    variables and every name its modules re-export with `fwd`, including a
    dependency's. A dependency's own `pub` surface is not exported unless it is
    re-exported. `#[symbol("name")]` sets the name an export carries and does not
    make anything visible: a `pub` function exports under its `#[symbol]` name,
    and a non-`pub` one stays hidden whatever its name.
  - **Internals.** Every other definition still links inside the library but is
    absent from `.dynsym`. In the `.so` it is a `LOCAL` symbol in `.symtab`, and
    in the per-module object it is a `GLOBAL` symbol whose visibility the
    format spells its own way: ELF `STV_HIDDEN`, Mach-O `N_PEXT`, and COFF, which
    has no visibility bit, a `.drectve` section listing every exported definition
    as ` /EXPORT:<name>`, so a global definition the directives do not name is
    hidden. A COFF object with no `.drectve` says nothing about visibility and
    is read as it was. A `fwd` re-export of a dependency's symbol is a request
    the object carries separately: on COFF it is one more `/EXPORT:` token, and
    on ELF and Mach-O it rides in a non-loaded mach section (`.mach.exports`, or
    `__MACH,__mach_exports`) the parser consumes. A relocatable object emitted
    and parsed back therefore keeps the same visibility and the same requests.
  - **Refusals.** A shared artifact that exports nothing is refused:

    ```
    link: shared library '<artifact>' exports nothing: a shared library needs at least one `pub` declaration in the project, or a `fwd` re-export of one
    ```

    A `freestanding` target is refused as well. With its default `raw` format
    the artifact fails naming (`artifact naming: this object format has no
    shared-library form`), and with `of = "elf"` the link refuses with
    `link: a shared library needs a loader to map it, and os = "freestanding"
    has none`.

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
| SPIR-V module | `.spv` | unsupported | unsupported |

The selected object format supplies the naming rules, including explicit target
format overrides. Unsupported library forms are errors. A SPIR-V `bin` artifact is
its entry module, delivered at `out` with the per-module objects still written under
`obj/` (see [Finished-module targets](#finished-module-targets)).

`{artifact.suffix}` is available only in an artifact output template. It does not
expand in project output roots, link paths, step arguments or source embeds.
Output collisions are checked after expansion among artifacts selected for the
target. An explicit literal such as `bin/app.exe` can therefore collide with
`bin/app{artifact.suffix}` on Windows.

### `subsystem` — the windows console/GUI selector

```toml
[artifact.game]
kind = "bin"
entry = "main.mach"
out = "{project.work}/bin/game.exe"
targets = ["*"]
link = []
need = []
subsystem = "gui"
```

A PE executable records in its optional header which environment it wants, and the
Windows loader honours it: `"console"` gets a console window attached to the
process, `"gui"` does not. mach defaults to `"console"`, which is what every PE it
has ever emitted declares, so an artifact that omits the key is byte-identical to
one built before the key existed. A graphical application sets `"gui"` to stop an
empty console from opening behind it on launch.

Only a PE image carries the field. A key written on an artifact that builds
for a target whose format has none (ELF, Mach-O, a flat image) is refused as
unsupported, naming the key, the target and the format:

```
mach.toml: artifact.game.subsystem: Subsystem gui is unsupported by elf (target 'host' produces a elf image, which declares no subsystem)
```

An omitted key is the console default and is never refused, so an artifact
that declares no subsystem still builds everywhere. An artifact that needs the
key and also targets a non-windows cell declares two artifacts, one per format,
the way `[link.X]` entries carry `os`/`isa`/`abi` axes: the manifest never
carries a declaration a build silently ignores.

`--subsystem console|gui` overrides the key for one invocation and is refused
the same way on a target whose format has no subsystem (`mach help build`).

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
| `os`      | yes | Filter axis: an array of canonical `os` values or `"*"` (any); `[]` matches none. |
| `isa`     | yes | Filter axis over `isa`, same forms. |
| `abi`     | yes | Filter axis over `abi`, same forms. |
| `export`  | absent: not exported | `true` cascades this entry to consumers; `false` keeps it to this project's own builds. |
| `include` | absent: `"always"` | `"always"` names the dynamic library in the linked image whether or not anything imports from it; `"referenced"` names it only when a live import references it, so an unused provider leaves no load command behind. Any other value is a manifest error (`[link.k].include must be "always" or "referenced"`). |
| `library` | removed | Refused as a removed key. The table key is the library's identity, and `#[library]` names the entry by that key. |

The `os`/`isa`/`abi` axes select the build cells an entry applies to. Each is an
array, like an artifact's `targets`: canonical values, `"*"` for any, and `[]`
matching nothing (an entry deliberately switched off). A bare string is refused. The three axes are required: an axis left out would mean every
target, which is an assumption to state, so a `kernel32` entry with no `os` is
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
os      = ["windows"]
isa     = ["*"]
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

Nothing reads a library's export table to derive this, so the claim is what makes
cross-linking a PE from a Linux host work with no target DLL present. Claims
travel with the entry, so `export = true` cascades them to consumers and a
C-binding project declares them once.

A symbol may be claimed only once per link: two selected entries claiming it, or a
claim contradicting a `#[library]` decorator, is an error naming both claimants
rather than an order-dependent win — repeating the *same* claim is fine. Listing a
symbol twice within one entry is rejected, and so is a claim on an entry that
resolves to a **static** input, which defines symbols rather than importing them.
On ELF the key is accepted and validated but changes no emitted bytes, since that
loader resolves imports by global search.

Whether an input links **statically** or **dynamically** follows the resolved file
— a loose `.o`/`.obj` or static `.a`/`.lib` links statically; ELF `.so`, Mach-O `.dylib`,
and PE `.dll` inputs are recorded using their format's canonical loader name.
An `@rpath/` Mach-O install name also retains the directory where resolution found
the dylib, which the executable records as `LC_RPATH`. Darwin frameworks use a
version-independent system framework path. See
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
arguments, and effective environment contribute to its fingerprint. An unchanged
step whose outputs still exist is skipped. Changing an inherited environment
value received by the child also invalidates the step.

The fingerprint of the last successful run is kept as a stamp in
`{project.work}/.cache/steps/`, and a step's outputs under `{project.work}` are
written into scratch space in `{project.work}/.stage/<name>/` and published only
once the step succeeds. Both belong
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
binary a cold one links, byte for byte. An object that is missing, has no
key, a damaged one or another key is rebuilt, never linked stale. `obj/` holds one
object per module, the latest, and each object is written to a sibling temporary
and renamed into place, so an interrupted build leaves the previous object or the
new one, never a torn file. The digests of the sources the keys read are
remembered in `{project.work}/.cache/digests` under each file's path, size,
modification time and identity, so an unchanged file is not hashed again for its
key; a missing or damaged memo is rebuilt. `--no-cache` forces an uncached
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
build cell's target tuple as `MACH_TARGET_ISA`, `MACH_TARGET_OS`, and
`MACH_TARGET_ABI`, so the script `argv` invokes can branch on the target
without threading it through the template — e.g. `cc --target=$MACH_TARGET_ISA-…`.
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

Paths and step `argv` entries expand over a closed, final set of eight variables:

- `{project.work}` — the **root** project's expanded `[project].work`, in every
  manifest of the closure. In a dependency's artifact `out` it is that
  dependency's home under it, `dep/<id>`.
- `{target.name}` — the resolved target name (never the literal `native`).
- `{target.isa}` — the resolved target's `isa` (e.g. `x86_64`).
- `{target.os}` — the resolved target's `os` (e.g. `linux`).
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

The three `{target.*}` tuple keys are also exported to every step process as
`MACH_TARGET_ISA`/`MACH_TARGET_OS`/`MACH_TARGET_ABI` (see [build steps](#stepname--build-steps)).

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
artifact's own `out`. In a step's `argv` and `env` values a brace group outside
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
kind    = "bin"
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
kind    = "bin"
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
are gated by their link entries' `os` axis, so the x11 step runs on a linux build
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
os     = ["*"]
isa    = ["*"]
abi    = ["*"]
export = false

[link.shim-x11]
source = "local"
path   = "{project.work}/obj/platform/x11.o"
os     = ["linux"]
isa    = ["*"]
abi    = ["*"]
export = false

[link.shim-win32]
source = "local"
path   = "{project.work}/obj/platform/win32.o"
os     = ["windows"]
isa    = ["*"]
abi    = ["*"]
export = false

[link.gl]
source = "system"
name   = "GL"
os     = ["linux"]
isa    = ["*"]
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
isa = "x86_64"
os  = "linux"
abi = "sysv64"

[target.windows]
isa = "x86_64"
os  = "windows"
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

The `gl` and `shim-x11` entries carry `os = "linux"`, so on a windows build cell
they filter out (and `shim-x11`'s step is never demanded); `shim-win32` carries
`os = "windows"` and applies only there. The unconditional `shim` entry (`os = "*"`)
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
os     = ["linux", "darwin"]
isa    = ["*"]
abi    = ["*"]
export = true

[link.glfw-win]
source = "system"
name   = "glfw3.dll"
os     = ["windows"]
isa    = ["*"]
abi    = ["*"]
export = true

[link.Cocoa]
source = "framework"
name   = "Cocoa"
os     = ["darwin"]
isa    = ["*"]
abi    = ["*"]
export = true
```

The binding module selects the entry's key once with `$if`, so every
declaration carries the same attribution on every target:

```mach
use std.types.string.str;

$if ($mach.build.os == $mach.os.windows) {
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
os     = ["*"]
isa    = ["*"]
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
