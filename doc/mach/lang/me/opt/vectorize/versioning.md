# mach.lang.me.opt.vectorize.versioning

loop versioning. a counted loop the vectorizer proved independent gets a
copy behind a guard: the guard checks at run time that the bound and the
start stay in the range the copy's arithmetic assumes, and that the loop's
accesses do not overlap, and enters the copy, the fast loop, only then. the
original loop is the fallback, and both rejoin at a merge block. a loop that
is not counted, reads a value after it, holds a volatile access or depends
on itself is refused before anything is changed

## val VERSION_OK

```mach
pub val VERSION_OK:           VersionStatus = 0
```

## rec VersionResult

```mach
pub rec VersionResult;
```

a versioning's outcome: on VERSION_OK the guard and merge blocks, the fast
copy's header and blocks, which the result owns, and the exclusive bound the
guard tested

## fun emit_exclusive_bound

```mach
pub fun emit_exclusive_bound(b: *builder.Builder, counted: *loops.CountedInfo) res[value.Value, fail.Fail];
```

the loop's bound as an exclusive one: an inclusive bound plus one

## fun version_loop

```mach
pub fun version_loop(m: *me_ir.Module, fn: *me_ir.Function, la: *loops.LoopAnalysis, loop_ix: u32, d: *dependence.DepInfo, alloc: *A.Allocator) res[VersionResult, fail.Fail];
```

the loop `loop_ix` of `fn` behind its guard, or the reason it was refused
with the function unchanged

## fun version_result_dnit

```mach
pub fun version_result_dnit(vr: *VersionResult, alloc: *A.Allocator);
```

releases the fast blocks a result owns

