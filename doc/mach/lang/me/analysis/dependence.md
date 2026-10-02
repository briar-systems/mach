# mach.lang.me.analysis.dependence

## rec MemAccess

```mach
pub rec MemAccess;
```

## val DEP_INDEPENDENT

```mach
pub val DEP_INDEPENDENT: DepVerdict = 0
```

## rec DepInfo

```mach
pub rec DepInfo;
```

## fun analyze_dependence

```mach
pub fun analyze_dependence(fn: *me_ir.Function, la: *loops.LoopAnalysis, loop_ix: u32, types: *ir_type.IrTypeTable, alloc: *A.Allocator) res[DepInfo, fail.Fail];
```

## fun equal

```mach
pub fun equal(a: value.Value, b: value.Value) bool;
```

## fun dep_dnit

```mach
pub fun dep_dnit(d: *DepInfo);
```

## rec ReductionInfo

```mach
pub rec ReductionInfo;
```

a loop's accumulator, `acc = acc op rv` each iteration. a predicated update
applies `op` only where the compare `pred_cmp` holds (fails, when
`pred_on_true` is false) and keeps the accumulator otherwise: `pred_join` is
the select if-conversion left, or the join phi of a diamond whose arm computes
the update, whose branch block and arms are named (an arm is BLOCK_NIL for a
direct edge into the join). `rv` is then the value the update combines in,
and the update instruction and the join are read only by each other and the
accumulator. `elem_ty` is the element the loop loads, which sets the lanes;
`acc_ty` is the accumulator's, the same type or a wider integer. `ordered`
marks a float update without reassociation, which must combine the values in
program order, one at a time

## fun reduction_dnit

```mach
pub fun reduction_dnit(ri: *ReductionInfo);
```

## fun analyze_reduction

```mach
pub fun analyze_reduction(fn: *me_ir.Function, la: *loops.LoopAnalysis, loop_ix: u32, types: *ir_type.IrTypeTable, alloc: *A.Allocator, float_reassoc: bool) res[ReductionInfo, fail.Fail];
```

