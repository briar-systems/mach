# mach.lang.target.input

link inputs: how a link token names a file for a target, and what kind of
input that file is. each object format declares how its inputs are named and
recognizes their bytes, so a token is resolved here without knowing a format

## def Kind

```mach
pub def Kind: u8
```

## val STATIC

```mach
pub val STATIC: Kind = 0
```

## val DYNAMIC

```mach
pub val DYNAMIC: Kind = 1
```

## rec Input

```mach
pub rec Input;
```

a resolved link input: a static file the link reads, or a shared library the
loader finds by `text` (its loader name), through `rpath` when that is not empty

## fun token_is_path

```mach
pub fun token_is_path(tok: *u8) bool;
```

whether a token names a file by path rather than a library by name: it holds
a separator, or it ends the way a registered object format names an input

## fun token_resolve

```mach
pub fun token_resolve(a: *A.Allocator, tgt: *binding.Binding, dirs: *Vector[str],
root: str, tok: *u8) res[Input, fail.Fail];
```

the input a link token names for the target: a path is read as written, or
from `root` when it is relative and not found as written; a bare name is
searched for as a static input on `dirs` and the working directory, then as
a shared library on `dirs` and the system's library directories

## fun framework_resolve

```mach
pub fun framework_resolve(a: *A.Allocator, tgt: *binding.Binding, name: *u8) res[Input, fail.Fail];
```

a system framework, which the loader finds by its install path

