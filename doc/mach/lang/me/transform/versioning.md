# mach.lang.me.transform.versioning

## def VersionStatus

```mach
pub def VersionStatus: u8
```

## val VERSION_OK

```mach
pub val VERSION_OK:           VersionStatus = 0
```

## val VERSION_NOT_COUNTED

```mach
pub val VERSION_NOT_COUNTED:  VersionStatus = 1
```

## val VERSION_ILL_FORMED

```mach
pub val VERSION_ILL_FORMED:   VersionStatus = 2
```

## val VERSION_DEPENDENT

```mach
pub val VERSION_DEPENDENT:    VersionStatus = 3
```

## val VERSION_LIVE_OUT

```mach
pub val VERSION_LIVE_OUT:     VersionStatus = 4
```

## val VERSION_VOLATILE

```mach
pub val VERSION_VOLATILE:     VersionStatus = 5
```

## val VERSION_UNSAFE_BOUND

```mach
pub val VERSION_UNSAFE_BOUND: VersionStatus = 6
```

## rec VersionResult

```mach
pub rec VersionResult;
```

## val NONE_U32

```mach
pub val NONE_U32: u32 = 0xFFFFFFFF
```

## fun emit_alias_guard

```mach
pub fun emit_alias_guard(b: *builder.Builder, d: *dep.DepInfo, init: value.Value, bound_ex: value.Value) res[value.Value, fail.Fail];
```

## fun emit_exclusive_bound

```mach
pub fun emit_exclusive_bound(b: *builder.Builder, counted: *loops.CountedInfo) res[value.Value, fail.Fail];
```

## fun pred_is_unsigned

```mach
pub fun pred_is_unsigned(pred: instruction.InstrKind) bool;
```

## fun magnitude_safe

```mach
pub fun magnitude_safe(b: *builder.Builder, v: value.Value, is_unsigned: bool) res[value.Value, fail.Fail];
```

## fun offset_safe

```mach
pub fun offset_safe(b: *builder.Builder, off: value.Value) res[value.Value, fail.Fail];
```

## fun const_magnitude_safe

```mach
pub fun const_magnitude_safe(v: value.Value, is_unsigned: bool) opt[bool];
```

## fun const_offset_safe

```mach
pub fun const_offset_safe(off: value.Value) opt[bool];
```

## fun version_loop

```mach
pub fun version_loop(m: *ir.Module, fn: *ir.Function, la: *loops.LoopAnalysis, loop_ix: u32, d: *dep.DepInfo) res[VersionResult, fail.Fail];
```

## fun region_index

```mach
pub fun region_index(blocks: *id.BlockId, len: u32, blk: id.BlockId) u32;
```

## fun clone_region

```mach
pub fun clone_region(m: *ir.Module, fn: *ir.Function, body: *id.BlockId, body_len: u32, out_clones: *id.BlockId) err[fail.Fail];
```

## fun version_result_dnit

```mach
pub fun version_result_dnit(vr: *VersionResult, alloc: *A.Allocator);
```

## fun clone_one

```mach
pub fun clone_one(m: *ir.Module, fn: *ir.Function, src_id: id.InstructionId, instr_map: *u32, map_len: u32) err[fail.Fail];
```

## fun remap_operands

```mach
pub fun remap_operands(fn: *ir.Function, block_map: *u32, n_blocks: u32, instr_map: *u32, n_instrs: u32);
```

## fun redirect_terminator

```mach
pub fun redirect_terminator(fn: *ir.Function, blk: id.BlockId, from: id.BlockId, to: id.BlockId);
```

## fun relabel_phi_pred

```mach
pub fun relabel_phi_pred(fn: *ir.Function, blk: id.BlockId, from: id.BlockId, to: id.BlockId);
```

## fun operand_is_block

```mach
pub fun operand_is_block(k: instruction.InstrKind, o: u32) bool;
```

