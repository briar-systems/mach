# mach.lang.output.remove

## val MAX_DEPTH

```mach
pub val MAX_DEPTH: usize = 64
```

## rec Removal

```mach
pub rec Removal;
```

a failed removal names its kind; code is the native code behind it, 0 when
the removal refused on its own

## fun message

```mach
pub fun message(r: Removal) str;
```

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

## fun t_component

```mach
pub fun t_component(a: *A.Allocator, n: usize) str;
```

a nul-terminated component of `n` bytes, or nil

