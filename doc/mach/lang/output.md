# mach.lang.output

## val DIR_MODE

```mach
pub val DIR_MODE: i32 = 0o755
```

## fun intern_message

```mach
pub fun intern_message(itn: *intern.Interner, a: *A.Allocator, buf: *u8, total: usize, generic: str) str;
```

## fun io_message

```mach
pub fun io_message(itn: *intern.Interner, a: *A.Allocator, op: str, path: str, os_error: str, generic: str) str;
```

## fun bytes

```mach
pub fun bytes(itn: *intern.Interner, a: *A.Allocator, path: str, buf: *u8, len: usize, mode: i32,
create_parents: bool, op: str, generic: str) err[outcome.Fail];
```

write `buf` to `path`, creating missing parent directories when `create_parents`

## fun writer

```mach
pub fun writer[W](itn: *intern.Interner, a: *A.Allocator, path: str, ctx: *W,
write_cb: fun(*W, *mwriter.Writer) err[mwriter.WriteError], mode: i32,
create_parents: bool, op: str, generic: str) err[outcome.Fail];
```

stream `path` through `write_cb`, creating missing parent directories when `create_parents`

## fun replace

```mach
pub fun replace(a: *A.Allocator, path: str, buf: *u8, len: usize, mode: i32) err[outcome.Fail];
```

replace a source-tree file through a sibling temporary and rename, so the
destination is either the old content or the complete new content

