# mach.lang.me.analysis.dependence

## rec MemAccess

```mach
pub rec MemAccess;
```

## def DepVerdict

```mach
pub def DepVerdict: u8
```

## val DEP_INDEPENDENT

```mach
pub val DEP_INDEPENDENT: DepVerdict = 0
```

## val DEP_DEPENDENT

```mach
pub val DEP_DEPENDENT:   DepVerdict = 1
```

## rec DepInfo

```mach
pub rec DepInfo;
```

## fun analyze_dependence

```mach
pub fun analyze_dependence(fn: *ir.Function, la: *loops.LoopAnalysis, loop_ix: u32, types: *ir_type.IrTypeTable, alloc: *A.Allocator) res[DepInfo, fail.Fail];
```

## fun equal

```mach
pub fun equal(a: value.Value, b: value.Value) bool;
```

## fun dep_dnit

```mach
pub fun dep_dnit(d: *DepInfo);
```

## fun ivs_only_recurrence

```mach
pub fun ivs_only_recurrence(fn: *ir.Function, la: *loops.LoopAnalysis, loop_ix: u32) bool;
```

## rec ReductionInfo

```mach
pub rec ReductionInfo;
```

## fun reduction_dnit

```mach
pub fun reduction_dnit(ri: *ReductionInfo);
```

## fun analyze_reduction

```mach
pub fun analyze_reduction(fn: *ir.Function, la: *loops.LoopAnalysis, loop_ix: u32, types: *ir_type.IrTypeTable, alloc: *A.Allocator, float_reassoc: bool) res[ReductionInfo, fail.Fail];
```

