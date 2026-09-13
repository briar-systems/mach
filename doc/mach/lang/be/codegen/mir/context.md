# mach.lang.be.codegen.mir.context

## rec LowerCtx

```mach
pub rec LowerCtx;
```

## fun ctx_setup

```mach
pub fun ctx_setup(ctx: *LowerCtx, tgt: *target.Target, fn: *ir.Function) err[fail.Fail];
```

## fun ctx_dnit

```mach
pub fun ctx_dnit(ctx: *LowerCtx);
```

## fun push_pending_dbg

```mach
pub fun push_pending_dbg(ctx: *LowerCtx, iid: u32, vreg: u32) err[fail.Fail];
```

## fun drain_pending_dbg

```mach
pub fun drain_pending_dbg(ctx: *LowerCtx, mi: *mir.MirInstr) err[fail.Fail];
```

## fun values_convention

```mach
pub fun values_convention(ctx: *LowerCtx) bool;
```

the convention declares how values cross a call: a values convention has no banks, so every vreg is a value

## fun new_vreg

```mach
pub fun new_vreg(ctx: *LowerCtx, reg_class: u32) res[mir.VRegId, fail.Fail];
```

## fun new_vreg_vec

```mach
pub fun new_vreg_vec(ctx: *LowerCtx, reg_class: u32, vector: bool) res[mir.VRegId, fail.Fail];
```

## fun instr_vreg

```mach
pub fun instr_vreg(ctx: *LowerCtx, iid: id.InstructionId) mir.VRegId;
```

## fun slot_key_vreg

```mach
pub fun slot_key_vreg(ctx: *LowerCtx, iid: id.InstructionId) mir.VRegId;
```

## fun value_is_float

```mach
pub fun value_is_float(ctx: *LowerCtx, ty: ir_type.IrTypeId) bool;
```

## fun value_is_aggregate

```mach
pub fun value_is_aggregate(ctx: *LowerCtx, ty: ir_type.IrTypeId) bool;
```

## fun value_is_vector

```mach
pub fun value_is_vector(ctx: *LowerCtx, ty: ir_type.IrTypeId) bool;
```

## fun vec_lane_for_type

```mach
pub fun vec_lane_for_type(ctx: *LowerCtx, ty: ir_type.IrTypeId) u32;
```

## fun compute_vec_lane

```mach
pub fun compute_vec_lane(ctx: *LowerCtx, inst: *instruction.Instruction) u32;
```

## fun fn_return_type

```mach
pub fun fn_return_type(ctx: *LowerCtx) ir_type.IrTypeId;
```

## fun ret_align

```mach
pub fun ret_align(ctx: *LowerCtx, ty: ir_type.IrTypeId) u32;
```

## fun op_width_of

```mach
pub fun op_width_of(ctx: *LowerCtx, ty: ir_type.IrTypeId) u8;
```

## fun mem_width_of

```mach
pub fun mem_width_of(ctx: *LowerCtx, ty: ir_type.IrTypeId) u8;
```

## fun clamp_alu_width

```mach
pub fun clamp_alu_width(alu_min_width: u32, w: u8) u8;
```

## fun stack_slot_bytes

```mach
pub fun stack_slot_bytes(tgt: *target.Target, size: u64) i64;
```

## fun lower_value

```mach
pub fun lower_value(ctx: *LowerCtx, v: value.Value) res[mir.MirOperand, fail.Fail];
```

the MIR operand of an IR value. a byte or aggregate constant is a declared
member no operand carries (it is lowered through storage), reported as
unsupported; a kind outside the catalog is an internal failure naming the
tag; both were panics behind the verifier

## fun lower_operand

```mach
pub fun lower_operand(ctx: *LowerCtx, inst: *instruction.Instruction, idx: u32) res[mir.MirOperand, fail.Fail];
```

## fun mem_operand_of

```mach
pub fun mem_operand_of(ctx: *LowerCtx, v: value.Value) res[mir.MirOperand, fail.Fail];
```

## fun push_instr

```mach
pub fun push_instr(ctx: *LowerCtx, mb: *mir.MirBlock, mi: mir.MirInstr) err[fail.Fail];
```

## fun grow_instr

```mach
pub fun grow_instr(ctx: *LowerCtx, mb: *mir.MirBlock) err[fail.Fail];
```

## fun emit_mov

```mach
pub fun emit_mov(ctx: *LowerCtx, mb: *mir.MirBlock, dst: mir.MirOperand, src: mir.MirOperand) err[fail.Fail];
```

## fun emit_mov_w

```mach
pub fun emit_mov_w(ctx: *LowerCtx, mb: *mir.MirBlock, dst: mir.MirOperand, src: mir.MirOperand, w: u8) err[fail.Fail];
```

## fun emit_declassify

```mach
pub fun emit_declassify(ctx: *LowerCtx, mb: *mir.MirBlock, dst: mir.MirOperand, src: mir.MirOperand, w: u8) err[fail.Fail];
```

## fun emit_fmov

```mach
pub fun emit_fmov(ctx: *LowerCtx, mb: *mir.MirBlock, dst: mir.MirOperand, src: mir.MirOperand, w: u8) err[fail.Fail];
```

## fun emit_store_to

```mach
pub fun emit_store_to(ctx: *LowerCtx, mb: *mir.MirBlock, mem: mir.MirOperand, src: mir.MirOperand) err[fail.Fail];
```

## fun emit_store_to_w

```mach
pub fun emit_store_to_w(ctx: *LowerCtx, mb: *mir.MirBlock, mem: mir.MirOperand, src: mir.MirOperand, width: u8) err[fail.Fail];
```

## fun emit_load_from_w

```mach
pub fun emit_load_from_w(ctx: *LowerCtx, mb: *mir.MirBlock, dst: mir.VRegId, mem: mir.MirOperand, width: u8) err[fail.Fail];
```

## fun push_store_chunk

```mach
pub fun push_store_chunk(ctx: *LowerCtx, mb: *mir.MirBlock, mem: mir.MirOperand, src: mir.MirOperand, w: u8) err[fail.Fail];
```

## fun store_vector_register

```mach
pub fun store_vector_register(ctx: *LowerCtx, mb: *mir.MirBlock, dest: mir.MirOperand,
value: mir.MirOperand, ty: ir_type.IrTypeId) err[fail.Fail];
```

## fun mem_at

```mach
pub fun mem_at(m: mir.MirOperand, off: i64) mir.MirOperand;
```

## fun store_subwidth_vector

```mach
pub fun store_subwidth_vector(ctx: *LowerCtx, mb: *mir.MirBlock, dest: mir.MirOperand,
value: mir.MirOperand, ty: ir_type.IrTypeId) err[fail.Fail];
```

## fun load_subwidth_vector

```mach
pub fun load_subwidth_vector(ctx: *LowerCtx, mb: *mir.MirBlock, dst: mir.MirOperand,
src: mir.MirOperand, ty: ir_type.IrTypeId) err[fail.Fail];
```

## fun subwidth_vector_memory

```mach
pub fun subwidth_vector_memory(ctx: *LowerCtx, ty: ir_type.IrTypeId) bool;
```

## fun copy_chunk_width

```mach
pub fun copy_chunk_width(tgt: *target.Target, remaining: u64) u8;
```

## fun emit_bin

```mach
pub fun emit_bin(ctx: *LowerCtx, mb: *mir.MirBlock, op: mir.MirOpcode, dst: mir.VRegId, a: mir.MirOperand, b: mir.MirOperand) err[fail.Fail];
```

## fun emit_lea_preg

```mach
pub fun emit_lea_preg(ctx: *LowerCtx, mb: *mir.MirBlock, dst: mir.VRegId, base_preg: mir.PRegId, disp: i64) err[fail.Fail];
```

## fun emit_lea_slot

```mach
pub fun emit_lea_slot(ctx: *LowerCtx, mb: *mir.MirBlock, mem: mir.MirOperand, out: *mir.VRegId) err[fail.Fail];
```

## fun addr_to_vreg

```mach
pub fun addr_to_vreg(ctx: *LowerCtx, mb: *mir.MirBlock, argop: mir.MirOperand, out_vreg: *mir.VRegId) err[fail.Fail];
```

## fun copy_aggregate

```mach
pub fun copy_aggregate(ctx: *LowerCtx, mb: *mir.MirBlock, dst_mem: mir.MirOperand, src: mir.MirOperand, size: u64) err[fail.Fail];
```

## fun alloc_object_storage

```mach
pub fun alloc_object_storage(ctx: *LowerCtx, mb: *mir.MirBlock, dst: mir.VRegId, ty: ir_type.IrTypeId) err[fail.Fail];
```

## fun add_alloca_slot

```mach
pub fun add_alloca_slot(ctx: *LowerCtx, value_id: u32, size: u64, align: u32, ty: u32) err[fail.Fail];
```

## fun add_spill_slot

```mach
pub fun add_spill_slot(ctx: *LowerCtx, value_id: u32, size: u64) err[fail.Fail];
```

## fun alloc_result_storage

```mach
pub fun alloc_result_storage(ctx: *LowerCtx, mb: *mir.MirBlock, dst: mir.VRegId, size: u64, align: u32) err[fail.Fail];
```

## fun add_spill_slot_aligned

```mach
pub fun add_spill_slot_aligned(ctx: *LowerCtx, value_id: u32, size: u64, align: u32) err[fail.Fail];
```

