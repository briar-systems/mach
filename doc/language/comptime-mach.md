# `$mach.*` — compiler-owned namespace

The `$mach.*` subtree is the compiler's view of the world: target details,
build context, project metadata, and source position. All reads, all
comptime constants. The tags `$mach.os.*` and `$mach.arch.*` exist for
path-value comparison.

> **Implementation status.** Only a subset resolves today. The
> `$mach.target.{os,arch,pointer_width}`, `$mach.os.*`, `$mach.arch.*`, and
> `$mach.compiler.{name,version}` reads are live. The `$mach.target.abi`,
> `$mach.build.*`, `$mach.project.*`, and `$mach.source.*` paths are
> reserved stubs — reading one is a compile error ("not yet available").
> Each subtree below notes its status.

## Subtrees

### `$mach.target.*` — what we're building for

```mach
$mach.target.os                 # live; compared against $mach.os.* tags
$mach.target.arch               # live; compared against $mach.arch.* tags
$mach.target.pointer_width      # live; integer count of bytes
$mach.target.abi                # stub — not yet available
```

### `$mach.compiler.*` — compiler identity

```mach
$mach.compiler.name             # live
$mach.compiler.version          # live
```

### `$mach.build.*` — properties of this build session (stubs)

```mach
$mach.build.mode
$mach.build.timestamp
$mach.build.git.commit
$mach.build.git.dirty
$mach.build.host
```

### `$mach.project.*` — values from mach.toml (stubs)

```mach
$mach.project.name
$mach.project.version
$mach.project.root
```

> Project metadata now lives at the top-level `$project.*` root
> (`$project.{id,version,name,description}`), fed from `[project]` in
> `mach.toml` — see [comptime.md](comptime.md). These `$mach.project.*` paths
> remain reserved stubs.

### `$mach.source.*` — current source position (stubs)

```mach
$mach.source.file
$mach.source.line
$mach.source.module
$mach.source.function
```

### `$mach.os.*` and `$mach.arch.*` — tag values

```mach
$mach.os.linux
$mach.os.darwin
$mach.os.windows
$mach.os.freestanding           # no OS / bare metal
$mach.arch.x86_64
$mach.arch.aarch64
```

The tag lists are closed and grow only when backend support lands.

## Comparison

Tag comparisons are path-value — no `.id` suffix or unwrapping:

```mach
$if ($mach.target.os == $mach.os.linux) { ... }
$if ($mach.target.arch == $mach.arch.x86_64) { ... }
```

## Use in runtime values

A `$mach.*` read can initialize a runtime binding. The compiler folds the
RHS at compile time:

```mach
pub val IS_LINUX: u8   = $mach.target.os == $mach.os.linux;
pub val COMPILER: *u8  = $mach.compiler.name;
```

## See also

- [comptime-control.md](comptime-control.md) — `$if` / `$or` using these
  reads
- [val-var.md](val-var.md) — binding compiler values into runtime
  constants
