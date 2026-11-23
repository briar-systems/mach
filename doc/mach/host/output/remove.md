[mach](../../README.md) / [host](../README.md) / [output](README.md) / **remove**

# mach.host.output.remove

descriptor-relative removal beneath a project root: every component is
opened without following symlinks so no part of the path is resolved as a
string and a link out of the tree is refused rather than followed

Source: [`src/host/output/remove.mach`](../../../../src/host/output/remove.mach)

Uses types from [`mach.base.diagnostic.kind`](../../base/diagnostic/kind.md), [`mach.base.fail`](../../base/fail.md).

| Type | | |
| :--- | :--- | :--- |
| [`Removal`](#removal) | `rec` | a failed removal names its kind; code is the native code behind it, 0 when the removal refused on its own, and cleanup_code the first close that failed on the way out, 0 when none did |

| On `Removal` | |
| :--- | :--- |
| [`message`](#message) | the text that says why removal `r` failed |
| [`with_cleanup`](#with_cleanup) | the failure `f` reported for removal `r` of `tree`, also naming the close that failed on the way out when there was one |
| [`refusal`](#refusal) | the failure of removing `op`'s path, which `r` refused outside containment, also naming the close that failed on the way out |
| [`relative`](#relative) | remove the tree at `rel` beneath `root_fd`, walking to the parent of the last component with a descriptor held at each step |

| Functions | |
| :--- | :--- |
| [`open_dir`](#open_dir) | open a directory entry beneath `dir` without following a symlink, writing its handle to out |
| [`open_path`](#open_path) | open the directory at `root`, read relative to the working directory, writing its handle to out |

---

## Removal

```mach
pub rec Removal {
    removed:      usize;
    failed:       bool;
    kind:         std.io.error.Kind;
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

the text that says why removal `r` failed

| Parameter | |
| :--- | :--- |
| `r` | a failed removal |
| **returns** | the containment reason, the native message for a native code, or the name of its kind |

### with_cleanup

```mach
pub fun with_cleanup(a: *std.allocator.Allocator, f: mach.base.fail.Fail, r: Removal, tree: str) mach.base.fail.Fail
```

the failure `f` reported for removal `r` of `tree`, also naming the close that
failed on the way out when there was one

### refusal

```mach
pub fun refusal(a: *std.allocator.Allocator, k: mach.base.diagnostic.kind.Kind, op: mach.base.fail.Op, r: Removal) mach.base.fail.Fail
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

open the directory at `root`, read relative to the working directory, writing its handle to out

| Parameter | |
| :--- | :--- |
| `root` | path of the directory |
| `out` | receives the handle on success |
| **returns** | 0 on success, otherwise the negative native error code |

