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
error: no mach.toml in the project directory
```

Nothing is inferred from the directory layout. `mach init` writes a complete
manifest so a new project never starts from that error (`mach help init`).

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

A dependency's `mach.toml` is read by the same closed schema, once, and an
unknown or removed key in it fails the consumer's build naming the dependency:

```
error: dep 'std': mach.toml: unknown key 'bogus' in [project]
```

What a consumer *uses* from a dependency's manifest is its export surface: the
project id, the module a bare `use <id>;` binds (see
[modules.md](modules.md#bare-project-id-imports)), its
`export = true` link entries, the steps those entries demand, and what its
`default = true` library artifact requires — the artifacts and steps named in
that artifact's `need` (see
[Dependency requirements travel](#dependency-requirements-travel)). Nothing else
travels: a `bin` artifact's `need`, a non-default library's, and every other
requirement of the dependency stay its own.

A dependency's `[profile.*]` tables are never read to build the consumer, which
resolves its own profile and builds everything with it. A dependency's
`[target.*]` tables are read for exactly one purpose: the targets its travelling
requirements name, `env` included, since those artifacts are built for the
targets the dependency declares for them. No other `[target.*]` entry is read.

## The schema at a glance

```toml
[project]
id      = "demo"                       # required: identifier; root of every module path
version = "0.1.0"                      # required
mach    = "^5.3"                       # required in a root manifest: the compiler range
src     = "src"                        # required: source dir, project-root-relative
out     = "out/{target.name}/{profile.name}"  # required: output-path template root

[target.linux]                         # a platform: a fully-spelled tuple
isa = "x86_64"
os  = "linux"
abi = "sysv64"

[profile.debug]                        # a build variant; at least one is required
opt   = 0                              # 0 (debug pipeline) | 1 | 2 (release pipeline)
debug = true                           # emit debug info for this profile
simd  = "scalarize"                    # SIMD lever: "scalarize" | "require"
vectorize = false                      # auto-vectorization lever
float_reassoc = false                  # float reassociation permission

[artifact.demo]                        # a produced artifact
kind    = "bin"                        # "bin" | "static" | "shared"
entry   = "main.mach"                  # entry source, relative to src
out     = "bin/demo{artifact.suffix}"  # output path, relative to the project out
targets = ["*"]                        # which declared targets build it ("*" = all)
link    = []                           # [link.X] names this artifact links
need    = []                           # step.X / artifact.X requirements
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
| `out`     | string | The output-path template root, referenced as `{project.out}` by artifact `out`, step paths, and `cmd`s. Expanded over `{target.name}`/`{target.isa}`/`{target.os}`/`{target.abi}`/`{profile.name}` (see [Path templates](#path-templates)). |
| `mach`    | string | The compiler versions this project builds with, as a [version range](#version-ranges) (`"^5.3"`). Required in a root manifest. See [Compiler range](#compiler-range). |

`[project]` is exactly these five keys. Any other key, `name` and `description`
included, is an unknown-key error (`mach.toml: unknown key 'name' in
[project]`), in a root manifest and a dependency's alike. `[profile.<name>]`
likewise carries no `emit_ir` or `emit_asm`: emission is `--emit-ir`/`--emit-asm`
on the command line.

### The output directory

Everything a build and its cache write lands under the expanded `out`, in one
layout:

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
`obj/<project>/<module>.test.o`. Artifact outputs go wherever their own `out`
names under the directory.

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
requirement and the chain that states it:

```
error: this is mach 5.2.1, and the dependency closure does not accept it:
    app (mach.toml) requires mach ^5.3
    app -> gfx -> glfw requires mach >=5.4, <6
```

A root manifest must state `mach`. One without it is refused with the line to
add (`mach.toml: [project] states no compiler range; add mach = "^5.3", the
oldest release that reads the key, and raise it when the project uses a later
feature`). A dependency without it states no constraint. `mach init` writes the same range. It is the oldest
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

Each `<name>` is a selector you pass to `--target <name>`. A target is a
fully-spelled platform tuple; nothing is inferred from another key. `native` is a
reserved name — declaring `[target.native]` is an error, because `native` resolves
to whichever *declared* target matches the host.

| Key   | Required | Meaning |
|-------|----------|---------|
| `isa` | yes      | Instruction-set architecture. Read by `$project.target.arch`. |
| `os`  | yes      | Operating system. Read by `$project.target.os`. |
| `abi` | yes      | Application binary interface. Read by `$project.target.abi`. |
| `of`  | no       | Object-format override; defers to the os's format when omitted. See [Object-format override](#object-format-override). |
| `base` | no      | Load-address override (integer). Overrides the os's default base virtual address; defers to it (`0` for `freestanding`) when omitted. |
| `platform` | no  | Open platform tag (string), surfaced to comptime as `$mach.build.platform` (empty when unset). A support library keys its backend on it; the compiler treats it as opaque. See [Platform targets](#platform-targets-bare-metal). |
| `stack_reserve` | no | Thread stack reserve in bytes. See [Image stack size](#image-stack-size). |
| `stack_commit` | no | Thread stack commit in bytes. See [Image stack size](#image-stack-size). |
| `default` | no | `true` marks the target `native` resolves to when no declared target matches the host and several are declared. Exactly one may carry it: two are refused at parse (`2 targets declare `default = true` ([target.linux-x86_64], [target.darwin-x86_64]); exactly one is allowed`). See [`native` target resolution](#native-target-resolution). |
| `extensions` | no | Array of instruction-set extension names the target may assume, such as `["sha", "ssse3"]`. Each name must be in the isa's vocabulary. See [Instruction-set extensions](#instruction-set-extensions). |
| `env` | no | Consumer environment (string). The values are owned by the target's isa: an `env` the isa does not define is a manifest error naming the target and the known values, and an isa that defines none refuses the key outright. Today only `spirv` defines any; see [Finished-module targets](#finished-module-targets). |

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
| `aarch64` | AdvSIMD | `sha2`, `sb`, `aes`, `pmull` |
| `riscv64`, `riscv32` | the isa string's selection | `i`, `m`, `a`, `f`, `d`, `c`, `zicsr`, `zifencei`, `zkt` |
| `spirv` | | none |

A name the selected isa does not hold is refused when the target resolves, with the
names it does hold:

```
error: target: `sha2` is not an extension or level of isa 'x86_64'; its extensions are:
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
`f` and `f` brings `zicsr`, as the isa string's own grammar has it, so `extensions = ["d"]` on `rv64i` selects
`rv64ifd` with Zicsr. The isa string and the list feed one set: `isa = "rv64i"` with
`extensions = ["m"]` selects the same machine as `isa = "rv64im"`. A name nothing in
the compiler encodes against yet (`avx2`, `avx512f`) is still a declared requirement:
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
refusal says why. Every x86_64 and aarch64 row, and riscv `m`, `a`, `zicsr` and
`zifencei`, may be.

Selecting an extension is a promise about **every** machine the binary runs on. The
inline assembler admits the extension's mnemonics anywhere in the build, and a host
without the extension faults on the first one it executes. A portable binary keeps the
target at its baseline instead. It confines the extension instructions to
[`#[extensions(...)]`](decorators.md#extensionsnames--an-outlier-function) functions
and picks one of those at run time, after detecting the host's features.

A mnemonic that needs an extension the target does not select, outside such a
function, is refused. The refusal names the line to add:

```
error: encode: inline-asm instruction 'sha256rnds2' needs the `sha` extension, which
this target does not select; add `extensions = ["sha"]` to the target, or mark the
function `#[extensions(sha)]` and call it only after detecting the extension at run time
```

The selected set is part of the target's identity: two targets that differ only in
`extensions` never share cached products.

The selection also reaches code generation. Every vector operation is legal on every
target and its shape never depends on the extension list; what moves is the lowering.
A cell the baseline expands to a scalar sequence lowers to the one packed instruction
when the selected set holds the extension that carries it (`i32x4 * i32x4` is
`pmulld` under `sse41` on x86_64 and a scalar expansion without), and
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
error: mach.toml: [target.lin].stack_reserve is not expressible on the `elf` object
format, which carries no stack size in its image headers
```

Every declared target is checked, not only the one being built, so the mistake is found
on the first build rather than whenever someone happens to build that cell.

A function whose own stack frame exceeds the reserve is refused at build time, naming
the function, its frame size and the reserve:

```
error: frame: `main` needs a 1107824-byte stack frame, which its target's
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
| `isa` | `x86_64`, `aarch64`, `riscv64`, `riscv32`, a canonical RISC-V extension string such as `rv32imc`, `spirv` |
| `os`  | `linux`, `windows`, `darwin`, `freestanding` |
| `abi` | `sysv64`, `win64`, `aapcs64`, `lp64`, `lp64f`, `lp64d`, `ilp32`, `ilp32f`, `ilp32d`, `spirv` |

A `[target.*]` naming an `isa`, `os` or `abi` outside these lists is refused
through the registry's own lookup (`no isa implementation registered for
'mos6502' (registered: ...)`).

`x86_64`/`linux`/`sysv64` is the primary host and target. `aarch64`-linux builds
and runs natively in CI on every PR; `riscv64`-linux runs under qemu and
self-hosts (#1852). `windows` is a supported cross-compilation target (PE/COFF,
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
The retained vocabulary is I, M, A, F, D, C, Zicsr, Zifencei and Zkt, written in
lowercase canonical order with multi-letter names after an underscore; `g`
expands to IMAFD plus Zicsr and Zifencei. F carries its required Zicsr, and D
requires F. Zkt changes no instruction. It states that the listed operations run
in data-independent time, which is what lets a secret multiply compile (see
`secrecy.md`). An optional version must be the one mach models: I 2.1, M 2.0,
A 2.1, F and D 2.2, C 2.0, Zicsr and Zifencei 2.0, Zkt 1.0. Unknown extensions,
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

A value outside its axis's set is a strict-parse error, so a typo is caught rather
than silently never matching.

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
environment. Without `env` a module is written as SPIR-V 1.6 with no ceiling.

| `env` | SPIR-V | capabilities within the ceiling |
|---|---|---|
| `vulkan1.0` | 1.0 | Int16, Int64, Float64, Sampled1D, SampledCubeArray |
| `vulkan1.1` | 1.3 | same as `vulkan1.0` |
| `vulkan1.2` | 1.5 | the above plus Int8, Float16 |
| `vulkan1.3` | 1.6 | same as `vulkan1.2` |

The ceiling is what a conforming implementation of that Vulkan version can
enable through core device features alone, with no extension: from the Vulkan
specification's "Vulkan Environment for SPIR-V" appendix, the capabilities table
maps `Int64`, `Int16`, `Float64` and `SampledCubeArray` to the `shaderInt64`,
`shaderInt16`, `shaderFloat64` and `imageCubeArray` features of Vulkan 1.0,
`Sampled1D` to core, and `Int8` and `Float16` to `shaderInt8` and
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
rest of the image (#2402) — an image costs its bss size in file bytes, and nothing
has to zero anything at startup.

## `[profile.<name>]`

A profile is one explicit compilation policy: a build variant. The optimization
level, the debug-emission toggle and the three SIMD levers live here because
they are variant concerns, and every one of them is stated. A root manifest
declares at least one profile; nothing is synthesized. Values that are
*derived* rather than declared live elsewhere: a target's object format and
naming come from `[target.*]` facts, and an absent optional feature such as a
`[link.*]` filter axis is spelled `"*"` where it applies, not defaulted here.

| Key     | Type    | Meaning |
|---------|---------|---------|
| `opt`   | integer | Optimization level: `0` selects the debug pipeline (the always-on passes only), `1` and `2` select the release pipeline. `1` and `2` currently share a pass set, which includes loop auto-vectorization (see `vectorize` below). Any other integer — or a non-integer — is a manifest error. |
| `debug` | bool    | Emit debug info for this profile: DWARF in ELF, Mach-O and COFF objects alike, and the core SPIR-V debug instructions on a `spirv` target (see [Finished-module targets](#finished-module-targets)). A PE image carries its DWARF in `.debug_*` sections, which gdb, lldb and the LLVM tools read and Visual Studio and WinDbg do not. Gates emission only, never the optimizer, so a `release` profile can keep symbols with `debug = true`. A non-boolean is a manifest error. |
| `simd`  | string  | SIMD scalarization lever. `"scalarize"` emits a defined unrolled scalar expansion wherever the target has no packed instruction for a vector operator, with a build-time note. `"require"` makes that a hard error naming the operation, its **lane width**, the function and the target. It applies **per operation on every target**, not only to targets with no vector unit: x86-64's SSE2 baseline has no 32-bit lane integer multiply and NEON has no 64-bit one, so a capable target scalarizes too. Any other string is a manifest error. |
| `vectorize` | bool | Auto-vectorization lever. When `true`, the release pipeline rewrites provably-safe counted loops to 128-bit SIMD on a target with hardware vectors; `false` skips the pass, so release output stays scalar. A non-boolean is a manifest error. |
| `float_reassoc` | bool | Permission to treat floating-point addition and multiplication as **associative**. It lets the vectorizer reduce an `f32`/`f64` accumulator through lane-count partial sums, which changes the result — see [Float reassociation](#float-reassociation) for what that costs and what it buys. A non-boolean is a manifest error. |
| `default` | bool | **Optional.** `true` marks the profile a build uses when several are declared and `--profile` is absent. Exactly one profile may carry it. See [Profile requirement and selection](#profile-requirement-and-selection). |

Five keys (`opt`, `debug`, `simd`, `vectorize`, `float_reassoc`) are required
in a declared profile, in a root and in a dependency manifest alike; only
`default` is optional. A missing key is a manifest error naming the table and
the key:

```
error: mach.toml: [profile.debug] is missing required key 'vectorize'; a profile declares opt, debug, simd, vectorize and float_reassoc
```

### Profile requirement and selection

A root manifest declares at least one `[profile.*]` table. A root that
declares none does not build:

```
error: mach.toml: no [profile.<name>] table is declared; a build needs an explicit profile declaring opt, debug, simd, vectorize and float_reassoc
```

`mach init` writes `debug` (`opt = 0`, `debug = true`, `default = true`) and
`release` (`opt = 2`, `vectorize = true`) in full, so a scaffold never starts
from that error. A dependency manifest that declares no profile is still read
(its profiles are never used to build the consumer, see
[Root vs. dependency strictness](#root-vs-dependency-strictness)); it gets the
two built-in profiles `debug` and `release` for its own `{profile.name}`
templates. That synthesis is a dependency-only convenience and applies to no
root.

Which profile a build uses follows one rule, the same one that selects a
target and an artifact:

1. an explicit `--profile <name>` wins;
2. otherwise a sole declared profile is chosen;
3. otherwise the one marked `default = true` is chosen.

Table order carries no meaning. A manifest that declares several profiles and
marks none is refused wherever a command must pick one:

```
error: mach.toml: several profiles are declared and none is marked `default = true`; no profile is selected by table order: mark exactly one [profile.<name>] with `default = true` or select one with --profile
```
Emission of the human-readable IR and assembly side-artifacts is **not** a profile
concern — it is controlled only by the `--emit-ir` / `--emit-asm` flags of
`mach build`.

The `vectorize` lever only ever *subtracts*. The pass it gates runs in the release
pipeline on targets that report 128-bit vector support (SSE2 on x86-64, NEON on
aarch64, and `OpTypeVector` on spirv) and rewrites counted, unit-stride loops whose dependence analysis proves
independence — element-wise maps behind a runtime alias guard, and associative-exact
integer reductions. A loop it cannot prove safe stays scalar, and a target without
hardware vectors (riscv64) never enters the pass, so `vectorize = false` changes
performance and never semantics. For a single function, the `#[scalar]` decorator is
the finer-grained opt-out (see [decorators.md](decorators.md)).

`float_reassoc` is the one lever here that *adds*, and the only profile key that can
change a program's computed answer. It widens that same pass to float reductions and
does nothing else; `vectorize = false` switches the pass off wholesale and so overrides
it.

The `simd`, `vectorize` and `float_reassoc` levers are always the **consumer's**.
Consistent with the root-vs-dependency strictness above, a dependency's `[profile.*]`
is parsed by the same schema and never read to build the consumer, so a library's
values are inert — the effective levers come from the consumer's resolved profile.
Libraries set nothing SIMD-specific and inherit the consumer's choice; there is no
ecosystem fork and no dual API.

### Float reassociation

`float_reassoc = true` grants the optimizer exactly one liberty: it may treat
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
reference implementation — must leave the key off.

Two exactness properties are preserved rather than traded away. The idle lanes are
seeded with the op's **exact** IEEE identity — `-0.0` for addition (`x + (-0.0)` is `x`
for every `x`, where `+0.0` would turn a negative-zero sum positive) and `1.0` for
multiplication — so no signed-zero or NaN behaviour changes. And a trip count below the
lane count never enters the vector loop at all, so short reductions are bit-identical
regardless of the key.

The key is profile-wide. For a single function, `#[scalar]` opts out of vectorization
entirely and takes precedence over it, so a routine that must stay IEEE-strict inside
an otherwise-reassociating build has a spelling. There is no per-function opt-*in*:
whether a reduction may be reassociated is the caller's tolerance to decide, not the
callee author's.

Integer reductions are untouched by this key. They vectorize unconditionally and are
bit-identical to the scalar reference, because integer add / xor / or / and reassociate
exactly.

The CLI selects and overrides at invocation time: `--profile <name>` picks the
profile; `-g` forces `debug` on for one build regardless of the profile's key
(precedence `-g` > profile > off — there is no flag to force it off over a
`debug = true` profile; edit the manifest or pick another profile). `-O0` and
`-O2` override the profile's `opt` the same way; `-O1` is rejected (`-O1 was
removed; use -O0 or -O2`).

## `[artifact.<name>]`

Every artifact is declared explicitly and named by its table key. `$bin.name`
reads the selected artifact's name.

| Key       | Required | Meaning |
|-----------|----------|---------|
| `kind`    | yes | `"bin"`, `"static"`, or `"shared"` (see below). |
| `entry`   | yes | Entry source, relative to the project `src` dir (e.g. `main.mach` for `src/main.mach`). The entry module's FQN is `<id>.<entry without .mach>`, `/` turned into `.`. |
| `out`     | yes | This artifact's output path, **relative to the expanded project `out`** and rooted there automatically — write `bin/demo`, not `{project.out}/bin/demo`. Use `{artifact.suffix}` for the target extension, or write a literal filename. See [Artifact filenames and identity](#artifact-filenames-and-identity). |
| `targets` | yes | Array of declared target names this artifact builds for; `["*"]` means every declared target. |
| `link`    | yes | Array of `[link.X]` names this artifact links (see below). `[]` for none. A name with no table is a manifest error naming the artifact and the declared tables (`[artifact.p1].link names no [link.*] table: 'nosuch' (declared: [link.kernel32])`). |
| `need`    | yes | Array of category-qualified requirements such as `step.generate`, `artifact.support`, and `artifact.shader-*`. Each glob matches only its named category. `[]` for none. See [Artifact requirements](#artifact-requirements). |
| `subsystem` | no | `"console"` (default) or `"gui"` — the environment a windows executable declares it runs under; refused on a target whose image format has no subsystem (see below). |
| `icon` | no | Project-root-relative `.ico` path embedded in a Windows executable's PE resources. Non-empty path string; `bin` artifacts only. |
| `manifest` | no | Project-root-relative application-manifest path embedded byte-for-byte in a Windows executable's PE resources. Non-empty path string; `bin` artifacts only. |
| `default` | no | `true` puts the artifact in the [default selection](#selection-and-the-build-matrix): with no `--bin`/`--lib`, `mach build` and `mach check` take the marked artifacts among those supporting the selected target (every one of them when none is marked), and a command that needs one artifact (`mach test`, `mach run`, the editor's union build) takes the marked one. A command that needs one artifact refuses two marked candidates; an explicit `--bin`/`--lib` always wins, and a sole candidate needs no marker. |

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
  as a `.a` (#1997).
- **`shared`** links a dynamic library at the resolved `out`. Only ELF targets
  write one today: `linux` on `x86_64`, `aarch64` and `riscv64` produce a `.so`
  whose `SONAME` is its file name. The Mach-O `.dylib` and PE `.dll` writers are
  not built yet, so a `darwin` or `windows` target refuses with `link: object
  format cannot write shared libraries` (#3588). A `freestanding` target never
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
out = "bin/app{artifact.suffix}"
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
| Mach-O on Darwin | empty | `.a` | `.dylib` (not written yet, #3588) |
| COFF/PE on Windows | `.exe` | `.lib` | `.dll` (not written yet, #3588) |
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
out = "bin/game.exe"
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
out = "bin/game.exe"
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
| `library` | no | Stable logical name used by `#[library("...")]`; defaults to the `[link.<name>]` table name. |
| `symbols` | no | Array of symbol names this dependency provides, attributing imports that have no `ext` declaration to decorate (see below). Written as **source-level** names; the target's C symbol prefix is applied by Mach. Omit for none. |
| `os`      | yes | Filter axis: a canonical `os` value, `"*"` (any), an array of values, or `[]` (none). |
| `isa`     | yes | Filter axis over `isa`, same forms. |
| `abi`     | yes | Filter axis over `abi`, same forms. |
| `export`  | yes | `true` cascades this entry to consumers; `false` keeps it to this project's own builds. |
| `include` | no | `"always"` (the default) names the dynamic library in the linked image whether or not anything imports from it; `"referenced"` names it only when a live import references it, so an unused provider leaves no load command behind. Any other value is a manifest error (`[link.k].include must be "always" or "referenced"`). |

The `os`/`isa`/`abi` axes select the build cells an entry applies to. Each takes a
single canonical value, `"*"` for any, or an array — `os = "linux"` and
`os = ["linux"]` filter identically. `[]` matches nothing (an entry deliberately
switched off). A non-canonical spelling is a strict-parse error. An entry applies
to a cell when all three axes match.

A `local` entry's `path` must, at build time, either match a `[step.X]`'s `out`
(which demands that step) or already exist on disk — anything else is an up-front
error, so a typo never silently drops an input. A `local` path naming a shared
library is validated for the selected target before it is recorded: an ELF
`.so` that is not a loadable shared object for the target's architecture (a
linker script, a foreign-architecture file) is refused (`'<file>' is not a
loadable ELF shared object for the selected architecture`).

`library` decouples source attribution from platform loader spelling. Give
mutually exclusive platform entries the same logical value when they provide the
same API; one unconditional `#[library("glfw")]` can then bind against
`libglfw.so.3` on Linux, an `LC_ID_DYLIB` install name on Darwin, and
`glfw3.dll` on Windows. Exact canonical loader names remain accepted for
compatibility. Selecting two dependencies that map the same logical name to
different loader names in one build is an error. A logical name that equals a
different dependency's canonical loader name is likewise rejected, so
attribution never depends on requirement order.

A `#[library]` resolves against the **effective** link set: the artifact's own
referenced entries, plus every entry a dependency exports. A binding project
therefore names its libraries once and a consumer writing its own `ext fun`
against them adds nothing but a `dep` entry.

A logical name may belong to an entry that resolves to a **static** input, and
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
library = "kernel32"
symbols = ["Sleep", "CreateFileW", "CloseHandle"]
os      = "windows"
isa     = "*"
abi     = "*"
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
| `env`  | no  | Table of string values added to the step process's environment. |
| `in`   | yes | Declared input file list. Accepts globs (`*`, `**`), expanded sorted for a stable fingerprint; a glob that matches nothing is a hard error. |
| `out`  | yes | Declared output file list. Concrete paths only — a glob here is an error, since the demand match and cache key expand `out` verbatim. |
| `need` | yes | Array of `step.<name>` requirements or `step.<pattern>` globs this step must run after. Steps may require only steps. Cycles are manifest errors. `[]` for none. |
| `timeout` | no | Duration string (`"30ms"`, `"30s"`, `"5m"`, `"1h"`) after which the step's process group is terminated and the build fails. Omit for an unbounded step. |

Steps carry **no filters** and **never run automatically**. A step runs only when
**demanded**:

- by a selected `[link.X]` whose `local` `path` matches the step's `out`;
- by another step's `need`;
- by an artifact's `need` (for outputs that are not link inputs), by name or
  through a glob;
- in a dependency, by the `need` of its `default = true` library artifact, which
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
`{project.out}/.cache/steps/`, and a step's outputs under `{project.out}` are
written into scratch space in `{project.out}/.stage/<name>/` and published only
once the step succeeds. Both belong
to the compiler: a declared `out` inside `{project.out}/.cache/` or
`{project.out}/.stage/` fails at manifest load, naming the step and the path.
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

**Output homing.** `{project.out}` resolves to the **root** project's expanded
`out` in every manifest of the closure. A dependency's step outputs land in the
consumer's output tree — exactly as a dependency's compiled modules do — and the
dependency's own checkout is never written to. For an exported dependency step,
the command receives `{project.out}` as an absolute path rooted at the consumer;
it does not depend on the directory from which the consumer was invoked. An ordinary relative
consumer root (including `./` segments) and its normalized absolute spelling produce
the same expanded command and cache key.

**The module object tree is reserved.** `{project.out}/obj/<project.id>/` belongs
to the compiler: every module of the project compiles to one object in it, named
after the module's path (`src/window.mach` in project `glfw` becomes
`{project.out}/obj/glfw/window.o`). A step that writes there collides with those
objects by name, and because the link takes whichever file survived, the result is
a binary that is subtly wrong rather than a build that fails.

**The object tree is the object cache.** Under `--cache` a module whose object in
`obj/` was built under the same key is reused as it is: the module is neither
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
remembered in `{project.out}/.cache/digests` under each file's path, size,
modification time and identity, so an unchanged file is not hashed again for its
key; a missing or damaged memo is rebuilt. `--no-cache` ignores the tree, and
`mach clean` removes it with the rest of the output.

A step output is therefore rejected in that subtree. A declared `out` inside it
fails at manifest load, naming the step and the path, before any step runs. A step
that writes an object there without declaring it is caught after it runs, with the
same message — this covers the common case of a vendored `make` dropping every
object it built into the output directory.

Pick any other subtree of `{project.out}`. The conventional choice for a vendored
library is a directory named after the library rather than after the project, e.g.
`{project.out}/obj/miniaudio/` for a project whose own id is `audio`; note that
this only stays clear of the reserved tree while the two names differ, so prefer a
distinct sibling such as `{project.out}/vendor/<library>/`.

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

A stanza declares exactly one source:

| Key    | Meaning |
|--------|---------|
| `git`  | Git URL. The dependency is a git **submodule** at `dep/<id>/`, pinned by the gitlink the root repository commits. Requires `ref` or `version`. |
| `ref`  | Selector for `git`: `branch/<name>`, `tag/<name>`, or `commit/<full-object-id>`. Any other spelling is rejected (`[dep.std].ref must be branch/<name>, tag/<name>, or commit/<full-object-id>`). |
| `version` | A [version range](#version-ranges) over the dependency's releases (see [Releases and resolution](#releases-and-resolution)). Valid only with `git`; a path dependency has no releases. |
| `path` | Local project tree, never fetched. A relative `path` is resolved relative to this manifest's directory. `mach dep add <path> <id> --path` copies its files into `dep/<id>/` without the source's own `dep/` or Git metadata. No repository or index is required for a path dependency, and copied files are not automatically staged. Forbids `ref` and `version`. |

`git` and `path` are mutually exclusive and exactly one is required. A `git`
dependency also names exactly one selector, `ref` or `version`.

`ref` and `version` are mutually exclusive (`[dep.std] names both 'ref' and
'version'; keep one`). `ref` selects one exact commit or tag, or follows a
branch. `version` selects among releases.

### Releases and resolution

A **release** of a git dependency is a tag `vX.Y.Z` (optionally
`vX.Y.Z-pre`) together with the `mach.toml` at that tag. A tag whose manifest
does not load, as an old tag's written for an earlier manifest schema, or whose
`[project].version` differs from the tag name is not a candidate. Resolution
sets it aside and keeps looking. When nothing fits, the error lists it among the
requirements (`gl 0.1.0 is not a candidate: its mach.toml does not load: unknown
key 'name' in [project]`, or `... its [project].version does not match the
tag`), so the error never reads as one in the project's own `mach.toml`.

Resolution runs in exactly three places: `mach dep add`, `mach dep update` and
`mach dep outdated`. **Builds never resolve.** They verify, offline (see
[What a build verifies](#what-a-build-verifies)). For every identity in the
closure that some manifest selects by `version`, resolution picks one release
such that:

1. every requirer's range contains it;
2. its own `[project].mach` contains the running compiler;
3. the closure its own manifest implies also resolves.

A requirer is the root, a release resolution chose, or a dependency the root
reaches by `ref` or `path`. The last declares its range in the closure directly,
so two such dependencies naming one identity by range are two requirements of
the same problem, and the error names each by its chain (`root -> c requires b
<1.2`).

Among the choices that satisfy all three, it takes the highest release of each
identity. The result is written as gitlinks, like any other pin; there is
still no lock file. `mach dep update <path> <name>` keeps every other
identity at its pinned release (the release its recorded gitlink carries) while
that release still fits, so an update moves as little as it can. `--all`
resolves from scratch. A pin the range no longer admits, as after the range is
raised past it, is re-pinned to the release resolution picks; `update` never
keeps it. The recorded gitlink is the pin, not whatever the checkout holds: a
checkout that drifted from its gitlink is moved to the chosen release and the
gitlink staged in the same run, even when the drifted checkout already sits at
that release.

When nothing fits, the error lists every requirement that took part and names
the identity the root can settle:

```
error: no set of releases satisfies every requirement:
    root requires b ^1.2
    a 1.0.0 requires b ^2.0
  the root decides by declaring the identity itself, for example:
    [dep.b]
    version = "<a range the root can use>"
```

A release that needs a newer compiler appears as one of those lines (`b 2.0.0
requires mach ^6, and this is mach 5.2.1`). Resolution never silently settles
for a lower release than the ranges allow.

What `mach dep add` writes:

- with `--git <url>` alone, `version = "^X.Y.Z"`, where `X.Y.Z` is the
  release resolution picked. The lower bound is the release actually tested
  when the dependency was added, and the caret follows the pre-1.0 rule;
- with `--version <range>`, that range;
- with `--ref <selector>`, that selector, as before.

`mach init` adds std the same way, so a new project names the std release that
works with the compiler that created it. std is an ordinary dependency, with
no std-specific command. `mach init --no-deps` still resolves and writes the
range and skips only the checkout, so it needs the network too. Offline it
fails and writes no `[dep.std]` table. A tool that needs the std for a given
compiler runs `mach init` and `mach dep pull` in a scratch project and takes
what resolution chose.

**`--offline`.** `add`, `update` and `outdated` read candidates from each
dependency's repository: one `git ls-remote --tags` per URL, and the manifest
at a release through a shallow fetch of its tag. With `--offline` they use only
the tags already present in the realized checkouts, and they say so
(`resolving from releases already fetched (--offline)`). A resolution that
needs a candidate it doesn't have fails, naming the identity.

**`--lowest`.** `mach dep update <path> --all --lowest` picks the lowest
release every range accepts. A library's CI runs it in a scratch checkout and
then builds and tests, which proves the lower bounds it declares are honest.
Without that check, `^3.2.0` can quietly depend on something only 3.4 has. It
belongs in a release or manually dispatched job, never a scheduled one.

**`mach dep outdated <path>`** prints, for each version-selected identity, the
pinned release, the highest release resolution would pick now, and the highest
release published. A newer release held back by a range or by the compiler is
marked as such.

**No yanking.** A bad release that is otherwise compatible is fixed forward
with a new release. Nothing marks a published version as withdrawn. A
consumer that must avoid one raises its range's lower bound (`^3.2.1`).

**Forks.** Identity is the project id, not the URL. A root that declares a
fork's URL makes that fork the candidate source, so its `vX.Y.Z` tags compete
under the same ranges. A fork that wants to stay distinguishable tags
pre-releases (`v1.4.3-fork.1`), and a consumer opts in by naming the
pre-release in its range.

### Root declarations: narrowing and overriding

A root `version` for an identity **narrows**: it is intersected with every
requirer's range, and resolution and verification hold the pin to all of them.
A root `ref` or `path` **overrides**: the requirers' ranges and selectors for
that identity no longer apply. A range is therefore never widened silently, and
the escape hatch is one visible line in the root manifest.

An override is always reported. `mach dep pull`, `mach dep update` and `mach
dep add` print one note on stderr for each requirement a root declaration
replaced, whatever `--quiet` says, naming the identity, the root's winning
selection, the requirer chain and what that chain asked for:

```
note: dependency 'std': the root declares ref = "tag/v2.0.0", overriding
hedgeacme -> hedge -> std which requires ref = "tag/v2.1.0"; nothing checks that
'std' supports the root's selection
```

A requirement that asks for exactly the root's selection is no override and is
not noted. `mach dep list` shows each root declaration's winning selection
(`ref=`, `version=` or `path=`, and the recorded `pin=`), its state
(`realized`, `missing`, or, for a `version` selection whose range excludes the
pinned release, `out of range (the pinned release 7.0.2 is outside ^8.0)`) and,
under it, every requirement it overrides (`overrides hedgeacme -> hedge -> std, which requires
ref = "tag/v2.1.0"`). `mach dep outdated` names the requirements of a chosen
release that a root or a fixed dependency overrides (`root declares b by ref
"branch/main", overriding root -> a 1.0.0 requires b ^1.2`).

An override is not checked against the requirers. Because the closure is flat,
a requirer's `use b.*` binds to whatever the root selected, even a major that
requirer was never built or tested against. Nothing proves the requirer supports
it: a passing build only shows that the code the build reached compiled, so it is
evidence and not a guarantee. `mach dep verify` prints a note for every edge an
override replaced, without failing (`note: dependency 'b': the root declares ref =
"tag/v2.0.0", overriding root -> a -> b which requires version = "^1.2"; nothing
checks that 'b' supports the root's selection`). Treat each note as a claim to
confirm, by testing the requirer at that selection or by checking its own range.

### A release selects only releases

The rule follows how a manifest was reached, not where it sits:

- A dependency reached through a **release** (a `version` range or an exact
  `tag/`) may itself select dependencies only by `version` or `tag/`. A release
  is then reproducible from its tag, all the way down.
- A dependency reached through a `branch/` or `commit/` selection is in
  development, and its manifest may use any selector.

A release that breaks the rule is refused wherever it is reached. Resolution
stops when it reaches one, and verification (and so every build) refuses it,
naming the chain and the offending line:

```
error: root -> a is a release (ref = "tag/v1.1.0"), and its manifest selects
[dep.b] by ref = "branch/main"; a release may select its dependencies only by
`version` or an exact `tag/`, so it cannot be reproduced from its tag
```

`mach dep verify <path> --release` holds the project itself to the same rule,
so a library's release workflow catches the mistake before it tags the
release, not when its first consumer resolves it.

### Pins are gitlinks; there is no lock file

The record of which commit a dependency is at is the **gitlink** committed in
the root repository, generated into `.gitmodules` by `mach dep`. Nothing else
records a pin: there is no `mach.lock`, and a file of that name in the project
root is an unrelated file no command reads.

A project does not need its own Git repository. In a repository root, Git
dependencies use the staged gitlinks as their pins. A subproject, a project in a
subdirectory of a repository, uses the gitlink the enclosing repository commits
under its prefix (`test/consumer/dep/std` for a subproject at `test/consumer`)
the same way: `pull` realizes that gitlink's commit and `update` moves it and
stages it. Without such a gitlink, and in a filesystem project, Git dependencies
are plain clones whose own checkout commits are verified. Local path dependencies
are verified from their filesystem realizations, independently of any Git index.

A version range is resolved for the whole closure, not for the root's own
declarations alone: a range a dependency declares, whether that dependency was
reached by a range, a `ref` or a `path`, is pinned under the root's `dep/` by
`mach dep add` and `mach dep update`. When the root has no checkout of the
identity yet, resolution starts from the declaring dependency's own committed
gitlink for it, so a dependency brings the pin it was tested with; `update
--all` moves every range to the highest release all of them admit; and a root
declaration of the same identity by `ref` or `path` overrides the range, noted
as above. `pull` refuses a range with
neither a gitlink nor a checkout under the root and names `mach dep update <root>
<id>`, which pins it wherever in the closure it is declared.

### The root owns the flat closure

The root's `dep/` holds every identity in its **transitive** closure, one
directory each, one level deep. The root manifest declares only what the root
uses directly (plus any override, below); a dependency's own dependencies reach
the root's `dep/` through closure computation and never need a declaration in
the consumer. A consumed dependency's own `dep/` is never initialized. A
package cloned on its own is a root and realizes its own flat `dep/`.

So with a root that declares `a`, and `a` that declares `b`, the layout is
`dep/a/` and `dep/b/`, and `a`'s `use b.*` resolves against the root's
`dep/b/`. Git materializes `a`'s own gitlink as an empty `dep/a/dep/b/`
directory; that entry is neither realized, verified, nor descended into.

### One identity, one commit

Identity is the project id, not the key and not the URL. One identity resolves
to exactly one commit per build, with no exception for majors: two majors of one
identity in one closure is a **clash**, not a case the build accommodates. The
diagnostic prints both requiring chains and the exact root declaration that
would resolve it:

```
error: dependency conflict: project id 'b' is reached with two different selections:
    root -> a -> b requires git <url> @ tag/v1.0.0
    root -> c -> b requires git <url> @ tag/v2.0.0
  the root decides by declaring the identity itself, for example:
    [dep.b]
    git = "<url>"
    ref = "tag/v2.0.0"
```

(`<url>` stands for the repository URL as declared.)

The root resolves by declaring the identity with a `ref`, which may point at
upstream or at a fork carrying the same id. A fork slots in without any
consumer source or manifest change, because identity is not the URL. Two
unrelated packages claiming one id is a collision and is rejected. URL
disagreement is a mirror, not a conflict: the root's declared URL wins, else the
first declaring path's; verification compares commits, never URLs. A realized
checkout whose remote points somewhere else (a `.git` suffix, another host, a
local mirror) verifies by its commit alone.

### What a build verifies

Builds never fetch and never write under `dep/`. Every build (and `mach dep
verify` as a command) checks, offline, that:

1. every Git dependency is a clean checkout at its applicable pin
   (`dependency 'std': checkout is dirty:  M mach.toml`), and every path
   dependency is a contained filesystem tree without repository metadata;
2. its project id equals the directory name;
3. the closure computed from the realized manifests equals the set of
   directories under `dep/`: nothing missing, nothing extra. A dependency with
   nothing checked out, whether `dep/<id>` is absent or is the empty directory
   Git leaves for an uninitialized gitlink on a fresh clone, is refused naming
   the command that realizes it (`dependency 'std' is not realized (nothing
   checked out at 'dep/std'); run `mach dep pull <path>` for project '<root>'`,
   and for a `version` selection also `or `mach dep update <path> std` when it
   has no pin yet`);
4. there are no cycles (reported as the chain);
5. every realized manifest's `[project].mach` accepts the running compiler (see
   [Compiler range](#compiler-range));
6. for every identity selected by `version`, the pinned commit carries a
   release tag, read from the checkout's own refs, and that release is inside
   every requirer's range, the root's included; and every release in the
   closure selects only releases (see [A release selects only
   releases](#a-release-selects-only-releases)).

A pin outside a range names the requirer chain, the range, the pinned release
and a runnable remedy (`dependency 'vb': root -> vb requires version '^1.2' but
the pinned release is 1.1.0; run `mach dep update <path> vb` for project
'<root>' to re-pin it, or declare the identity at the root to override`). An
untagged pin reads `... but the pinned commit '<commit>' carries no release
tag`.

The recorded gitlink is the pin, and two kinds of drift from it are refused,
each naming the identity and the command that fixes it:

- a `dep/<id>` checkout at another commit than its gitlink (`dependency 'b':
  the checkout is at '<commit>' but the recorded gitlink is '<other>'; run
  `mach dep pull <path>` for project '<root>' to restore the recorded pin, or
  `mach dep update <path> b` to re-pin it to the manifest's selection`);
- a gitlink outside the manifest's selection. The root's own `tag/` or
  `commit/` must be satisfied by the pin (`dependency 'b': exact ref
  'tag/v1.0.0' required by root -> b resolves to '<commit>' but the realized
  commit is '<other>'; run `mach dep update <path> b` for project '<root>' to
  re-pin it to the root's selection`; a root `commit/` that does not match
  reads `exact commit ref 'commit/<id>' is not satisfied by the realized commit
  '<other>'`), and so must every range the root declares (item 6). `pull`
  realizes the gitlink as it is, so it never cures this; `update` moves the
  gitlink to the selection.

A root `ref` or `path` is the override for its identity, so the requirers'
selectors are not checked against its pin (the override notes above name them).
For an identity the root does **not** declare, every requirer's exact selector
(`tag/`, resolved through the checkout's own refs, or `commit/`) must be
satisfied by the realized commit; a mismatch names both commits and the two
remedies (`dependency 'b': exact ref 'tag/v1.0.0' required by root -> a -> b
resolves to '<commit>' but the realized commit is '<other>'; run `mach dep
update <path> b` for project '<root>' to re-pin it, or declare the identity at
the root to override`). A `branch/` selector is an input to `update`, never a
verify fact. The verifier reads the git **index**, so a freshly realized
dependency is verifiable before it is committed.

`mach dep pull` reads what a Git dependency's `dep/<id>` holds together with
its record (the staged gitlink, its `.gitmodules` entry, and any module
directory Git retained) and takes the one step that brings it to what a build
verifies:

- a staged gitlink with nothing checked out, as on a fresh clone or after the
  directory was deleted, is initialized in place (`realized std @ …
  (initialized the committed gitlink)`), first restoring its `.gitmodules`
  entry from the manifest if that entry is gone;
- a checkout at another commit than its gitlink is checked out at the gitlink;
- a clean checkout of its own with no gitlink is registered, moved to the
  declared selector from the declared source, and staged (`(registered the
  existing checkout)`);
- with neither, the submodule is added at the selector, reusing a module
  directory Git retained from an earlier removal.

A symlink, a file, a directory that is not a checkout of its own, and a dirty
checkout that would be registered are refused and left as they are. `mach dep
add` takes the same step for its Git source, so re-adding a dependency whose
checkout `remove` retained registers that checkout, and it refuses a dirty one.
No gitlink command ever runs against a path that is not a checkout of its own.

A path dependency has no pin, so `mach dep pull` syncs its `dep/<id>` with the
declared `path` every time, and `mach dep update` does the same, once per
command. Unless `--quiet`, each says whether the copy was refreshed or reused (`realized hedge
from ../.. (copy refreshed from its source)`, or `(copy reused: it already
matched its source)`), so a copy left from another checkout cannot pass
unnoticed. A build reads the copy as it is and never syncs it. A changed
`path` realizes the new source. A file the source no longer has is removed and
named, and a file whose content differs from the source is overwritten and named
(`replaced 'src/lib.mach' with its source's content`), so local edits to the
copy do not survive a pull. A `dep/<id>` that is a symlink is refused and left
as it is.

A project root is identified by its own `mach.toml`, not by an enclosing git
repository; `dep/<id>` is resolved relative to the project root. A project
nested inside an unrelated repository or without any repository builds. Git
dependencies in these projects are verified from their own plain checkouts.

### Selection on `update`

`mach dep update` is the only command that moves a pin. It advances every
`branch/` selector to its current remote tip and re-stages the gitlink, and it
moves an exact selector to the commit it names, so an identity realized at a
dependency's selection lands on the root's declaration once the root declares
one (`b: 0564… -> e508… (pinned to the exact selector)`, or `(exact selector,
already pinned)` when nothing moves). `<name>` is looked up in the whole
dependency closure, so `mach dep update <path> b` for an identity the root does
not declare moves its checkout to the selector its requirers declare. A
name outside the closure is refused (`dependency 'x' is not in the dependency
closure`). For an identity reached by
more than one path, one rule decides: the root's selector wins if the root
declares the identity; otherwise agreement among the requirers is taken;
otherwise the command stops, prints both chains, and names the root
declaration that would decide (the diagnostic above). A consumed
dependency's own gitlink records a tested commit, readable without initializing
that dependency's `dep/`. It is not a compatibility floor. Compatibility is
stated by ranges, and `update` resolves every version-selected identity as
described in [Releases and resolution](#releases-and-resolution).

### Removed forms

Two shapes of a realized closure are refused by `pull`, `verify` and every
build:

- a **key that is not the project id**, a `[dep.<key>]` whose realized project
  declares a different id:
  `[dep.foo] realizes project 'std': the manifest key, the directory under
  dep/, and the project id are one name, so rename the table to [dep.std] and
  the directory to dep/std`;
- a **nested realization**, a `dep/<id>/dep/<x>/mach.toml`: `dependency 'a':
  dep/a/dep/b is a nested realization: the root's dep/ owns the flat closure
  and a dependency's own dep/ is never realized, so delete dep/a/dep`. The
  empty directory git materializes for a consumed dependency's own gitlink is
  not a realization and passes.

Command-line usage (`pull`, `verify`, `add`, `update`, `remove`, `list`) is
documented by `mach help dep`.

A dependency's export surface — all a consumer sees — is its source module tree
(addressed by the dep's id), the module a bare `use <id>;` binds, its
`export = true` link entries, and the steps those entries demand. Nothing else
in a dependency's manifest applies to consumers.

## Path templates

Paths and `cmd`s expand over a closed, final set of eight variables:

- `{project.out}` — the **root** project's expanded `[project].out`, in every
  manifest of the closure.
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
  root project's directory exactly as `{project.out}` is. In a dependency's module
  it names a requirement of that dependency's default library artifact, homed under
  `dep/<id>`. See [Artifact requirements](#artifact-requirements).

The three `{target.*}` tuple keys are also exported to every step process as
`MACH_TARGET_ISA`/`MACH_TARGET_OS`/`MACH_TARGET_ABI` (see [build steps](#stepname--build-steps)).

An artifact's `out` is relative to the expanded project `out` and is rooted there
automatically — write `bin/demo`, not `{project.out}/bin/demo`. Step `out` lists
and local link `path`s are **not** auto-rooted: they name `{project.out}`
explicitly, which is what homes a dependency's build products into the *consumer's*
output tree rather than the dependency's checkout.

There are no `{name}`/`{ext}` or bare `{target}`/`{profile}` aliases. An
unresolvable `{...}` reference, or an unterminated `{`, is a strict-parse error.
`{project.out}` is not available inside `[project].out` itself (it would be
self-referential), and `{artifact.<id>.out}` is not available inside an artifact's
own `out` for the same reason. `{artifact.suffix}` is available nowhere but an
artifact's own `out`. Two artifacts selected for one target that resolve to the
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
out     = "shaders/blur.spv"
targets = ["vulkan"]
link    = []
need    = []

[artifact.app]
kind    = "bin"
entry   = "main.mach"
out     = "bin/app"
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
- artifact cycles and step cycles, including cycles formed by globs.

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
**`default = true` library artifact** carries its requirements as part of its
export surface: the artifacts and steps its `need` names are built for any project
whose dependency closure holds that dependency, before the cells that compile
against the closure. Only that artifact's `need` travels. Several library artifacts
may share the `default` marker and the public entry; their requirements travel
together.

```toml
# the dependency's mach.toml
[artifact.shlib]
default = true
kind    = "static"
entry   = "lib.mach"
out     = "lib/shlib{artifact.suffix}"
targets = ["linux-x86_64"]
link    = []
need    = ["artifact.shader-frag"]

[artifact.shader-frag]
kind    = "bin"
entry   = "shaders/frag.mach"
out     = "spv/frag{artifact.suffix}"
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
- **Homed in the consumer, namespaced by id.** The output is
  `<expanded root [project].out>/dep/<dependency id>/<the artifact's own out>`, so
  two dependencies that both declare `shader-quad` produce two files and neither
  writes into its own checkout. `{project.out}` keeps meaning the root's out in
  every manifest of the closure, as it does for a dependency's steps, and the
  cell's objects sit in the root's `obj/` beside every other module's. `mach clean`
  removes that home and those objects with the rest of the output, reading the
  realized dependency manifests for the target names the root never declares.
- **Scope follows the module.** `{artifact.<id>.out}` in a module the dependency
  owns is read in that dependency's manifest and checked against its default
  library artifact; the root's modules keep reading the root's manifest and the
  requirements of the artifact being built. Naming anything else is the ordinary
  refusal, located at the scope it was read in.
- **Its own closure, recursively.** A travelling requirement compiles against the
  dependency's own transitive closure, realized in the root's flat `dep/`, and
  the requirements of *those* dependencies' default library artifacts travel to
  it in turn. A requirement reached through several consumers is built once, and
  a failure names the dependency chain (`dependency app -> boom -> shader: ...`).
- **Steps too.** A step the default library artifact's `need` names runs for the
  consumer, alongside the steps its `export = true` link entries demand.

Nothing here makes an `#[embed]` an edge in the build graph: a missing embedded
file is still a compile error and still triggers nothing (#2887). What runs is
the requirement the dependency declared.

## Selection and the build matrix

A build cell is one artifact × one target × one profile.

With no `--bin`/`--lib`, every command reads one rule, the **default selection**:
of the artifacts whose `targets` includes the selected target, those marked
`default = true` when any is marked, and every one of them when none is.

- `mach build <path>` and `mach check <path>` build and check the default selection
  for the selected target, for the default profile. `--all-targets` crosses every
  artifact with every target in its `targets`, applying the default selection per
  target. `--bin <name>` / `--lib <name>` narrow to one artifact, marked or not;
  `--target <name>` selects a declared target; `--profile <name>` selects a profile.
- `mach run <path>` consumes exactly one artifact. With no `--bin`/`--lib`, it selects
  one when exactly one artifact declares the resolved target; if several do, it asks
  you to pick one, naming every candidate.
- `mach test <path>` and `mach doc <path>` need one artifact as their primary
  context and take the default selection when it holds one artifact: `--bin`/`--lib`
  wins, a sole artifact that declares the resolved target is chosen, several
  need exactly one `default = true` (several with none marked is refused).
  `mach test` builds that artifact's cell as `mach build` would, its closure, its
  `link` entries, its `need` and exported dependency entries, and links the test
  dispatcher in place of its entry. Foreign-target tests require a compatible
  `--runner`.

### Enumerated cells are filtered; named ones are not

A cell whose artifact does not list the cell's target is a cell the manifest never
declared, so enumerating skips it. Naming that pair is a different act: `--bin
kernel --target host` is refused by name, because you asked for a cell that does not
exist. `--bin kernel --all-targets` re-enumerates the target axis and so filters
back to the targets `kernel` declares.

If a selection is well-formed but enumerates nothing — a `--target` no artifact
lists — the build fails naming that target and the declared artifacts, rather than
succeeding with an empty plan.

### A named artifact can settle the target

`--bin <name>` / `--lib <name>` with no `--target` lets the artifact decide, since
its `targets` list may already leave only one answer:

- exactly one declared target: that target is used, and `--target` would only
  repeat what the manifest already said
- several, one of which matches the host: the host target, as before
- several, none matching the host: refused, naming the targets the artifact does
  declare so the choice is visible without opening `mach.toml`

An explicit `--target` always wins, including when it names a target the artifact
does not list — that pair is still refused by name. This only applies to a named
artifact: enumerating the artifact axis keeps the target fixed for the whole
matrix, so a bare `mach build <path>` never widens into a target it was not asked
for.

### `-o` names one output

`-o` is accepted exactly when the selection resolves to a single build cell, and
refused otherwise, naming the cells it resolved to. Two artifacts collide on one
output path the same way two targets do: each would link over the previous, leaving
only the last with no warning. Narrow with `--bin`/`--lib` and `--target`.

`-o` names a canonical path inside the project root, as an artifact's `out` does:
relative, `/`-separated, with no `.` or `..` component and no empty one.
`-o ../mach`, `-o ./mach` and `-o /tmp/mach` are refused with `-o must name a
canonical path inside the project root`, so a build never writes outside the
tree it was asked to build.

### When one cell fails

Every cell is attempted; a failure does not abandon the ones after it. Each cell's
diagnostics are reported under its own heading as it happens, and every cell that
succeeded leaves its artifact on disk at its own path — nothing is rolled back. The
exit code is `0` when all cells succeeded, `2` if the first failure was an internal
error, and `1` otherwise.

Artifacts cannot share an output path: a manifest whose expanded `out` templates
collide is rejected before the build starts.

### `native` target resolution

`native` resolves the host's `(isa, os)` against the **declared** targets only —
never a synthesized tuple. Exactly one host match is chosen; several matching tuples
is an ambiguity error naming the candidates. With no match, a sole declared target
is chosen with a warning, so a cross-only project still builds on a foreign host;
several declared targets select the one marked `default = true` (or an explicit
`--target`). A manifest that declares several and marks none is refused, since
table order carries no meaning:

```
error: mach.toml: several targets are declared, none matches the host and none is marked `default = true`; no target is selected by table order: mark exactly one [target.<name>] with `default = true` or select one with --target
```

The same rule applies to `[profile.*]` and to `[artifact.*]` when a command
needs one artifact.

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
out     = "bin/demo"
targets = ["linux", "windows"]
link    = ["shim", "shim-x11", "shim-win32", "gl"]
need    = []

[step.shim]
argv = ["cc", "-c", "-O2", "-fPIC", "-Ivendor/platform", "-o", "{project.out}/obj/platform/shim.o", "vendor/platform/shim.c"]
in   = ["vendor/platform/shim.c"]
out  = ["{project.out}/obj/platform/shim.o"]
need = []

[step.shim-x11]
argv = ["cc", "-c", "-O2", "-fPIC", "-Ivendor/platform", "-o", "{project.out}/obj/platform/x11.o", "vendor/platform/x11.c"]
in   = ["vendor/platform/x11.c"]
out  = ["{project.out}/obj/platform/x11.o"]
need = []

[step.shim-win32]
argv = ["cc", "-c", "-O2", "-fPIC", "-Ivendor/platform", "-o", "{project.out}/obj/platform/win32.o", "vendor/platform/win32.c"]
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
simd  = "scalarize"

[profile.release]
opt   = 2
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
out     = "out/{target.name}/{profile.name}"

[link.glfw]
source = "system"
name   = "glfw"
library = "glfw"
os     = ["linux", "darwin"]
isa    = "*"
abi    = "*"
export = true

[link.glfw-win]
source = "system"
name   = "glfw3.dll"
library = "glfw"
os     = ["windows"]
isa    = "*"
abi    = "*"
export = true

[link.Cocoa]
source = "framework"
name   = "Cocoa"
os     = ["darwin"]
isa    = "*"
abi    = "*"
export = true
```

Both GLFW entries expose the logical name `glfw`, so the binding can use the
same attribution on every target:

```mach
#[library("glfw")]
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
out     = "out/{target.name}/{profile.name}"

[link.miniz]
source = "local"
path   = "{project.out}/obj/miniz/miniz.o"
os     = "*"
isa    = "*"
abi    = "*"
export = true

[step.miniz]
argv = ["cc", "-c", "-O2", "-fPIC", "-Ivendor/miniz", "-o", "{project.out}/obj/miniz/miniz.o", "vendor/miniz/miniz.c"]
in   = ["vendor/miniz/*.c", "vendor/miniz/*.h"]
out  = ["{project.out}/obj/miniz/miniz.o"]
need = []
```

## The compiler's own manifest

Mach builds itself from a manifest that declares six targets, two explicit
profiles, two binary artifacts with literal output paths, and one dependency,
`std`:

```toml
[project]
id = "mach"
version = "5.0.0"
mach = "^5.3"
src = "src"
out = "out/{target.name}/{profile.name}"

[target.linux-x86_64]
isa = "x86_64"
os  = "linux"
abi = "sysv64"

[target.windows-x86_64]
isa  = "x86_64"
os   = "windows"
abi  = "win64"
stack_reserve = 0x800000

[profile.debug]
default = true
opt = 0
debug = false
simd = "scalarize"
vectorize = true
float_reassoc = false

[profile.release]
opt = 2
debug = false
simd = "scalarize"
vectorize = true
float_reassoc = false

[artifact.mach]
kind = "bin"
entry = "bin/main.mach"
out = "bin/mach"
targets = ["linux-x86_64", "linux-arm64", "linux-riscv64", "darwin-x86_64", "darwin-aarch64"]
link = []
need = []

[artifact.mach-windows]
kind = "bin"
entry = "bin/main.mach"
out = "bin/mach.exe"
targets = ["windows-x86_64"]
link = []
need = []

[dep.std]
git = "https://github.com/briar-systems/mach-std"
ref = "commit/e6fc41251e442eb736d4d15de677903a4ba52461"
```

(The full manifest declares all six targets; `version` is whatever the tree's
current release is, and the `std` selector is the exact std 2.0.0 commit the
tree builds against until the tag is cut.) `mach build .` selects the host-matching target via
`native`, compiles `src/bin/main.mach` and its transitive imports — including
modules from `std` at `dep/std/` — and links `out/<target>/<profile>/bin/mach`.
The two artifacts keep literal outputs rather than one
`bin/mach{artifact.suffix}` because the published seed compiler that
bootstraps this tree predates the template; a manifest the seed must parse
stays within what the seed accepts.
`dep/std` is a git submodule whose gitlink is the pin; the build resolves it
purely by that directory and verifies the gitlink from the repository's index,
fetching nothing.

## See also

- [files.md](files.md) — file layout and `lib.mach` / `main.mach`
- [modules.md](modules.md) — how files map to module paths
- [ext-fun.md](ext-fun.md) — linking against external symbols
