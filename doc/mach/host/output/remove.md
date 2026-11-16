[mach](../../README.md) / [host](../README.md) / [output](README.md) / **remove**

# mach.host.output.remove

descriptor-relative removal beneath a project root: every component is
opened without following symlinks so no part of the path is resolved as a
string and a link out of the tree is refused rather than followed

<sub>source: [`src/host/output/remove.mach`](../../../../src/host/output/remove.mach) · imported by 4 modules</sub>

<sub>`io_error` is `std.io.error` · `A` is `std.allocator` · `fail` is [`mach.base.fail`](../../base/fail.md) · `diagnostic_kind` is [`mach.base.diagnostic.kind`](../../base/diagnostic/kind.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Removal`](#removal) | `rec` | a failed removal names its kind; code is the native code behind it, 0 when the removal refused on its own, and cleanup_code the first close that failed on the way out, 0 when none did |

| On `Removal` | |
| :--- | :--- |
| [`message`](#message) | |
| [`with_cleanup`](#with_cleanup) | the failure `f` reported for removal `r` of `tree`, also naming the close that failed on the way out when there was one |
| [`refusal`](#refusal) | the failure of removing `op`'s path, which `r` refused outside containment, also naming the close that failed on the way out |
| [`relative`](#relative) | remove the tree at `rel` beneath `root_fd`, walking to the parent of the last component with a descriptor held at each step |

| Functions | |
| :--- | :--- |
| [`open_dir`](#open_dir) | open a directory entry beneath `dir` without following a symlink, writing its handle to out |
| [`open_path`](#open_path) | |

---

## Removal

```mach
pub rec Removal {
    removed:      usize;
    failed:       bool;
    kind:         io_error.Kind;
    code:         i64;
    containment:  bool;
    cleanup_code: i64;
}
```

a failed removal names its kind; code is the native code behind it, 0 when
the removal refused on its own, and cleanup_code the first close that failed
on the way out, 0 when none did

### message

```mach
pub fun message(r: Removal) str
```

### with_cleanup

```mach
pub fun with_cleanup(a: *A.Allocator, f: fail.Fail, r: Removal, tree: str) fail.Fail
```

the failure `f` reported for removal `r` of `tree`, also naming the close that
failed on the way out when there was one

### refusal

```mach
pub fun refusal(a: *A.Allocator, k: diagnostic_kind.Kind, op: fail.Op, r: Removal) fail.Fail
```

the failure of removing `op`'s path, which `r` refused outside containment, also
naming the close that failed on the way out

### relative

```mach
pub fun relative(root_fd: usize, rel: str) Removal
```

remove the tree at `rel` beneath `root_fd`, walking to the parent of the
last component with a descriptor held at each step

## Functions

### open_dir

```mach
pub fun open_dir(dir: usize, name: str, out: *usize) i64
```

open a directory entry beneath `dir` without following a symlink, writing its handle to out

### open_path

```mach
pub fun open_path(root: str, out: *usize) i64
```

