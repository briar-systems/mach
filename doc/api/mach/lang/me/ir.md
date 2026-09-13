# mach.lang.me.ir

## val FN_FLAG_PUB

```mach
pub val FN_FLAG_PUB:       u32 = 0x01
```

## val FN_FLAG_EXTERN

```mach
pub val FN_FLAG_EXTERN:    u32 = 0x02
```

## val FN_FLAG_INLINE

```mach
pub val FN_FLAG_INLINE:    u32 = 0x04
```

## val FN_FLAG_NORETURN

```mach
pub val FN_FLAG_NORETURN:  u32 = 0x08
```

## val FN_FLAG_TEST

```mach
pub val FN_FLAG_TEST:      u32 = 0x10
```

## val FN_FLAG_WEAK

```mach
pub val FN_FLAG_WEAK:      u32 = 0x20
```

## val FN_FLAG_OBLIVIOUS

```mach
pub val FN_FLAG_OBLIVIOUS: u32 = 0x40
```

## val FN_FLAG_SCALAR

```mach
pub val FN_FLAG_SCALAR: u32 = 0x80
```

## val FN_FLAG_NOINLINE

```mach
pub val FN_FLAG_NOINLINE: u32 = 0x100
```

## val FN_FLAG_NAKED

```mach
pub val FN_FLAG_NAKED: u32 = 0x200
```

## val STAGE_NONE

```mach
pub val STAGE_NONE:     u8 = 0
```

## val STAGE_VERTEX

```mach
pub val STAGE_VERTEX:   u8 = 1
```

## val STAGE_FRAGMENT

```mach
pub val STAGE_FRAGMENT: u8 = 2
```

## val STAGE_COMPUTE

```mach
pub val STAGE_COMPUTE:  u8 = 3
```

## val IFACE_NONE

```mach
pub val IFACE_NONE:    u8 = 0
```

## val IFACE_INPUT

```mach
pub val IFACE_INPUT:   u8 = 1
```

## val IFACE_OUTPUT

```mach
pub val IFACE_OUTPUT:  u8 = 2
```

## val IFACE_BUILTIN

```mach
pub val IFACE_BUILTIN: u8 = 3
```

## val IFACE_UNIFORM

```mach
pub val IFACE_UNIFORM: u8 = 4
```

## val IFACE_STORAGE

```mach
pub val IFACE_STORAGE: u8 = 5
```

## val IFACE_SAMPLER

```mach
pub val IFACE_SAMPLER: u8 = 6
```

## val IFACE_FLAG_READONLY

```mach
pub val IFACE_FLAG_READONLY: u32 = 1
```

## val BUILTIN_POSITION

```mach
pub val BUILTIN_POSITION:          u32 = 0
```

## val BUILTIN_VERTEX_INDEX

```mach
pub val BUILTIN_VERTEX_INDEX:      u32 = 1
```

## val BUILTIN_INSTANCE_INDEX

```mach
pub val BUILTIN_INSTANCE_INDEX:    u32 = 2
```

## val BUILTIN_FRAG_COORD

```mach
pub val BUILTIN_FRAG_COORD:        u32 = 3
```

## val BUILTIN_POINT_SIZE

```mach
pub val BUILTIN_POINT_SIZE:        u32 = 4
```

## val BUILTIN_GLOBAL_INVOCATION

```mach
pub val BUILTIN_GLOBAL_INVOCATION: u32 = 5
```

## val BUILTIN_LOCAL_INVOCATION

```mach
pub val BUILTIN_LOCAL_INVOCATION:  u32 = 6
```

## val BUILTIN_WORKGROUP_ID

```mach
pub val BUILTIN_WORKGROUP_ID:      u32 = 7
```

## rec IrAsmBind

```mach
pub rec IrAsmBind;
```

## rec IrAsm

```mach
pub rec IrAsm;
```

## rec DbgVar

```mach
pub rec DbgVar;
```

## val DBG_OP_PLUS_CONST

```mach
pub val DBG_OP_PLUS_CONST:  u8 = 0
```

## val DBG_OP_MINUS_CONST

```mach
pub val DBG_OP_MINUS_CONST: u8 = 1
```

## rec DbgOp

```mach
pub rec DbgOp;
```

## val DBG_EXPR_IDENTITY

```mach
pub val DBG_EXPR_IDENTITY: DbgExprId = 0
```

## def DbgExprId

```mach
pub def DbgExprId: u32
```

## rec DbgExpr

```mach
pub rec DbgExpr;
```

## rec InlineSite

```mach
pub rec InlineSite;
```

## rec Block

```mach
pub rec Block;
```

## rec Function

```mach
pub rec Function;
```

## rec Global

```mach
pub rec Global;
```

## rec ModuleHome

```mach
pub rec ModuleHome;
```

## fun home_init

```mach
pub fun home_init(h: *ModuleHome) err[fail.Fail];
```

## fun home_dnit

```mach
pub fun home_dnit(h: *ModuleHome);
```

## rec Module

```mach
pub rec Module;
```

## fun init

```mach
pub fun init(a: *A.Allocator, name: intern.StrId) res[Module, fail.Fail];
```

## fun dnit

```mach
pub fun dnit(m: *Module);
```

## fun function_dnit

```mach
pub fun function_dnit(m: *Module, fn: *Function);
```

## fun function_new

```mach
pub fun function_new(alloc: *A.Allocator, name: intern.StrId, sig: type.IrTypeId, flags: u32) Function;
```

## fun function_add

```mach
pub fun function_add(m: *Module, name: intern.StrId, sig: type.IrTypeId, flags: u32) res[u32, fail.Fail];
```

## fun function_register

```mach
pub fun function_register(m: *Module, name: intern.StrId, idx: u32) err[fail.Fail];
```

## fun function_lookup

```mach
pub fun function_lookup(m: *Module, name: intern.StrId) opt[u32];
```

## fun global_register

```mach
pub fun global_register(m: *Module, name: intern.StrId, idx: u32) err[fail.Fail];
```

## fun global_lookup

```mach
pub fun global_lookup(m: *Module, name: intern.StrId) opt[u32];
```

## fun instruction_get

```mach
pub fun instruction_get(fn: *Function, i: id.InstructionId) opt[*instruction.Instruction];
```

## fun instruction_add

```mach
pub fun instruction_add(m: *Module, fn: *Function, inst: instruction.Instruction) res[id.InstructionId, fail.Fail];
```

## rec OperandArray

```mach
pub rec OperandArray;
```

## fun operands_alloc

```mach
pub fun operands_alloc(m: *Module, n: u32) res[OperandArray, fail.Fail];
```

## fun instr_replace_operands

```mach
pub fun instr_replace_operands(m: *Module, ins: *instruction.Instruction, arr: OperandArray);
```

## fun phi_append_incoming

```mach
pub fun phi_append_incoming(m: *Module, phi: *instruction.Instruction, pred: id.BlockId, incoming: value.Value) err[fail.Fail];
```

## fun asm_get

```mach
pub fun asm_get(fn: *Function, iid: id.InstructionId) opt[*IrAsm];
```

## fun dbg_var_get

```mach
pub fun dbg_var_get(fn: *Function, iid: id.InstructionId) opt[*DbgVar];
```

## fun record_alloca_dbg_var

```mach
pub fun record_alloca_dbg_var(fn: *Function, alloca_id: id.InstructionId, dv: DbgVar) err[fail.Fail];
```

## fun alloca_dbg_var_get

```mach
pub fun alloca_dbg_var_get(fn: *Function, alloca_id: id.InstructionId) opt[*DbgVar];
```

## fun clone_alloca_dbg_var

```mach
pub fun clone_alloca_dbg_var(src_fn: *Function, dst_fn: *Function, src_iid: id.InstructionId, dst_iid: id.InstructionId) err[fail.Fail];
```

## fun dbg_value_const

```mach
pub fun dbg_value_const(fn: *Function, iid: id.InstructionId) opt[i64];
```

## fun dbg_expr_id

```mach
pub fun dbg_expr_id(fn: *Function, iid: id.InstructionId) DbgExprId;
```

## fun dbg_expr_get

```mach
pub fun dbg_expr_get(fn: *Function, eid: DbgExprId) opt[*DbgExpr];
```

## fun dbg_expr_new

```mach
pub fun dbg_expr_new(m: *Module, fn: *Function, iid: id.InstructionId) res[DbgExprId, fail.Fail];
```

## fun dbg_expr_prepend_op

```mach
pub fun dbg_expr_prepend_op(m: *Module, fn: *Function, eid: DbgExprId, op: DbgOp) err[fail.Fail];
```

## fun inline_site_add

```mach
pub fun inline_site_add(m: *Module, fn: *Function, callee: intern.StrId, call_loc: source.SrcLoc, parent: u32) res[u32, fail.Fail];
```

## fun inline_site_get

```mach
pub fun inline_site_get(fn: *Function, site: u32) opt[*InlineSite];
```

## fun instr_inline_site_set

```mach
pub fun instr_inline_site_set(fn: *Function, iid: id.InstructionId, site: u32) err[fail.Fail];
```

## fun instr_inline_site_of

```mach
pub fun instr_inline_site_of(fn: *Function, iid: id.InstructionId) u32;
```

## fun clone_instr_inline_site

```mach
pub fun clone_instr_inline_site(src_fn: *Function, dst_fn: *Function, src_iid: id.InstructionId, dst_iid: id.InstructionId) err[fail.Fail];
```

## fun clone_dbg_metadata

```mach
pub fun clone_dbg_metadata(m: *Module, src_fn: *Function, dst_fn: *Function, src_iid: id.InstructionId, dst_iid: id.InstructionId) err[fail.Fail];
```

## fun clone_asm_payload

```mach
pub fun clone_asm_payload(m: *Module, src_fn: *Function, dst_fn: *Function,
src_iid: id.InstructionId, dst_iid: id.InstructionId,
instr_map: *u32, map_len: u32) err[fail.Fail];
```

## fun block_add

```mach
pub fun block_add(m: *Module, fn: *Function) res[id.BlockId, fail.Fail];
```

## fun block_append_instr

```mach
pub fun block_append_instr(m: *Module, blk: *Block, i: id.InstructionId) err[fail.Fail];
```

## fun block_prepend_instr

```mach
pub fun block_prepend_instr(m: *Module, blk: *Block, i: id.InstructionId) err[fail.Fail];
```

## fun block_append_phi

```mach
pub fun block_append_phi(m: *Module, blk: *Block, i: id.InstructionId) err[fail.Fail];
```

## fun block_append_pred

```mach
pub fun block_append_pred(m: *Module, blk: *Block, pred: id.BlockId) err[fail.Fail];
```

## fun block_target_make

```mach
pub fun block_target_make(m: *Module, block: id.BlockId) res[value.Value, fail.Fail];
```

## fun block_target

```mach
pub fun block_target(v: value.Value) id.BlockId;
```

## fun block_successors

```mach
pub fun block_successors(fn: *Function, blk: *Block, out0: *id.BlockId, out1: *id.BlockId) u32;
```

## fun preds_rebuild

```mach
pub fun preds_rebuild(m: *Module, fn: *Function) err[fail.Fail];
```

