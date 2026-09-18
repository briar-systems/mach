# `$mach.*` — compiler-owned namespace

The `$mach.*` subtree is the compiler's view of the world: the resolved build
context, the compiler identity, and source position. All reads, all comptime
constants. The tags `$mach.{os,arch,abi,mode}.*` exist for path-value
comparison against the resolved-build facts.

> **Live and reserved paths.** The resolved-build facts (`$mach.build.{os,arch,
> abi,pointer_width,mode,pie,platform}`, `$mach.build.extensions.<name>` and the
> `$mach.build.ct_mul(op, width)` query), the tag tables (`$mach.{os,arch,abi,mode}.*`),
> the compiler version (`$mach.version` and `$mach.version.{major,minor,patch}`),
> and `$mach.compiler.{name,version}` are live. The `$mach.build.{timestamp,
> host}`, `$mach.build.git.*`, `$mach.project.*`, and `$mach.source.*` paths are
> reserved: the spelling is held for a later release (#3590) and reading one is
> a compile error naming the subtree (`` `$mach.source.*` is not yet available ``).
> Each subtree below notes which it is.

## Subtrees

### `$mach.build.*` — what we're building for

The resolved active build's facts. `os`/`arch`/`abi`/`mode` share the numeric
tag space of `$mach.{os,arch,abi,mode}.*`, so a comparison is a plain integer
compare (see [Comparison](#comparison)).

```mach fragment
$mach.build.os                  # live; compared against $mach.os.* tags
$mach.build.arch                # live; compared against $mach.arch.* tags
$mach.build.abi                 # live; compared against $mach.abi.* tags
$mach.build.pointer_width       # live; integer count of bytes
$mach.build.mode                # live; compared against $mach.mode.* tags
$mach.build.pie                 # live; 1 when building position-independent, else 0
$mach.build.platform            # live; the target's open platform tag as a string, "" when unset
$mach.build.ct_mul(op, width)   # live; 1 when a secret multiply of that cell is admitted, else 0
$mach.build.extensions.<name>   # live; 1 when the target selects that instruction-set extension, else 0
$mach.build.timestamp           # stub — not yet available
$mach.build.host                # stub — not yet available
$mach.build.git.commit          # stub — not yet available
$mach.build.git.dirty           # stub — not yet available
```

The members above are the whole subtree, and no manifest key adds one. The
`extensions` members are the selected isa's own vocabulary, not the manifest's. A
`$mach.build.<name>` that names none of them is a compile error at the use site
(`` unknown `$mach.*` path ``). A project's own configuration constants are
ordinary `val`s selected with `$if` over the facts above.

```mach error unknown `$mach.*` path
val TRACING: u64 = $mach.build.TRACING;
```

#### `$mach.build.ct_mul(op, width)` — the constant-time multiply catalog

The one call-shaped fact. It folds to 1 exactly when the selected target admits
a secret-operand multiply of that cell, and to 0 otherwise. The answer comes from
the same decision the lowering gate and the `#[oblivious]` validators make, so a
library can choose its hardware or bit-serial path without keeping a per-target
list:

```mach
$if ($mach.build.ct_mul(wide_u, 64) == 1) {
    # one widening multiply per limb product
} $or {
    # the bit-serial product
}
```

- `op` is a bare word, one of:
  - `low`: the low half of a same-width product;
  - `high_u`, `high_s`, `high_su`: the high half, with unsigned, signed or mixed operands;
  - `wide_u`, `wide_s`: the full double-width product.
- `width` is the operand width in bits, a comptime integer: 8, 16, 32 or 64.
- A cell is admitted when the target declares it and the row's condition holds:
  - an always-safe instruction;
  - the extension it names, selected for the target;
  - a data-independent-timing mode the target guarantees.
- No target declares a cell yet, so the query folds to 0 everywhere. Lane
  multiplies are not part of the query.
- The result is a `u8`, like `$mach.build.pie`. An unknown `op` or `width`, a
  missing argument, or arguments on any other path is a compile error that names
  what is accepted.

#### `$mach.build.extensions.<name>` — instruction-set extensions

One `u8` member per extension name of the selected isa, like `$mach.build.pie`. It is
1 when the target selects that extension and 0 otherwise. A build selects extensions
with the target's `extensions` key and, on riscv, its isa string, closed over what
each one implies (see
[Instruction-set extensions](manifest.md#instruction-set-extensions)), so
`extensions = ["sse41"]` answers 1 for `ssse3` too.

The names are the selected isa's vocabulary and nothing else:

- `x86_64`: `ssse3`, `sse41`, `sha`, `fsgsbase`;
- `aarch64`: `sha2`;
- `riscv64` and `riscv32`: `i`, `m`, `a`, `f`, `d`, `c`, `zicsr`, `zifencei`.

A name the selected isa does not declare is a compile error, never a silent 0, as
`$mach.arch.*` refuses an unknown architecture:

```
`$mach.build.extensions.sha`: `sha` is not an extension of isa 'aarch64'; its
extensions are: sha2
```

So a source that serves several isas nests the extension question under an
architecture guard. `&&` does not stand in for the nesting: every `$mach` path in a
condition is checked on its own, so
`$if ($mach.build.arch == $mach.arch.x86_64 && $mach.build.extensions.sha == 1)` is
refused on aarch64 although the left side is false. Write the nested form:

```mach fragment
$if ($mach.build.arch == $mach.arch.x86_64) {
    $if ($mach.build.extensions.sha == 1) {
        use backend: std.crypto.hash.sha256.x86_sha;
    }
    $or {
        use backend: std.crypto.hash.sha256.portable;
    }
}
$or ($mach.build.arch == $mach.arch.aarch64) {
    $if ($mach.build.extensions.sha2 == 1) {
        use backend: std.crypto.hash.sha256.arm_sha2;
    }
    $or {
        use backend: std.crypto.hash.sha256.portable;
    }
}
$or {
    use backend: std.crypto.hash.sha256.portable;
}
```

A bare `$mach.build.extensions` is refused too.

In an editor union build each target tuple answers for its own target, as
`$mach.build.os` does.

The member answers for the whole build. A function that uses an extension the target
does not select, behind a run-time check, is marked
[`#[extensions(...)]`](decorators.md#extensionsnames--an-outlier-function) instead,
and the member stays 0 for it.

### `$mach.version` — the compiler version

```mach fragment
$mach.version                   # live; the version string, e.g. "2.0.0"
$mach.version.major             # live; integer component
$mach.version.minor             # live; integer component
$mach.version.patch             # live; integer component
```

### `$mach.compiler.*` — compiler identity

```mach fragment
$mach.compiler.name             # live
$mach.compiler.version          # live; same value as $mach.version
```

### `$mach.project.*` — values from mach.toml (stubs)

```mach error `$mach.project.*` is not yet available
val root: u64 = $mach.project.root;
```

> Project metadata lives at the top-level `$project.*` root
> (`$project.{id,version}` and the declared target tuple
> `$project.target.{os,arch,abi}`), fed from `[project]` / `[target.*]` in
> `mach.toml` — see [comptime.md](comptime.md). These `$mach.project.*` paths
> remain reserved stubs.

### `$mach.source.*` — current source position (stubs)

```mach error `$mach.source.*` is not yet available
val line: u64 = $mach.source.line;
```

### `$mach.os.*`, `$mach.arch.*`, `$mach.abi.*`, `$mach.mode.*` — tag values

```mach fragment
$mach.os.linux
$mach.os.darwin
$mach.os.windows
$mach.os.freestanding           # no OS / bare metal
$mach.arch.x86_64
$mach.arch.aarch64
$mach.arch.riscv64
$mach.arch.riscv32
$mach.arch.spirv
$mach.abi.sysv64
$mach.abi.win64
$mach.abi.aapcs64
$mach.abi.lp64
$mach.abi.lp64f
$mach.abi.lp64d
$mach.abi.ilp32
$mach.abi.ilp32f
$mach.abi.ilp32d
$mach.mode.debug
$mach.mode.release
```

The tag names are the target registries' own spellings, read from them
directly; there is no second list to keep in step. A tag name the registry
does not carry is a compile error, never a silent fold.

The x86-64 System V ABI is spelled `sysv64`, as the registry spells it;
`$mach.abi.sysv` is an unknown tag, as is any other name the registry does not
carry.

## Comparison

Tag comparisons are path-value — no `.id` suffix or unwrapping. Both sides share
one numeric space, so the comparison is an ordinary integer compare:

```mach fragment
$if ($mach.build.os == $mach.os.linux) { ... }
$if ($mach.build.arch == $mach.arch.x86_64) { ... }
```

## Use in runtime values

A `$mach.*` read can initialize a runtime binding. The compiler folds the
RHS at compile time:

```mach
use std.types.string.str;

pub val IS_LINUX: u8   = $mach.build.os == $mach.os.linux;
pub val COMPILER: *u8  = $mach.compiler.name;
pub val VERSION:  str  = $mach.version;
pub val MAJOR:    u64  = $mach.version.major;
pub val WIDTH:    u64  = $mach.build.pointer_width;
```

## See also

- [comptime-control.md](comptime-control.md) — `$if` / `$or` using these
  reads
- [comptime.md](comptime.md) — the `$project.*` / `$bin.*` roots
- [val-var.md](val-var.md) — binding compiler values into runtime
  constants
