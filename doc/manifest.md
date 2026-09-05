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

## Totality

A manifest is required. A project directory without one does not build:

```
error: no mach.toml in the project directory
```

Nothing is inferred from the directory layout. `mach init` writes a complete
manifest so a new project never starts from that error (see
[cli.md](cli.md#mach-init)).

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
[language/modules.md](language/modules.md#bare-project-id-imports)), its
`export = true` link entries, and the steps those entries demand. A dependency's
`[profile.*]` and `[target.*]` tables are never read to build the consumer.

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
simd  = "scalarize"                    # SIMD lever: "scalarize" | "require"

[artifact.demo]                        # a produced artifact
kind    = "bin"                        # "bin" | "static" | "shared"
entry   = "main.mach"                  # entry source, relative to src
out     = "bin/demo"                   # output path, relative to the project out
targets = ["*"]                        # which declared targets build it ("*" = all)
link    = []                           # [link.X] names this artifact links
need    = []                           # [step.X] / [artifact.X] names this artifact requires
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

`[project]` is exactly these four keys. `name`, `description`, and `mach` are
deprecated until 5.0.0: 4.26.x accepted them and never read them, so 4.30.0
accepts them with a warning naming the key, the table, and the 5.0.0 refusal,
in a root manifest and a dependency's alike; `[profile.<name>]`'s `emit_ir` and
`emit_asm` are in the same window (emission is `--emit-ir`/`--emit-asm` on the
command line). Any other key is an unknown-key error in every manifest.

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
| `env` | no | Consumer environment (string). The values are owned by the target's isa: an `env` the isa does not define is a manifest error naming the target and the known values, and an isa that defines none refuses the key outright. Today only `spirv` defines any; see [Finished-module targets](#finished-module-targets). |

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
| `isa` | `x86_64`, `aarch64`, `riscv64`, `riscv32`, `spirv` |
| `os`  | `linux`, `windows`, `darwin`, `freestanding` |
| `abi` | `sysv64`, `win64`, `aapcs64`, `lp64`, `lp64f`, `lp64d`, `ilp32`, `ilp32f`, `ilp32d`, `spirv` |

`mos6502` (an `isa` and an `abi`) is still accepted by 4.30.0 as a withdrawn
experiment and is removed in 5.0.0; do not declare it.

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
profile**: `riscv64` is `rv64gc` and `riscv32` is `rv32imac`. That default is
the only profile a target can name today. There is no key that bounds the
extensions below it (an `isa = "rv32imc"` or a standard profile string is not
accepted), and the width-only spelling is never rejected; extension bounding is
future additive work. The ABI, not the isa, selects the calling convention and
the float facts.

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
(see [Totality](#totality)). `riscv32`
currently only reaches a `freestanding` target — `mach info targets` lists
`freestanding-riscv32` and no `linux`/`darwin` riscv32 tuple.

`mach info targets` prints the tuples this binary can actually build; it is
derived from the same declarations composition reads, so it never advertises a
tuple that would fail to resolve.

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

### Finished-module targets

A `spirv` target's object output is a complete, self-contained module rather than
a link input. The build therefore delivers the **module tree** — one
`<out>/obj/<fqn-as-path>.spv` per module — and runs no link phase, so a default
build and `--emit obj` produce the same files:

```toml
[target.gpu]
isa = "spirv"
os  = "freestanding"
abi = "spirv"
# no `of`: the finished-module format resolves on its own
```

```
mach build . --target gpu     # writes out/gpu/<profile>/obj/<module>.spv
```

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

The artifact's `out` template and `-o` name a linked binary, which such a target
has none of; the module tree is delivered instead. A `static` or `shared`
artifact kind, and `mach test`, are refused by name — there is no archive, shared
object, or executable form for a module.

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

A profile is a build variant. The optimization level and debug-emission toggle
live here because they are variant concerns.

| Key     | Type    | Meaning |
|---------|---------|---------|
| `opt`   | integer | Optimization level: `0` selects the debug pipeline (the always-on passes only), `1` and `2` select the release pipeline. `1` and `2` currently share a pass set, which includes loop auto-vectorization (see `vectorize` below). Any other integer — or a non-integer — is a manifest error. |
| `debug` | bool    | Emit debug info (DWARF on ELF/Mach-O, CodeView on COFF) for this profile. Gates emission only, never the optimizer, so a `release` profile can keep symbols with `debug = true`. A non-boolean is a manifest error. |
| `simd`  | string  | SIMD scalarization lever. `"scalarize"` emits a defined unrolled scalar expansion wherever the target has no packed instruction for a vector operator, with a build-time note. `"require"` makes that a hard error naming the operation, its **lane width**, the function and the target. It applies **per operation on every target**, not only to targets with no vector unit: x86-64's SSE2 baseline has no 32-bit lane integer multiply and NEON has no 64-bit one, so a capable target scalarizes too. Any other string is a manifest error. |
| `vectorize` | bool | **Optional** auto-vectorization lever; absent it defaults to `true`. When `true`, the release pipeline rewrites provably-safe counted loops to 128-bit SIMD on a target with hardware vectors; `false` skips the pass, so release output stays scalar. A non-boolean is a manifest error. |
| `float_reassoc` | bool | **Optional** permission to treat floating-point addition and multiplication as **associative**; absent it defaults to `false`. It lets the vectorizer reduce an `f32`/`f64` accumulator through lane-count partial sums, which changes the result — see [Float reassociation](#float-reassociation) for what that costs and what it buys. A non-boolean is a manifest error. |
| `default` | bool | **Optional.** `true` marks the profile a build uses when several are declared and `--profile` is absent. Exactly one profile may carry it. See [Built-in profiles and profile selection](#built-in-profiles-and-profile-selection). |

Three keys (`opt`, `debug`, `simd`) are required in a declared profile;
`vectorize` and `float_reassoc` are optional, defaulting to on and off
respectively, and `default` is optional.

### Built-in profiles and profile selection

A manifest that declares no `[profile.*]` table at all gets two built-in
profiles, `debug` (`opt = 0`, `debug = true`) and `release` (`opt = 2`), and
`debug` is the default. Declaring any profile replaces both built-ins: a
manifest with only `[profile.fast]` has no `debug` and no `release`.

Which profile a build uses follows one rule, the same one that selects a
target and an artifact:

1. an explicit `--profile <name>` wins;
2. otherwise a sole declared profile is chosen;
3. otherwise the one marked `default = true` is chosen.

Table order carries no meaning. In 4.30.0 a manifest that declares several
profiles and marks none still builds: the first declared profile is taken with
a warning that names the fix. 5.0.0 refuses that manifest.

```
warning: mach.toml: several profiles are declared and none is marked `default = true`; the first declared profile is selected by table order, which 5.0.0 stops doing: mark exactly one [profile.<name>] with `default = true` or select one with --profile
```
Emission of the human-readable IR and assembly side-artifacts is **not** a profile
concern — it is controlled only by the `--emit-ir` / `--emit-asm` CLI flags (see
[cli.md](cli.md)).

The `vectorize` lever only ever *subtracts*. The pass it gates runs in the release
pipeline on targets that report 128-bit vector support (SSE2 on x86-64, NEON on
aarch64, and `OpTypeVector` on spirv) and rewrites counted, unit-stride loops whose dependence analysis proves
independence — element-wise maps behind a runtime alias guard, and associative-exact
integer reductions. A loop it cannot prove safe stays scalar, and a target without
hardware vectors (riscv64) never enters the pass, so `vectorize = false` changes
performance and never semantics. For a single function, the `#[scalar]` decorator is
the finer-grained opt-out (see [language/decorators.md](language/decorators.md)).

`float_reassoc` is the one lever here that *adds*, and the only profile key that can
change a program's computed answer. It widens that same pass to float reductions and
does nothing else; `vectorize = false` switches the pass off wholesale and so overrides
it.

The `simd`, `vectorize` and `float_reassoc` levers are always the **consumer's**.
Consistent with the root-vs-dependency strictness above, a dependency's `[profile.*]`
is parsed permissively and never read to build the consumer, so a library's values are
inert — the effective levers come from the consumer's resolved profile. Libraries set
nothing SIMD-specific and inherit the consumer's choice; there is no ecosystem fork and
no dual API.

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

Every artifact is declared explicitly and named by its table key. `$project.name`
reads the selected artifact's name.

| Key       | Required | Meaning |
|-----------|----------|---------|
| `kind`    | yes | `"bin"`, `"static"`, or `"shared"` (see below). |
| `entry`   | yes | Entry source, relative to the project `src` dir (e.g. `main.mach` for `src/main.mach`). The entry module's FQN is `<id>.<entry without .mach>`, `/` turned into `.`. |
| `out`     | yes | This artifact's output path, **relative to the expanded project `out`** and rooted there automatically — write `bin/demo`, not `{project.out}/bin/demo`. An executable extension, where wanted, is written literally here, and it is one string shared by every target in `targets`, so an executable that ships on Windows and elsewhere **cannot use `targets = ["*"]`** (see [One artifact per extension convention](#one-artifact-per-extension-convention)). |
| `targets` | yes | Array of declared target names this artifact builds for; `["*"]` means every declared target. |
| `link`    | yes | Array of `[link.X]` names this artifact links (see below). `[]` for none. A name with no table is a manifest error naming the artifact and the declared tables (`[artifact.p1].link names no [link.*] table: 'nosuch' (declared: [link.kernel32])`). |
| `need`    | yes | Array of `[step.X]` and `[artifact.X]` names this artifact requires, and `*`-globs over both. `[]` for none. See [Artifact requirements](#artifact-requirements). |
| `subsystem` | no | `"console"` (default) or `"gui"` — the environment a windows executable declares it runs under (see below). |
| `icon` | no | Project-root-relative `.ico` path embedded in a Windows executable's PE resources. Non-empty path string; `bin` artifacts only. |
| `manifest` | no | Project-root-relative application-manifest path embedded byte-for-byte in a Windows executable's PE resources. Non-empty path string; `bin` artifacts only. |
| `default` | no | `true` marks the artifact chosen when a command needs one artifact (`mach test`, `mach run`, the editor's union build) and several declared artifacts support the selected target. Exactly one of those candidates may carry it; an explicit `--bin`/`--lib` always wins, and a sole candidate needs no marker. |

`entry` is the build cell's source root. The build follows its active `use` and
`fwd` edges transitively and compiles that reachable module set; another file under
`src` is not part of the cell merely because it shares the project directory. This
is what lets one project declare host and accelerator artifacts with disjoint target
sets. `mach test` is the deliberate whole-source exception: it roots collection at
every module in the current project's `src` tree.

- **`bin`** links an executable at the resolved `out` path.
- **`static`** materialises a real `ar` archive at the resolved `out` path — the
  per-module objects with an archive symbol index, the deliverable a consumer links
  as a `.a` (#1997).
- **`shared`** is reserved for a shared-library deliverable; its emission is phase 2
  (#1980).

Per-target extension or per-target entry is not a per-cell exception table — it is a
second artifact stanza, so the condition stays visible like everything else.

### One artifact per extension convention

`out` is one literal string, and every target in `targets` resolves it the same way.
Windows will not execute a file without an executable extension until someone renames
it by hand, and no other platform wants one, so there is no single `out` that is right
for both. `bin/app` gives Windows an unrunnable `app`, and `bin/app.exe` gives linux
and darwin a binary called `app.exe`. An executable that ships on Windows and anywhere
else therefore cannot use `targets = ["*"]`. It is two artifacts with disjoint
`targets` lists:

```toml
[artifact.app]
kind = "bin"
entry = "main.mach"
out = "bin/app"
targets = ["linux-x86_64", "darwin-aarch64"]
link = []
need = []

[artifact.app-windows]
kind = "bin"
entry = "main.mach"
out = "bin/app.exe"
targets = ["windows-x86_64"]
link = []
need = []
```

This is deliberate rather than a defect, and it costs three things worth knowing before
you meet them.

The two stanzas differ only in `out` and `targets` and are otherwise duplicates, so
they drift. A `link` or `need` added to one and not the other changes the build on
Windows only, which is the platform least likely to be the one in front of you.

Every new target has to be added to the right list by hand, because neither stanza can
use `*`. Declaring a target and forgetting to list it means that target simply builds
nothing.

The artifact **name** differs between the two, so `$bin.name` differs by platform: a
project that reads it sees `app` everywhere and `app-windows` on Windows. Nothing in
mach's own source reads it, and a project that does needs to expect both.

`mach build` enumerates artifact-by-target cells and builds only the ones that match,
so each platform gets its stanza with nothing named on the command line. `mach test`
links the whole source tree rather than one artifact, but selects its primary context
from the sole artifact that declares the resolved target, so the same split works
there. `mach run` selects the target's artifact when exactly one matches; if several
artifacts can run on that target, it still asks for `--bin` rather than guessing.

mach's own `mach.toml` is split this way: an ordinary Windows build produces
`bin/mach.exe`, and its test run selects `mach-windows` as the primary context without
a command-line workaround.

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

The key takes no `os` filter, and it is not an error on a linux or darwin target.
Only the PE writer consumes it — ELF, Mach-O, and flat images have no such field —
so on any other target the key is accepted and inert, changing nothing about the
output. That matches how `[link.X]` entries carry `os`/`isa`/`abi` axes on every
declaration and simply do not apply to the cells they do not match: the manifest
stays one declaration read by every build, rather than a per-platform file.

`--subsystem console|gui` overrides the key for one invocation; see
[cli.md](cli.md#mach-build).

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
[cli.md](cli.md#static-vs-dynamic-resolution) for the resolution rules and
[language/ext-fun.md](language/ext-fun.md#linking-external-objects) for the
`ext fun` workflow that consumes these inputs.

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
| `need` | yes | Array of other `[step.X]` names this step must run after (explicit ordering; cycles error). `[]` for none. |
| `timeout_seconds` | no | Positive integer number of seconds after which the step's process group is terminated and the build fails. Omit for an unbounded step. |

Steps carry **no filters** and **never run automatically**. A step runs only when
**demanded**:

- by a selected `[link.X]` whose `local` `path` matches the step's `out`;
- by another step's `need`;
- by an artifact's `need` (for outputs that are not link inputs), by name or
  through a glob.

Because a step has no filter of its own, the condition for running it lives in the
link entry that demands it: on a build cell where that entry filters out, the step
is never demanded and never runs.

A step is cached by content: its `in` contents plus its expanded `argv` fingerprint
the step (the query engine's `Q_LINK_CONFIG` pattern). An unchanged step whose
outputs still exist is skipped; change an input or the command and it re-runs.

**Bounding a step.** `timeout_seconds` gives the step a deadline measured from
the moment it is spawned. When the deadline passes, the step's whole process
group is signalled — a compiler or archiver the step's shell invoked dies with
it rather than outliving the build — the child is reaped, and the build fails
naming the step and the bound. Omitting the key leaves the step unbounded,
which is the default and the behaviour of every step that does not set it. The
value is an integer number of seconds; zero, a negative number, and a
non-integer are rejected at manifest load, and there is no duration grammar.
The bound is not part of the step's cache key: changing it does not invalidate
a cached step, because it cannot change what the step produces.

A source file's `#[embed(...)]` decorator (see
[decorators.md](language/decorators.md#embedstr--compile-time-file-embedding))
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
The step's environment is the planner's environment with those three variables
**replaced**, never appended: a `MACH_TARGET_*` value inherited from an
enclosing build is overwritten by the cell's own. The same three values are
available in the `argv` templates as the `{target.*}` keys (see
[Path templates](#path-templates)).

`cmd` was the pre-4.x spelling of a step's command and is a removed key:
`[step.s] uses removed key 'cmd'; use a nonempty 'argv' array`.

## `[dep.<id>]`

A dependency is named by its **project id**, and that one name is used in three
places: the manifest key `[dep.<id>]`, the directory `dep/<id>/`, and the head
segment of every module path the dependency exposes (`use <id>.x;`). The
compiler checks all three agree: `dep/<id>/mach.toml` must declare
`id = "<id>"`.

```toml
[dep.std]
git = "https://github.com/briar-systems/mach-std"
ref = "branch/main"
```

A stanza declares exactly one source:

| Key    | Meaning |
|--------|---------|
| `git`  | Git URL. The dependency is a git **submodule** at `dep/<id>/`, pinned by the gitlink the root repository commits. Requires `ref`. |
| `ref`  | Selector for `git`: `branch/<name>`, `tag/<name>`, or `commit/<full-object-id>`. Any other spelling is rejected (`[dep.std].ref must be branch/<name>, tag/<name>, or commit/<full-object-id>`). |
| `path` | Local project tree, never fetched. A relative `path` is resolved relative to this manifest's directory. `mach dep add --path` copies its tracked files into `dep/<id>/` as ordinary tracked files of the root repository, without the source's own `dep/`. Forbids `ref`. |

`git` and `path` are mutually exclusive and exactly one is required. A
registry-style `version =` is reserved and rejected
(`[dep.std].version is reserved for the registry era`).

### Pins are gitlinks; there is no lock file

The record of which commit a dependency is at is the **gitlink** committed in
the root repository, generated into `.gitmodules` by `mach dep`. Nothing else
records a pin: there is no `mach.lock`, and a `mach.lock` left over from an
earlier release is not read. In 4.30.0 it is ignored; 5.0.0 rejects a project
that carries one, with a diagnostic naming the migration.

Because the pin is a gitlink, the source distribution of a project is a git
clone: a tree without `.git` does not build, and dependency verification never
degrades in its absence (`project root '...' is not inside a Git repository;
dependency realization is verified from Git`).

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

1. every `dep/<id>` is a gitlink checked out at the committed commit and clean
   (`dependency 'std': checkout is dirty:  M mach.toml`), or tracked path
   content;
2. its project id equals the directory name;
3. the closure computed from the realized manifests equals the set of
   directories under `dep/`: nothing missing (`dependency 'std' is not
   resolved (missing 'dep/std'); run `mach dep pull`), nothing extra;
4. there are no cycles (reported as the chain).

The committed gitlink is what is verified, and the root's own declaration is
the override: the root's gitlink is its pin, and a `tag/` it declares is not
re-checked against that pin (a `commit/` it declares is). For an identity the
root does **not** declare, every requirer's exact selector (`tag/`, resolved
through the checkout's own refs, or `commit/`) must be satisfied by the
realized commit; a mismatch names both commits and the two remedies
(`dependency 'b': exact ref 'tag/v1.0.0' resolves to '<commit>' but the
realized commit is '<other>'; run `mach dep update b` to re-pin it, or declare
the identity at the root to override`; a root `commit/` that does not match
reads `exact commit ref 'commit/<id>' is not satisfied by the realized commit
'<other>'`). A `branch/` selector is an input to `update`, never a verify fact.
The verifier reads the git **index**, so a freshly realized dependency is
verifiable before it is committed.

`mach dep pull` on a fresh clone finds each committed gitlink as an empty
directory and initializes it in place (`realized std @ … (initialized the
committed gitlink)`); no gitlink command ever runs against a path that is not
a checkout of its own.

A project root is identified by its own `mach.toml`, not by an enclosing git
repository; `dep/<id>` is resolved relative to the project root. A project
nested inside an unrelated repository builds. When the root is not a repository
root, verification says so rather than reporting a missing gitlink.

### Selection on `update`

`mach dep update` is the only command that moves a pin. It advances every
`branch/` selector to its current remote tip and re-stages the gitlink, and it
moves an exact selector to the commit it names, so an identity realized at a
dependency's selection lands on the root's declaration once the root declares
one (`b: 0564… -> e508… (pinned to the exact selector)`, or `(exact selector,
already pinned)` when nothing moves). For an identity reached by
more than one path, one rule decides: the root's selector wins if the root
declares the identity; otherwise agreement among the requirers is taken;
otherwise the command stops, prints both chains, and names the root
declaration that would decide (the diagnostic above). A consumed
dependency's own gitlink for an identity is its tested floor, readable
without initializing that dependency's `dep/`. 5.0.0 adds a SemVer proposal on
top: among tagged releases at or above every tested floor and within one
major, the highest is proposed; candidates spanning two majors are a clash.
The proposal never bypasses a root override.

### 4.30.0 and 5.0.0

4.30.0 accepts two older forms beside the ones above and notes the migration;
5.0.0 rejects them:

- an **alias key**, a `[dep.<key>]` whose realized project declares a
  different id. 4.30.0 realizes it and prints
  `note: [dep.foo] realizes project 'std'; rename the table to [dep.std] and
  the directory to dep/std. alias keys are rejected in 5.0.0`;
- **nested realization**, a `dep/<id>/dep/` left by an older tool; 4.30.0
  ignores it;
- `mach.lock`, ignored in 4.30.0 as above; `pull` prints
  `note: mach.lock is not read; the committed gitlinks under dep/ are the
  pins, so delete it. mach.lock is rejected in 5.0.0`.

Command-line usage (`pull`, `verify`, `add`, `update`, `remove`, `list`) is
documented in [cli.md](cli.md#mach-dep).

A dependency's export surface — all a consumer sees — is its source module tree
(addressed by the dep's id), the module a bare `use <id>;` binds, its
`export = true` link entries, and the steps those entries demand. Nothing else
in a dependency's manifest applies to consumers.

## Path templates

Paths and `cmd`s expand over a closed, final set of seven variables:

- `{project.out}` — the **root** project's expanded `[project].out`, in every
  manifest of the closure.
- `{target.name}` — the resolved target name (never the literal `native`).
- `{target.isa}` — the resolved target's `isa` (e.g. `x86_64`).
- `{target.os}` — the resolved target's `os` (e.g. `linux`).
- `{target.abi}` — the resolved target's `abi` (e.g. `sysv64`).
- `{profile.name}` — the selected profile name.
- `{artifact.<id>.out}` — the output path of a required artifact, relative to the
  project root exactly as `{project.out}` is. See
  [Artifact requirements](#artifact-requirements).

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
own `out` for the same reason. Two artifacts that resolve to the same `out` path
collide and fail at build start.

## Artifact requirements

An artifact's `need` names what must exist before it is built. An entry is a
`[step.X]` name, an `[artifact.X]` name, or a `*`-glob matching either. A glob
never matches the artifact that declares it.

A required artifact is built before its consumer, for the consumer's profile and
for the required artifact's **own** targets: the consumer's target when the
requirement declares it, and otherwise every target the requirement names. That is
what lets a host executable require a shader compiled for an accelerator target.
A requirement reached from several consumers is built once.

Inside the consumer, `{artifact.<id>.out}` expands to that artifact's output path,
so the consumer can name the file:

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
need    = ["shader-*"]
```

```mach
#[embed("{artifact.shader-blur.out}")]
val BLUR: [_]u8;
```

An `#[embed]` argument holding a template resolves against the **project root**, as
the template's own value does; a literal `#[embed]` path keeps resolving against the
declaring file's directory. Only `{artifact.<id>.out}` may appear in an `#[embed]`
path.

Requirements are within one manifest: a `need` entry never reaches into a
dependency.

These are errors:

- a `need` entry that is neither an identifier nor a glob over one;
- a name that is both a `[step.X]` and an `[artifact.X]` — the diagnostic names
  both tables, and one of them has to be renamed;
- a name matching no declared step or artifact;
- a glob matching nothing — an empty match is an error, not an empty set;
- an artifact that requires itself, and any cycle among artifacts, reported with
  the chain (`a -> b -> c -> a`);
- `{artifact.<id>.out}` naming an artifact the consumer does not require, or one
  that builds for more than one target here and so has no single output.

A required artifact that fails to build fails its consumer, naming the requirement,
and the consumer is not attempted.

## Selection and the build matrix

A build cell is one artifact × one target × one profile.

- `mach build <path>` builds every declared artifact whose `targets` includes the
  selected target, for the default profile. `--all-targets` crosses every artifact
  with every target in its `targets`. `--bin <name>` / `--lib <name>` narrow to one
  artifact; `--target <name>` selects a declared target; `--profile <name>` selects
  a profile.
- `mach run <path>` consumes exactly one artifact. With no `--bin`/`--lib`, it selects
  one when exactly one artifact declares the resolved target; if several do, it asks
  you to pick one, naming every candidate.
- `mach test <path>` and `mach doc <path>` need one artifact as their primary
  context and select it by the same rule as everything else: `--bin`/`--lib`
  wins, a sole artifact that declares the resolved target is chosen, several
  need exactly one `default = true` (4.30.0 falls back to the first declared
  with a warning; 5.0.0 refuses). `mach test` links the union of all artifacts' referenced entries plus
  exported dependency entries, filtered to that target. Foreign-target tests require
  a compatible `--runner`. If two artifacts' objects collide on symbols in that union,
  that is an honest link error — restructure the entries.

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
`--target`). A manifest that declares several and marks none still builds in 4.x:
the first declared target is taken, with a deprecation warning, and 5.0.0 refuses
that manifest, since table order carries no meaning. The same window applies to
`[profile.*]` and to `[artifact.*]` when a command needs one artifact.

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

Mach builds itself from a manifest that declares six targets, two binary
artifacts split on the executable extension, and one dependency, `std`:

```toml
[project]
id = "mach"
version = "4.26.5"
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
opt = 0
debug = false
simd = "scalarize"

[profile.release]
opt = 2
debug = false
simd = "scalarize"

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
ref = "branch/main"
```

(The full manifest declares all six targets; `version` is whatever the tree's
current release is.) `mach build .` selects the host-matching target via
`native`, compiles `src/bin/main.mach` and its transitive imports — including
modules from `std` at `dep/std/` — and links `out/<target>/<profile>/bin/mach`.
`dep/std` is a git submodule whose gitlink is the pin; the build resolves it
purely by that directory and verifies the gitlink from the repository's index,
fetching nothing.

## See also

- [cli.md](cli.md) — the `mach` command-line reference
- [language/files.md](language/files.md) — file layout and `lib.mach` / `main.mach`
- [language/modules.md](language/modules.md) — how files map to module paths
- [language/ext-fun.md](language/ext-fun.md) — linking against external symbols
