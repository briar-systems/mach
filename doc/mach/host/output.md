[mach](../README.md) / [host](README.md) / **output**

# mach.host.output

output: how a compiler-controlled file reaches its final name. a build
product under `out/` is regenerable by the next build or `mach clean`, so
it is written in place with no staging, lock or journal, except an `obj/`
object: the object cache reads it back, so it goes through
`through_temporary`. a file in the source tree goes through `replace`. both
write a sibling temporary and rename it over the destination, so an
interrupted write leaves the original intact. the build owns its output
layout: `directory` and `reserve` replace a stale entry of the wrong kind,
and every failure names the path and the operation through `fail.refusal`

Source: [`src/host/output.mach`](../../../src/host/output.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.base.intern`](../base/intern.md).

| Type | | |
| :--- | :--- | :--- |
| [`Temporary`](#temporary) | `rec` | the sibling temporary a `through_temporary` write fills, and the destination a refused write of it is named by |

| On `Temporary` | |
| :--- | :--- |
| [`fill`](#fill) | write `buf` to the temporary `t`, a refusal naming the destination and the temporary it was written through |

| Functions | |
| :--- | :--- |
| [`directory`](#directory) | the build owns the layout beneath its output root: `dir` and each missing ancestor become directories, and beneath `root` an entry where one belongs that does not resolve to a directory (a stale file, a dangling link) is removed first, a link as the link and never its target |
| [`reserve`](#reserve) | ready `file` to be written: its parent is a directory as `directory` makes it, and beneath `root` a stale directory at its own name is removed |
| [`bytes`](#bytes) | write `buf` to `path`, whose parent exists, a refusal naming it as `role`, nil for a product its path names |
| [`writer`](#writer) | stream `path`, whose parent exists, through `write_cb`, staged in `a` so a writer of many small pieces makes few writes to the file; a refusal names it as `role`, nil for a product its path names |
| [`replace`](#replace) | replace a source-tree file through a sibling temporary and rename, so the destination is either the old content or the complete new content |
| [`mirror_fqn`](#mirror_fqn) | the file a module's output takes under `base`: its dotted name as a path, with `suffix` as the extension |
| [`read_back`](#read_back) | read the whole of a file the build wrote, a refusal naming `path` |
| [`through_temporary`](#through_temporary) | `write` fills a sibling temporary of `destination` through `fill`, which is then renamed over it: a reader sees the previous file or the complete new one, never a torn one, and a failed write leaves the previous file and no temporary |

---

## Temporary

```mach
pub rec Temporary {
    a:           *std.allocator.Allocator;
    path:        str;
    destination: str;
}
```

the sibling temporary a `through_temporary` write fills, and the destination a
refused write of it is named by

| Field | Type | |
| :--- | :--- | :--- |
| `a` | `*std.allocator.Allocator` | owns the text of a refusal |
| `path` | `str` | the temporary |
| `destination` | `str` | the file the temporary is renamed over |

### fill

```mach
pub fun fill(t: *Temporary, buf: *u8, len: usize, mode: i32) err[mach.base.fail.Fail]
```

write `buf` to the temporary `t`, a refusal naming the destination and the temporary it was
written through

## Functions

### directory

```mach
pub fun directory(a: *std.allocator.Allocator, root: str, dir: str) err[mach.base.fail.Fail]
```

the build owns the layout beneath its output root: `dir` and each missing
ancestor become directories, and beneath `root` an entry where one belongs
that does not resolve to a directory (a stale file, a dangling link) is
removed first, a link as the link and never its target. `root` itself and
what lies above it or outside it are created but never replaced, and a
symlink to a directory is used as it stands

### reserve

```mach
pub fun reserve(a: *std.allocator.Allocator, root: str, file: str) err[mach.base.fail.Fail]
```

ready `file` to be written: its parent is a directory as `directory` makes
it, and beneath `root` a stale directory at its own name is removed. a
symlink there is left for the write to follow

### bytes

```mach
pub fun bytes(itn: *mach.base.intern.Interner, a: *std.allocator.Allocator, path: str, buf: *u8, len: usize, mode: i32, role: str) err[mach.base.fail.Fail]
```

write `buf` to `path`, whose parent exists, a refusal naming it as `role`, nil
for a product its path names

### writer

```mach
pub fun writer[W](itn: *mach.base.intern.Interner, a: *std.allocator.Allocator, path: str, ctx: *W,
    write_cb: fun(*W, *std.io.writer.Writer) err[std.io.writer.WriteError], mode: i32, role: str) err[mach.base.fail.Fail]
```

stream `path`, whose parent exists, through `write_cb`, staged in `a` so a writer of many small
pieces makes few writes to the file; a refusal names it as `role`, nil for a product its path
names

### replace

```mach
pub fun replace(a: *std.allocator.Allocator, path: str, buf: *u8, len: usize, mode: i32) err[mach.base.fail.Fail]
```

replace a source-tree file through a sibling temporary and rename, so the
destination is either the old content or the complete new content

### mirror_fqn

```mach
pub fun mirror_fqn(a: *std.allocator.Allocator, base: *u8, fqn: str, suffix: str) res[str, mach.base.fail.Fail]
```

the file a module's output takes under `base`: its dotted name as a path,
with `suffix` as the extension

### read_back

```mach
pub fun read_back(a: *std.allocator.Allocator, path: str) res[Vector[u8], mach.base.fail.Fail]
```

read the whole of a file the build wrote, a refusal naming `path`

### through_temporary

```mach
pub fun through_temporary[C](a: *std.allocator.Allocator, destination: str, ctx: *C,
    write: fun(*C, *Temporary) err[mach.base.fail.Fail]) err[mach.base.fail.Fail]
```

`write` fills a sibling temporary of `destination` through `fill`, which is
then renamed over it: a reader sees the previous file or the complete new
one, never a torn one, and a failed write leaves the previous file and no
temporary

