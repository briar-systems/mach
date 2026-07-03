# `$mach.*` — compiler-owned namespace

The `$mach.*` subtree is the compiler's view of the world: the resolved build
context, the compiler identity, and source position. All reads, all comptime
constants. The tags `$mach.{os,arch,abi,mode}.*` exist for path-value
comparison against the resolved-build facts.

> **Implementation status.** The resolved-build facts (`$mach.build.{os,arch,
> abi,pointer_width,mode,pie}`), the tag tables (`$mach.{os,arch,abi,mode}.*`),
> the compiler version (`$mach.version` and `$mach.version.{major,minor,patch}`),
> and `$mach.compiler.{name,version}` are live. The `$mach.build.{timestamp,
> host}`, `$mach.build.git.*`, `$mach.project.*`, and `$mach.source.*` paths are
> reserved stubs — reading one is a compile error ("not yet available"). Each
> subtree below notes its status.

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
$mach.build.timestamp           # stub — not yet available
$mach.build.host                # stub — not yet available
$mach.build.git.commit          # stub — not yet available
$mach.build.git.dirty           # stub — not yet available
```

A bare `$mach.build.<name>` that names none of the reserved facts resolves to
the manifest comptime define of that name, or is a compile error when no such
define was declared.

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
$mach.project.name
$mach.project.version
$mach.project.root
```

> Project metadata lives at the top-level `$project.*` root
> (`$project.{id,version,name,description}` and the declared target tuple
> `$project.target.{os,arch,abi}`), fed from `[project]` / `[target.*]` in
> `mach.toml` — see [comptime.md](comptime.md). These `$mach.project.*` paths
> remain reserved stubs.

### `$mach.source.*` — current source position (stubs)

```mach
$mach.source.file
$mach.source.line
$mach.source.module
$mach.source.function
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
$mach.abi.sysv
$mach.abi.win64
$mach.abi.aapcs64
$mach.abi.lp64
$mach.mode.debug
$mach.mode.release
```

The tag lists are closed and grow only when backend support lands. An
unrecognized tag name is a compile error, never a silent fold.

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
pub val IS_LINUX: u8   = $mach.build.os == $mach.os.linux;
pub val COMPILER: *u8  = $mach.compiler.name;
```

## See also

- [comptime-control.md](comptime-control.md) — `$if` / `$or` using these
  reads
- [comptime.md](comptime.md) — the `$project.*` / `$bin.*` roots
- [val-var.md](val-var.md) — binding compiler values into runtime
  constants
