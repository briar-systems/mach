# std.filesystem.transaction.ownership

## val LOCK_LEAF

```mach
pub val LOCK_LEAF:   str = ".mach-txn-lock"
```

## val CLAIMS_LEAF

```mach
pub val CLAIMS_LEAF: str = ".machtxn.claims"
```

## rec Root

```mach
pub rec Root;
```

initialized owners stay at their original address until destruction

## rec Lock

```mach
pub rec Lock;
```

## rec Borrow

```mach
pub rec Borrow;
```

name is borrowed from the destination claim and remains valid through release

## fun root_valid

```mach
pub fun root_valid(root: *Root) bool;
```

## fun lock_valid

```mach
pub fun lock_valid(held: *Lock) bool;
```

## fun borrow_valid

```mach
pub fun borrow_valid(borrow: *Borrow) bool;
```

## fun root_init

```mach
pub fun root_init(out: *Root, fd: i32) i64;
```

takes descriptor ownership only on success

## fun root_destroy

```mach
pub fun root_destroy(root: *Root) i64;
```

## fun acquire

```mach
pub fun acquire(out: *Lock, root: *Root) i64;
```

## fun release_lock

```mach
pub fun release_lock(held: *Lock) i64;
```

## fun borrow_worker

```mach
pub fun borrow_worker(out: *Borrow, held: *Lock) i64;
```

admission happens before a worker is launched

## fun borrow_leaf

```mach
pub fun borrow_leaf(out: *Borrow, held: *Lock, name: str) i64;
```

## fun release_borrow

```mach
pub fun release_borrow(borrow: *Borrow) i64;
```

even failed claim cleanup consumes the borrow and leaves recoverable residue

## fun begin_recovery

```mach
pub fun begin_recovery(held: *Lock) i64;
```

the caller keeps the exclusive root claim throughout its recovery effects

## fun end_recovery

```mach
pub fun end_recovery(held: *Lock, admit: bool) i64;
```

## fun initialize_claims

```mach
pub fun initialize_claims(held: *Lock) err[removal.Error];
```

fresh creation inherits the root namespace instead of trusting stale directory flags

