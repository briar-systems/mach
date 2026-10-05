# mach.lang.me.opt.vectorize.versioning

## val VERSION_OK

```mach
pub val VERSION_OK:           VersionStatus = 0
```

## rec VersionResult

```mach
pub rec VersionResult;
```

## fun emit_exclusive_bound

```mach
pub fun emit_exclusive_bound(b: *builder.Builder, counted: *loops.CountedInfo) res[value.Value, fail.Fail];
```

## fun version_loop

```mach
pub fun version_loop(m: *me_ir.Module, fn: *me_ir.Function, la: *loops.LoopAnalysis, loop_ix: u32, d: *dependence.DepInfo, alloc: *A.Allocator) res[VersionResult, fail.Fail];
```

## fun version_result_dnit

```mach
pub fun version_result_dnit(vr: *VersionResult, alloc: *A.Allocator);
```

