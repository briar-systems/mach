# mach.lang.output.remove

descriptor-relative removal beneath a project root: every component is
opened without following symlinks so no part of the path is resolved as a
string and a link out of the tree is refused rather than followed

## rec Removal

```mach
pub rec Removal;
```

a failed removal names its kind; code is the native code behind it, 0 when
the removal refused on its own, and cleanup_code the first close that failed
on the way out, 0 when none did

## fun message

```mach
pub fun message(r: Removal) str;
```

## fun with_cleanup

```mach
pub fun with_cleanup(a: *A.Allocator, f: fail.Fail, r: Removal, tree: str) fail.Fail;
```

the failure `f` reported for removal `r` of `tree`, also naming the close that
failed on the way out when there was one

## fun refusal

```mach
pub fun refusal(a: *A.Allocator, k: diagnostic_kind.Kind, op: fail.Op, r: Removal) fail.Fail;
```

the failure of removing `op`'s path, which `r` refused outside containment, also
naming the close that failed on the way out

## fun open_dir

```mach
pub fun open_dir(dir: usize, name: str, out: *usize) i64;
```

open a directory entry beneath `dir` without following a symlink, writing its handle to out

## fun open_path

```mach
pub fun open_path(root: str, out: *usize) i64;
```

## fun relative

```mach
pub fun relative(root_fd: usize, rel: str) Removal;
```

remove the tree at `rel` beneath `root_fd`, walking to the parent of the
last component with a descriptor held at each step

