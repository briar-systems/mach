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

## fun writer_homed

```mach
pub fun writer_homed[W](itn: *intern.Interner, a: *A.Allocator, home: *Home, path: str, ctx: *W,
write_cb: fun(*W, *mwriter.Writer) err[mwriter.WriteError], mode: i32,
pre: Precondition, parents: Parents, op: str, generic: str) err[outcome.Fail];
```

`writer` with the destination rooted through `home` when it lies beneath
the home's path, so its control files land under the build output

## fun subtree

```mach
pub fun subtree(a: *A.Allocator, destination: *txn.Claim, inv: *txn.Inventory, dir_mode: i32) err[outcome.Fail];
```

## val HOME_OUTPUT

```mach
pub val HOME_OUTPUT: str = "out"
```

the control home of a project: `out/.mach-txn` under its root, created on
demand, so a publication into the source tree leaves nothing beside the
sources. a root opened with this home derives children homed by their
relative path, one claims directory per destination

## val HOME_LEAF

```mach
pub val HOME_LEAF:   str = ".mach-txn"
```

## fun control_home

```mach
pub fun control_home(home: *txn.Root, project: *txn.Root) err[txn.Error];
```

## fun project_root

```mach
pub fun project_root(out: *txn.Root, home: *txn.Root, base: str, rel: str) err[txn.Error];
```

open a project root homed at its control home: `home` outlives `out` and
the caller releases both, `out` first

## fun project_home

```mach
pub fun project_home(out: *Home, a: *A.Allocator, base: str) err[txn.Error];
```

open a project's root and control home for a plan: `out.path` is the
cleaned base every planned destination is compared against

## fun project_home_dnit

```mach
pub fun project_home_dnit(home: *Home, a: *A.Allocator) err[txn.Error];
```

## fun ephemeral_home

```mach
pub fun ephemeral_home(out: *txn.Root, a: *A.Allocator) res[str, txn.Error];
```

a private control home under the temporary directory for a publication
whose project does not exist yet: the destination directory receives no
sentinel, and exclusion against a concurrent creator rests on the one
atomic rename. the caller removes it with `ephemeral_home_dnit`

## fun ephemeral_home_dnit

```mach
pub fun ephemeral_home_dnit(a: *A.Allocator, home: *txn.Root, dir: str) err[txn.Error];
```

release an ephemeral home and remove its directory; `home` must have no
live lock

