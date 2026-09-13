# `$mach.*` — compiler-owned namespace

The `$mach.*` subtree is the compiler's view of the world: the resolved build
context, the compiler identity, and source position. All reads, all comptime
constants. The tags `$mach.{os,arch,abi,mode}.*` exist for path-value
comparison against the resolved-build facts.

> **Live and reserved paths.** The resolved-build facts (`$mach.build.{os,arch,
> abi,pointer_width,mode,pie,platform}`), the tag tables (`$mach.{os,arch,abi,mode}.*`),
> the compiler version (`$mach.version` and `$mach.version.{major,minor,patch}`),
> and `$mach.compiler.{name,version}` are live. The `$mach.build.{timestamp,
> host}`, `$mach.build.git.*`, `$mach.project.*`, and `$mach.source.*` paths are
> reserved: the spelling is held for a later release and reading one is a
> compile error naming the subtree (`` `$mach.source.*` is not yet available ``).
> Each subtree below notes which it is.

## Subtrees

### `$mach.build.*` — what we're building for

The resolved active build's facts. `os`/`arch`/`abi`/`mode` share the numeric
tag space of `$mach.{os,arch,abi,mode}.*`, so a comparison is a plain integer
compare (see [Comparison](#comparison)).

```mach
$mach.build.os                  # live; compared against $mach.os.* tags
$mach.build.arch                # live; compared against $mach.arch.* tags
$mach.build.abi                 # live; compared against $mach.abi.* tags
$mach.build.pointer_width       # live; integer count of bytes
$mach.build.mode                # live; compared against $mach.mode.* tags
$mach.build.pie                 # live; 1 when building position-independent, else 0
$mach.build.platform            # live; the target's open platform tag as a string, "" when unset
$mach.build.timestamp           # stub — not yet available
$mach.build.host                # stub — not yet available
$mach.build.git.commit          # stub — not yet available
$mach.build.git.dirty           # stub — not yet available
```

The members above are the whole subtree, and no manifest key adds one. A
`$mach.build.<name>` that names none of them is a compile error at the use site
(`` unknown `$mach.*` path ``). A project's own configuration constants are
ordinary `val`s selected with `$if` over the facts above.

```mach
val TRACING: u64 = $mach.build.TRACING;
```

### `$mach.version` — the compiler version

```mach
$mach.version                   # live; the version string, e.g. "2.0.0"
$mach.version.major             # live; integer component
$mach.version.minor             # live; integer component
$mach.version.patch             # live; integer component
```

### `$mach.compiler.*` — compiler identity

```mach
$mach.compiler.name             # live
$mach.compiler.version          # live; same value as $mach.version
```

### `$mach.project.*` — values from mach.toml (stubs)

```mach
val root: u64 = $mach.project.root;
```

> Project metadata lives at the top-level `$project.*` root
> (`$project.{id,version}` and the declared target tuple
> `$project.target.{os,arch,abi}`), fed from `[project]` / `[target.*]` in
> `mach.toml` — see [comptime.md](comptime.md). These `$mach.project.*` paths
> remain reserved stubs.

### `$mach.source.*` — current source position (stubs)

```mach
val line: u64 = $mach.source.line;
```

### `$mach.os.*`, `$mach.arch.*`, `$mach.abi.*`, `$mach.mode.*` — tag values

```mach
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

The 4.30 alias `$mach.abi.sysv` was removed in 5.0.0. It is refused by name
(`` `$mach.abi.sysv` was removed in 5.0.0; the registry spells this ABI
`sysv64` ``) rather than as an unknown tag, so write `$mach.abi.sysv64`.
`$mach.arch.mos6502` went with the deleted MOS 6502 target: the registry no
longer carries the spelling, so it is an unknown tag.

## Comparison

Tag comparisons are path-value — no `.id` suffix or unwrapping. Both sides share
one numeric space, so the comparison is an ordinary integer compare:

```mach
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
