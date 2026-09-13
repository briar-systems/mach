# std.allocator.testing

## val POISON_NEW

```mach
pub val POISON_NEW: u8 = 0xAA
```

filled into memory as it is handed out, so an uninitialized read is visible

## val POISON_FREE

```mach
pub val POISON_FREE: u8 = 0xDE
```

written over memory as it is taken back, so a use-after-free is visible

## val MAX_TRACKED

```mach
pub val MAX_TRACKED: usize = 4096
```

how many live allocations one instance can track at once

## rec Testing

```mach
pub rec Testing;
```

a tracking, guarding, failing allocator

create with `make`, hand `?state.a` to the code under test, and read the
counters afterwards. one instance is not safe to share across threads.

## fun make

```mach
pub fun make(t: *Testing) err[allocator.Error];
```

initialize a testing allocator

the instance owns the mappings it hands out, so it must outlive every
allocation made through it, and `dnit` must run while those are still
reachable.

t: the instance to initialize
ret: ok, or `invalid` when t is nil

## fun allocator_of

```mach
pub fun allocator_of(t: *Testing) *allocator.Allocator;
```

the allocator interface to hand to the code under test

## fun fail_at_ordinal

```mach
pub fun fail_at_ordinal(t: *Testing, n: usize);
```

make the `n`th allocation from now fail, counting the allocations this
instance has already served

passing zero disables failure injection.

## fun clear_failure

```mach
pub fun clear_failure(t: *Testing);
```

stop failing allocations

## fun live_count

```mach
pub fun live_count(t: *Testing) usize;
```

how many allocations are outstanding

## fun total_count

```mach
pub fun total_count(t: *Testing) usize;
```

how many allocations this instance has ever served

## fun attempt_count

```mach
pub fun attempt_count(t: *Testing) usize;
```

how many allocation requests have been made, including the refused ones

## fun leaked

```mach
pub fun leaked(t: *Testing) bool;
```

true when anything allocated through this instance was never freed

## fun double_free_count

```mach
pub fun double_free_count(t: *Testing) usize;
```

how many frees named a pointer this instance did not currently own

## fun tracking_exhausted

```mach
pub fun tracking_exhausted(t: *Testing) bool;
```

true when tracking filled up, which makes the other counters unreliable

## fun dnit

```mach
pub fun dnit(t: *Testing) usize;
```

release everything still outstanding and reset the counters

leaks are reported by `leaked` before this runs; afterwards the instance is
clean and can be reused.

ret: how many live allocations had to be reclaimed, which is the leak count

