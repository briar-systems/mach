# `$mach.*` — compiler-owned namespace

The `$mach.*` subtree is the compiler's view of the world: the resolved build
context, the compiler identity, the project that owns the module, and source
position. All reads, all comptime constants. The tags `$mach.{sys,arch,abi,mode}.*`
exist for path-value comparison against the resolved-build facts.

v6 spelled `$mach.sys.*` as `$mach.os.*`, `$mach.build.sys` as `$mach.build.os`,
`$mach.build.ext.*` as `$mach.build.extensions.*` and `$project.target.sys` as
`$project.target.os`. Each previous spelling is refused naming its v7 one, which
`mach migrate` writes.

Every path is deterministic: a function of the source, the manifests and the
selected build, never of the clock, the host machine or the checkout. The same
source built at a different path, at a different time or on a different machine
reads the same values, so a module's compiled output can be cached and reused.
There is no build timestamp, host name or version-control state. A build that
wants one generates a source file from a
[build step](manifest.md#stepname--build-steps).

## Subtrees

### `$mach.build.*` — what we're building for

The resolved active build's facts. `sys`/`arch`/`abi`/`mode` share the numeric
tag space of `$mach.{sys,arch,abi,mode}.*`, so a comparison is a plain integer
compare (see [Comparison](#comparison)).

```mach fragment
$mach.build.sys                 # live; compared against $mach.sys.* tags
$mach.build.arch                # live; compared against $mach.arch.* tags
$mach.build.abi                 # live; compared against $mach.abi.* tags
$mach.build.pointer_width       # live; integer count of bytes
$mach.build.mode                # live; compared against $mach.mode.* tags
$mach.build.pie                 # live; 1 when building position-independent, else 0
$mach.build.platform            # live; the target's open platform tag as a string, "" when unset
$mach.build.ct_mul(op, width)   # live; 1 when mirl runs that multiply in constant time on the target, else 0
$mach.build.ext.<name>          # live; 1 when the target selects that instruction-set extension, else 0
```

`$mach.build.arch` answers the target's `arch` key: the manifest calls the axis
`arch` and both comptime roots call it `arch`, and the tags are the architecture
catalog's spellings. A target's `isa` never changes it: `arch = "riscv32"` on
`isa = "rv32imc"` answers `$mach.arch.riscv32`.

`$mach.build.mode` answers which pipeline the build runs, whatever the profile is
named: `$mach.mode.release` when the build runs the optimization passes (the
selected profile's `optimize = true`), and `$mach.mode.debug` when it does not. A
`-O` or `--no-optimize` on the command line overrides the profile's `optimize`
and the mode with it.

The members above are the whole subtree, and no manifest key adds one. The
`ext` members are the selected architecture's own vocabulary, not the manifest's. A
`$mach.build.<name>` that names none of them is a compile error at the use site
(`` unknown `$mach.*` path ``). A project's own configuration constants are
ordinary `val`s selected with `$if` over the facts above.

```mach error unknown `$mach.*` path
val TRACING: u64 = $mach.build.TRACING;
```

#### `$mach.build.ct_mul(op, width)` — the constant-time multiply catalog

The one call-shaped fact. It folds to 1 exactly when mirl runs that multiply on
the selected target in time independent of its operands under the timing modes
the build guarantees, the bar a `#[constant_time("hard")]` function is held to,
and to 0 otherwise. mirl reads it off its selection table and masc's rows when
the target is resolved, and mach states no table of its own, so a library can
choose its hardware or bit-serial path without keeping a per-target list:

```mach
$if ($mach.build.ct_mul(wide_u, 64) == 1) {
    # one widening multiply per limb product
}
$or {
    # the bit-serial product
}
```

- `op` is a bare word, one of:
  - `low`: the low half of a same-width product;
  - `high_u`, `high_s`, `high_su`: the high half, with unsigned, signed or mixed operands;
  - `wide_u`, `wide_s`: the full double-width product.
- `width` is the operand width in bits, a comptime integer: 8, 16, 32, 64 or 128.
- A widening cell is the low and the high half at that width, held when both
  are. A cell answers for the multiply as the build computes it, after mirl's
  legalisations: a narrow product widened to the multiply's width, a high half
  with no form of its own built from low products, a multiply wider than the
  word split into products of words, and a width with no multiply at all a
  helper's call, which is 0 (see
  [secrecy.md](secrecy.md#constant-time-multiply-by-instruction-set)). It holds
  when every row the selection emits for those multiplies is classified
  constant time under a mode the build guarantees:
  - always;
  - the extension it names, selected for the target;
  - a data-independent-timing mode the target's baseline holds.
- x86-64 answers 1 for every multiply at every width on every OS, and riscv64
  and riscv32 with Zkt selected (`rv64gc_zkt`, `rv32gc_zkt`) do as well.
  aarch64 classifies its multiplies under PSTATE.DIT, so it answers 1, for
  every multiply at every width, only where the target's baseline holds DIT,
  as darwin-arm64's does. A `soft` function sets the mode where the processor
  has it, which the query does not count on (see
  [secrecy.md](secrecy.md#soft-and-hard)). Every other target folds to 0. Lane
  multiplies are not part of the query.
- The result is a `u1`, like `$mach.build.pie`. An unknown `op` or `width`, a
  missing argument, or arguments on any other path is a compile error that names
  what is accepted.

#### `$mach.build.ext.<name>` — instruction-set extensions

One `u1` member per extension name of the selected architecture, like `$mach.build.pie`. It is
1 when the target selects that extension and 0 otherwise. A build selects extensions
with the target's `ext` key and, on riscv, its `isa`, closed over what
each one implies (see
[Instruction-set extensions](manifest.md#instruction-set-extensions)), so
`ext = ["sse41"]` answers 1 for `ssse3` too. On spirv the target's `env`
selects them.

The names are the selected architecture's vocabulary and nothing else. Each
architecture's vocabulary is listed under
[Instruction-set extensions](manifest.md#instruction-set-extensions).

A name the selected architecture does not declare is a compile error, never a silent 0, as
`$mach.arch.*` refuses an unknown architecture:

```
error[comptime.extension_unknown] `$mach.build.ext.sha` cannot be read, because `sha` names no single extension of masc's catalog for arch 'aarch64'
```

So a source that serves several architectures asks the extension question only
under an architecture guard. A comptime `&&` short-circuits as it does at run time:
when the left side is false the right side is not read, so
`$if ($mach.build.arch == $mach.arch.x86_64 && $mach.build.ext.sha == 1)` builds on
aarch64. The right side is read when the left is true, and an unknown name there is
refused. The nested form keeps one fallback per architecture:

```mach fragment
$if ($mach.build.arch == $mach.arch.x86_64) {
    $if ($mach.build.ext.sha == 1) {
        use backend: crypto.hash.sha256.x86_sha;
    }
    $or {
        use backend: crypto.hash.sha256.portable;
    }
}
$or ($mach.build.arch == $mach.arch.aarch64) {
    $if ($mach.build.ext.sha2 == 1) {
        use backend: crypto.hash.sha256.arm_sha2;
    }
    $or {
        use backend: crypto.hash.sha256.portable;
    }
}
$or {
    use backend: crypto.hash.sha256.portable;
}
```

A bare `$mach.build.ext` is refused too.

In an editor union build each target tuple answers for its own target, as
`$mach.build.sys` does.

The member answers for the whole build. A function that uses an extension the target
does not select, behind a run-time check, is marked
[`#[extension(...)]`](decorators.md#extnames--an-outlier-function) instead,
and the member stays 0 for it.

### `$mach.version` — the compiler version

```mach fragment
$mach.version                   # live; the version string, e.g. "7.0.0"
$mach.version.major             # live; integer component
$mach.version.minor             # live; integer component
$mach.version.patch             # live; integer component
```

### `$mach.compiler.*` — compiler identity

```mach fragment
$mach.compiler.name             # live
$mach.compiler.version          # live; same value as $mach.version
```

### `$mach.project.*` — the project that owns the module

```mach fragment
$mach.project.id                # the owning project's [project].id
$mach.project.version           # the owning project's [project].version string
$mach.project.version.major     # integer component
$mach.project.version.minor     # integer component
$mach.project.version.patch     # integer component
```

The project that owns the module being compiled, read from that project's own
`mach.toml`. A module of the root project reads the root's manifest. A module of
a dependency reads the dependency's, whichever project is being built. The top-level
[`$project.*`](comptime.md#compiler-owned-roots) root is the other half: it names
the project the build is for, the same in every module. In a root module the two
agree.

A library reports its own version with it, and the value stays right in every
consumer's build:

```mach
use std.types.string.str;

pub val VERSION: str = $mach.project.version;
```

`$project.version` in the same place would read each consumer's version instead.
A member the subtree does not carry is `` unknown `$mach.project.*` path ``.

### `$mach.source.*` — current source position

```mach fragment
$mach.source.file               # the module's file, relative to its project's root: "src/lib/hedge.mach"
$mach.source.line               # the 1-based line the path is written on
$mach.source.module             # the module's fully qualified name: "hedge.lib.hedge"
```

`file` is relative to the root of the project that owns the module and never an
absolute host path. It is `/`-separated on every host, Windows included, so a
checkout at another path or on another host reads the same value.
`line` is the line of the `$mach.source.line` read itself. `module` is the name a
`use` imports the module by.

```mach
val LINE: u64 = $mach.source.line;
```

A member the subtree does not carry is `` unknown `$mach.source.*` path ``.

### `$mach.sys.*`, `$mach.arch.*`, `$mach.abi.*`, `$mach.mode.*` — tag values

```mach fragment
$mach.sys.linux
$mach.sys.darwin
$mach.sys.windows
$mach.sys.freestanding           # no OS / bare metal
$mach.sys.uefi                   # a PE image a UEFI firmware loads
$mach.sys.vulkan                 # a spir-v module's system
$mach.sys.wasi
$mach.arch.x86_64
$mach.arch.aarch64
$mach.arch.riscv64
$mach.arch.riscv32
$mach.arch.spirv
$mach.arch.wasm32
$mach.abi.sysv64
$mach.abi.win64
$mach.abi.aapcs64
$mach.abi.darwinpcs
$mach.abi.aapcs64win
$mach.abi.lp64
$mach.abi.lp64f
$mach.abi.lp64d
$mach.abi.ilp32
$mach.abi.ilp32f
$mach.abi.ilp32d
$mach.abi.shader                 # a spir-v module's convention
$mach.abi.basic
$mach.mode.debug
$mach.mode.release
```

The tag names are the target registries' own spellings, read from them
directly, so there is no second list to keep in step. A tag name the registry
does not carry is a compile error, never a silent fold.

The x86-64 System V ABI is spelled `sysv64`, as the registry spells it.
`$mach.abi.sysv` is an unknown tag, as is any other name the registry does not
carry.

## Comparison

Tag comparisons are path-value, with no `.id` suffix or unwrapping. Both sides share
one numeric space, so the comparison is an ordinary integer compare:

```mach fragment
$if ($mach.build.sys == $mach.sys.linux) { ... }
$if ($mach.build.arch == $mach.arch.x86_64) { ... }
```

## Use in runtime values

A `$mach.*` read can initialize a runtime binding. The compiler folds the
RHS at compile time:

```mach
use std.types.string.str;

pub val IS_LINUX: u1  = $mach.build.sys == $mach.sys.linux;
pub val COMPILER: *u8 = $mach.compiler.name;
pub val VERSION:  str = $mach.version;
pub val MAJOR:    u64 = $mach.version.major;
pub val WIDTH:    u64 = $mach.build.pointer_width;
```

## See also

- [comptime-control.md](comptime-control.md): `$if` / `$or` using these
  reads
- [comptime.md](comptime.md): the `$project.*` / `$bin.*` roots, and which
  project each root names
- [val-var.md](val-var.md): binding compiler values into runtime
  constants
