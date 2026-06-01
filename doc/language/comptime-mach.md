# `$mach.*` — compiler-owned namespace

The `$mach.*` subtree is the compiler's view of the world: target details,
build context, project metadata, and source position. All reads, all
comptime constants. The tags `$mach.os.*` and `$mach.arch.*` exist for
path-value comparison.

## Subtrees

### `$mach.target.*` — what we're building for

```mach
$mach.target.os                 # compared against $mach.os.* tags
$mach.target.arch               # compared against $mach.arch.* tags
$mach.target.abi
$mach.target.pointer_width      # integer count of bytes
```

### `$mach.build.*` — properties of this build session

```mach
$mach.build.mode
$mach.build.timestamp
$mach.build.git.commit
$mach.build.git.dirty
$mach.build.host
```

### `$mach.project.*` — values from mach.toml

```mach
$mach.project.name
$mach.project.version
$mach.project.root
```

### `$mach.source.*` — current source position

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
pub val IS_LINUX: u8  = $mach.target.os == $mach.os.linux;
pub val VERSION:  *u8  = $mach.project.version;
```

## See also

- [comptime-control.md](comptime-control.md) — `$if` / `$or` using these
  reads
- [val-var.md](val-var.md) — binding compiler values into runtime
  constants
