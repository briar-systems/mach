# mach.lang.me.legalize.scalarize.place

the expansion of a vector value held in memory: every operation on it
becomes a load, a scalar operation and a store per lane through its stack
home, the lanes a register cannot hold

## rec Types

```mach
pub rec Types;
```

the types an expansion builds with

## rec Ctx

```mach
pub rec Ctx;
```

## fun lane_scratch

```mach
pub fun lane_scratch(c: *Ctx, n: u32) res[*value.Value, fail.Fail];
```

## rec VInfo

```mach
pub rec VInfo;
```

## fun is_binary_op

```mach
pub fun is_binary_op(k: ir_instruction.InstrKind) bool;
```

## fun is_unary_op

```mach
pub fun is_unary_op(k: ir_instruction.InstrKind) bool;
```

## fun is_compare_op

```mach
pub fun is_compare_op(k: ir_instruction.InstrKind) bool;
```

## fun is_convert_op

```mach
pub fun is_convert_op(k: ir_instruction.InstrKind) bool;
```

## fun is_operator

```mach
pub fun is_operator(m: *me_ir.Module, inst: *ir_instruction.Instruction) bool;
```

## fun vec_info

```mach
pub fun vec_info(m: *me_ir.Module, vty: ir_type.IrTypeId, out: *VInfo) err[fail.Fail];
```

## fun new_instr

```mach
pub fun new_instr(m: *me_ir.Module, fn: *me_ir.Function, kind: ir_instruction.InstrKind, ty: ir_type.IrTypeId,
aux: ir_type.IrTypeId, ops: *value.Value, opn: u32, loc: lang_source.Location) res[ir_id.InstructionId, fail.Fail];
```

a new instruction listed in no block, at `loc`

## fun push

```mach
pub fun push(c: *Ctx, blk: *me_ir.Block, kind: ir_instruction.InstrKind, ty: ir_type.IrTypeId,
aux: ir_type.IrTypeId, ops: *value.Value, opn: u32, loc: lang_source.Location) res[value.Value, fail.Fail];
```

## fun lane_ptr

```mach
pub fun lane_ptr(c: *Ctx, blk: *me_ir.Block, base: value.Value, arr_ty: ir_type.IrTypeId, k: u32, loc: lang_source.Location) res[value.Value, fail.Fail];
```

## fun emit_load

```mach
pub fun emit_load(c: *Ctx, blk: *me_ir.Block, ptr: value.Value, elem_ty: ir_type.IrTypeId, loc: lang_source.Location, flags: u16) res[value.Value, fail.Fail];
```

## fun emit_store

```mach
pub fun emit_store(c: *Ctx, blk: *me_ir.Block, v: value.Value, ptr: value.Value, loc: lang_source.Location, flags: u16) err[fail.Fail];
```

## fun lane_secret

```mach
pub fun lane_secret(c: *Ctx, loaded: value.Value, src_secret: bool) value.Value;
```

## fun emit_binop

```mach
pub fun emit_binop(c: *Ctx, blk: *me_ir.Block, kind: ir_instruction.InstrKind, ty: ir_type.IrTypeId, a: value.Value, b: value.Value, loc: lang_source.Location) res[value.Value, fail.Fail];
```

## fun emit_unop

```mach
pub fun emit_unop(c: *Ctx, blk: *me_ir.Block, kind: ir_instruction.InstrKind, ty: ir_type.IrTypeId, a: value.Value, loc: lang_source.Location) res[value.Value, fail.Fail];
```

## fun resolve

```mach
pub fun resolve(c: *Ctx, v: value.Value) value.Value;
```

## fun expand_convert

```mach
pub fun expand_convert(c: *Ctx, blk: *me_ir.Block, kind: ir_instruction.InstrKind, slot: value.Value, dst_ty: ir_type.IrTypeId,
src: value.Value, src_ty: ir_type.IrTypeId, secret: bool, loc: lang_source.Location) res[value.Value, fail.Fail];
```

converts each lane of `src` into the stack home `slot` of the result: a side
with a memory image is reached through it, a side held in a register
through lane reads, and a register result is loaded back from the home

## fun reserve_conv_slots

```mach
pub fun reserve_conv_slots(c: *Ctx, wanted: fun(*Ctx, *ir_instruction.Instruction) bool) err[fail.Fail];
```

reserves a home for every conversion `wanted` selects, before any block is rebuilt

## fun append_conv_slots

```mach
pub fun append_conv_slots(c: *Ctx, blk: *me_ir.Block) err[fail.Fail];
```

## fun body_free

```mach
pub fun body_free(c: *Ctx, old: *ir_id.InstructionId, old_len: u32);
```

## fun lane_types

```mach
pub fun lane_types(m: *me_ir.Module, tgt: *resolved.Target) res[Types, fail.Fail];
```

the types a lane or gap expansion builds with

## fun emit_lane_read

```mach
pub fun emit_lane_read(c: *Ctx, blk: *me_ir.Block, vec: value.Value, elem_ty: ir_type.IrTypeId, k: u32, loc: lang_source.Location) res[value.Value, fail.Fail];
```

## fun emit_lane_write

```mach
pub fun emit_lane_write(c: *Ctx, blk: *me_ir.Block, vec: value.Value, vty: ir_type.IrTypeId, k: u32, v: value.Value, loc: lang_source.Location) res[value.Value, fail.Fail];
```

## fun rewrite_uses

```mach
pub fun rewrite_uses(c: *Ctx, fn: *me_ir.Function);
```

## val PASS

```mach
pub val PASS: pass.Pass = pass.Pass;
```

the pass the pipeline schedules

## fun mark_listed

```mach
pub fun mark_listed(c: *Ctx) err[fail.Fail];
```

## fun ctx_free

```mach
pub fun ctx_free(c: *Ctx);
```

