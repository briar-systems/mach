# mach.lang.publication

## def Precondition

```mach
pub def Precondition: u8
```

## val REPLACE

```mach
pub val REPLACE: Precondition = 0
```

## val CREATE

```mach
pub val CREATE:  Precondition = 1
```

## def Parents

```mach
pub def Parents: u8
```

## val PARENTS_EXIST

```mach
pub val PARENTS_EXIST:  Parents = 0
```

## val PARENTS_CREATE

```mach
pub val PARENTS_CREATE: Parents = 1
```

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

## fun replace_source

```mach
pub fun replace_source(a: *A.Allocator, destination: *txn.Claim, source_fd: *i32,
identity: txn.Identity, bytes: *u8, len: usize, mode: i32,
exact_mode: bool) err[txn.Error];
```

replace a source object that the caller holds open: `identity` was observed
on `source_fd` before its content was read, and the commit is refused when
the destination is no longer that object, so an edit never lands on a file
that changed underneath the read. `source_fd` stays the caller's on a
prepare or identity refusal and is consumed otherwise, closed before the
rename. `exact_mode` publishes `mode` as the final permission bits; false
keeps the platform's creation policy

## fun bytes_at_claim

```mach
pub fun bytes_at_claim(a: *A.Allocator, destination: *txn.Claim, buf: *u8, len: usize,
mode: i32, pre: Precondition, digest_out: *[32]u8) err[txn.Error];
```

## fun bytes_at

```mach
pub fun bytes_at(itn: *intern.Interner, a: *A.Allocator, destination: *txn.Claim, path: str,
buf: *u8, len: usize, mode: i32, pre: Precondition, op: str, generic: str,
digest_out: *[32]u8) err[outcome.Fail];
```

## fun bytes

```mach
pub fun bytes(itn: *intern.Interner, a: *A.Allocator, path: str, buf: *u8, len: usize, mode: i32,
pre: Precondition, parents: Parents, op: str, generic: str,
digest_out: *[32]u8) err[outcome.Fail];
```

## fun writer_at

```mach
pub fun writer_at[W](itn: *intern.Interner, a: *A.Allocator, destination: *txn.Claim, path: str, ctx: *W,
write_cb: fun(*W, *mwriter.Writer) err[mwriter.WriteError], mode: i32,
pre: Precondition, op: str, generic: str) err[outcome.Fail];
```

## fun writer

```mach
pub fun writer[W](itn: *intern.Interner, a: *A.Allocator, path: str, ctx: *W,
write_cb: fun(*W, *mwriter.Writer) err[mwriter.WriteError], mode: i32,
pre: Precondition, parents: Parents, op: str, generic: str) err[outcome.Fail];
```

## fun subtree

```mach
pub fun subtree(a: *A.Allocator, destination: *txn.Claim, inv: *txn.Inventory, dir_mode: i32) err[outcome.Fail];
```

