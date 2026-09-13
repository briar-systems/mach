# mach.lang.me.testing

## rec Attempt

```mach
pub rec Attempt;
```

## val REFUSED_AT_BASELINE

```mach
pub val REFUSED_AT_BASELINE:     i32 = 3
```

the attempt's failure codes above the fixture's own

## val SUCCEEDED_UNDER_REFUSAL

```mach
pub val SUCCEEDED_UNDER_REFUSAL: i32 = 4
```

## val WRONG_REFUSAL_TEXT

```mach
pub val WRONG_REFUSAL_TEXT:      i32 = 5
```

## val INPUT_CHANGED

```mach
pub val INPUT_CHANGED:           i32 = 6
```

## val REMAINDER_BROKEN

```mach
pub val REMAINDER_BROKEN:        i32 = 7
```

## fun attempt

```mach
pub fun attempt(a: *Attempt, p: *probe.Probe) i32;
```

## fun control

```mach
pub fun control(a: *Attempt, owner: *A.Allocator, out: *ir.Module) i32;
```

the fixture built over a page allocator, for the unchanged comparison

## fun walk

```mach
pub fun walk(a: *Attempt) i32;
```

the walk as one test outcome: the attempt's code or the walk's own, with
the failing ordinal and the request count on stderr since an exit code
cannot carry them

## fun verifies

```mach
pub fun verifies(m: *ir.Module, full: bool) bool;
```

the module passes the verifier with no target, fully (reachability included)
or structurally: the remainder a refusal leaves is still a well-formed module

## fun listed

```mach
pub fun listed(m: *ir.Module) u32;
```

how many instructions every block of every function lists

