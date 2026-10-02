# mach.lang.me.transform.versioning

## val VERSION_OK

```mach
pub val VERSION_OK:           VersionStatus = 0
```

## rec VersionResult

```mach
pub rec VersionResult;
```

## val NONE_U32

```mach
pub val NONE_U32: u32 = 0xFFFFFFFF
```

## fun emit_exclusive_bound

```mach
pub fun emit_exclusive_bound(b: *builder.Builder, counted: *loops.CountedInfo) res[value.Value, fail.Fail];
```

## fun version_loop

```mach
pub fun version_loop(m: *me_ir.Module, fn: *me_ir.Function, la: *loops.LoopAnalysis, loop_ix: u32, d: *dependence.DepInfo) res[VersionResult, fail.Fail];
```

## fun region_index

```mach
pub fun region_index(blocks: *ir_id.BlockId, len: u32, blk: ir_id.BlockId) u32;
```

## fun clone_region

```mach
pub fun clone_region(m: *me_ir.Module, fn: *me_ir.Function, body: *ir_id.BlockId, body_len: u32, out_clones: *ir_id.BlockId) err[fail.Fail];
```

## fun version_result_dnit

```mach
pub fun version_result_dnit(vr: *VersionResult, alloc: *A.Allocator);
```

## fun redirect_terminator

```mach
pub fun redirect_terminator(fn: *me_ir.Function, blk: ir_id.BlockId, from: ir_id.BlockId, to: ir_id.BlockId);
```

## fun relabel_phi_pred

```mach
pub fun relabel_phi_pred(fn: *me_ir.Function, blk: ir_id.BlockId, from: ir_id.BlockId, to: ir_id.BlockId);
```

## fun operand_is_block

```mach
pub fun operand_is_block(k: ir_instruction.InstrKind, o: u32) bool;
```

