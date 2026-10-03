# mach.lang.me.ir

## fwd ir_id.BlockId

```mach
fwd ir_id.BlockId
```

forwards [`mach.lang.me.ir.id.BlockId`](ir/id.md#rec-blockid)

## fwd ir_id.InstructionId

```mach
fwd ir_id.InstructionId
```

forwards [`mach.lang.me.ir.id.InstructionId`](ir/id.md#rec-instructionid)

## fwd ir_id.BLOCK_NIL

```mach
fwd ir_id.BLOCK_NIL
```

forwards [`mach.lang.me.ir.id.BLOCK_NIL`](ir/id.md#val-block_nil)

## fwd ir_id.INSTR_NIL

```mach
fwd ir_id.INSTR_NIL
```

forwards [`mach.lang.me.ir.id.INSTR_NIL`](ir/id.md#val-instr_nil)

## val FN_FLAG_PUB

```mach
pub val FN_FLAG_PUB:       u32 = 0x01
```

the declaration carried `pub`, and nothing else sets it: a synthesized
definition is never `pub`, however visible it has to stay within a link

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

## val IFACE_PUSH

```mach
pub val IFACE_PUSH:    u8 = 7
```

## val IFACE_SPEC

```mach
pub val IFACE_SPEC:   u8 = 8
```

a specialization constant: iface_a is its SpecId, the initializer its default

## val IFACE_SHARED

```mach
pub val IFACE_SHARED: u8 = 9
```

## val IFACE_FLAG_READONLY

```mach
pub val IFACE_FLAG_READONLY:  u32 = 1
```

## val IFACE_FLAG_WRITEONLY

```mach
pub val IFACE_FLAG_WRITEONLY: u32 = 2
```

## val IFACE_FLAG_COHERENT

```mach
pub val IFACE_FLAG_COHERENT:  u32 = 4
```

## val BUILTIN_POSITION

```mach
pub val BUILTIN_POSITION:               u32 = 0
```

## val BUILTIN_VERTEX_INDEX

```mach
pub val BUILTIN_VERTEX_INDEX:           u32 = 1
```

## val BUILTIN_INSTANCE_INDEX

```mach
pub val BUILTIN_INSTANCE_INDEX:         u32 = 2
```

## val BUILTIN_FRAG_COORD

```mach
pub val BUILTIN_FRAG_COORD:             u32 = 3
```

## val BUILTIN_POINT_SIZE

```mach
pub val BUILTIN_POINT_SIZE:             u32 = 4
```

## val BUILTIN_GLOBAL_INVOCATION

```mach
pub val BUILTIN_GLOBAL_INVOCATION:      u32 = 5
```

## val BUILTIN_LOCAL_INVOCATION

```mach
pub val BUILTIN_LOCAL_INVOCATION:       u32 = 6
```

## val BUILTIN_WORKGROUP_ID

```mach
pub val BUILTIN_WORKGROUP_ID:           u32 = 7
```

## val BUILTIN_NUM_WORKGROUPS

```mach
pub val BUILTIN_NUM_WORKGROUPS:         u32 = 8
```

## val BUILTIN_LOCAL_INVOCATION_INDEX

```mach
pub val BUILTIN_LOCAL_INVOCATION_INDEX: u32 = 9
```

## val BUILTIN_SUBGROUP_SIZE

```mach
pub val BUILTIN_SUBGROUP_SIZE:          u32 = 10
```

## val BUILTIN_SUBGROUP_INVOCATION

```mach
pub val BUILTIN_SUBGROUP_INVOCATION:    u32 = 11
```

## val BUILTIN_SUBGROUP_ID

```mach
pub val BUILTIN_SUBGROUP_ID:            u32 = 12
```

## val BUILTIN_NUM_SUBGROUPS

```mach
pub val BUILTIN_NUM_SUBGROUPS:          u32 = 13
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

## rec DbgExprId

```mach
pub rec DbgExprId;
```

a debug expression's slot in its function's expression table

## val DBG_EXPR_IDENTITY

```mach
pub val DBG_EXPR_IDENTITY: DbgExprId = DbgExprId;
```

the empty expression, slot 0: a variable read as it is

## fun dbg_expr

```mach
pub fun dbg_expr(index: u32) DbgExprId;
```

## fun dbg_expr_index

```mach
pub fun dbg_expr_index(id: DbgExprId) u32;
```

## fun dbg_expr_same

```mach
pub fun dbg_expr_same(left: DbgExprId, right: DbgExprId) bool;
```

## fun dbg_expr_is_identity

```mach
pub fun dbg_expr_is_identity(id: DbgExprId) bool;
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
pub fun home_init(h: *ModuleHome, backing: *A.Allocator) err[fail.Fail];
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

## fun block_dnit

```mach
pub fun block_dnit(m: *Module, blk: *Block);
```

releases the block's instruction, phi and predecessor lists

## fun function_new

```mach
pub fun function_new(alloc: *A.Allocator, name: intern.StrId, sig: ir_type.IrTypeId, flags: u32) Function;
```

## fun function_add

```mach
pub fun function_add(m: *Module, name: intern.StrId, sig: ir_type.IrTypeId, flags: u32) res[u32, fail.Fail];
```

## fun export_request_add

```mach
pub fun export_request_add(m: *Module, name: intern.StrId) err[fail.Fail];
```

a name this module's `fwd` re-exports puts on the library's export surface;
recording the same name twice is the common case (two modules re-exporting one
declaration) and collapses here

## fun function_register

```mach
pub fun function_register(m: *Module, name: intern.StrId, idx: u32) err[fail.Fail];
```

a name answers its definition once the module has one: a declaration
registered after a definition of the same name leaves the definition named

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

## fun global_init

```mach
pub fun global_init(name: intern.StrId, ty: ir_type.IrTypeId, init_value: value.Value) Global;
```

a global named `name` of type `ty` holding `init`, private, immutable and
defined here, every other field its empty value

## fun global_add

```mach
pub fun global_add(m: *Module, g: Global) res[u32, fail.Fail];
```

`g` appended to the module's globals, its index; the name is not registered

## fun instruction_get

```mach
pub fun instruction_get(fn: *Function, i: ir_id.InstructionId) opt[*ir_instruction.Instruction];
```

## fun constant_behind

```mach
pub fun constant_behind(fn: *Function, v: value.Value) value.Value;
```

the constant a value is: the operand of the `const` that materializes it
, read at the use's type and secrecy, and any other value as itself.
a pass that reads constants reads through this, so a materialized constant
folds and compares as the constant it holds

## fun instruction_add

```mach
pub fun instruction_add(m: *Module, fn: *Function, inst: ir_instruction.Instruction) res[ir_id.InstructionId, fail.Fail];
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
pub fun instr_replace_operands(m: *Module, ins: *ir_instruction.Instruction, arr: OperandArray);
```

## fun phi_append_incoming

```mach
pub fun phi_append_incoming(m: *Module, phi: *ir_instruction.Instruction, pred: ir_id.BlockId, incoming: value.Value) err[fail.Fail];
```

## fun phi_incoming_del

```mach
pub fun phi_incoming_del(phi: *ir_instruction.Instruction, pred: ir_id.BlockId);
```

the incoming edge from `pred` dropped from `phi`, the first when several

## fun phi_incoming_del_at

```mach
pub fun phi_incoming_del_at(phi: *ir_instruction.Instruction, at: u32);
```

the incoming pair whose block operand is at `at` dropped from `phi`

## fun phi_incoming_keep

```mach
pub fun phi_incoming_keep[T](phi: *ir_instruction.Instruction, ctx: *T, keep: fun(*T, *value.Value) bool);
```

the incoming pairs of `phi` that `keep` accepts, in order; `keep` sees each
pair's block operand and may rewrite it

## fun dbg_var_get

```mach
pub fun dbg_var_get(fn: *Function, iid: ir_id.InstructionId) opt[*DbgVar];
```

## fun record_alloca_dbg_var

```mach
pub fun record_alloca_dbg_var(fn: *Function, alloca_id: ir_id.InstructionId, dv: DbgVar) err[fail.Fail];
```

## fun alloca_dbg_var_get

```mach
pub fun alloca_dbg_var_get(fn: *Function, alloca_id: ir_id.InstructionId) opt[*DbgVar];
```

## fun clone_alloca_dbg_var

```mach
pub fun clone_alloca_dbg_var(src_fn: *Function, dst_fn: *Function, src_iid: ir_id.InstructionId, dst_iid: ir_id.InstructionId) err[fail.Fail];
```

## fun dbg_value_const

```mach
pub fun dbg_value_const(fn: *Function, iid: ir_id.InstructionId) opt[i64];
```

## fun dbg_expr_id

```mach
pub fun dbg_expr_id(fn: *Function, iid: ir_id.InstructionId) DbgExprId;
```

## fun dbg_expr_get

```mach
pub fun dbg_expr_get(fn: *Function, eid: DbgExprId) opt[*DbgExpr];
```

## fun dbg_expr_new

```mach
pub fun dbg_expr_new(m: *Module, fn: *Function, iid: ir_id.InstructionId) res[DbgExprId, fail.Fail];
```

## fun dbg_expr_prepend_op

```mach
pub fun dbg_expr_prepend_op(m: *Module, fn: *Function, eid: DbgExprId, op: DbgOp) err[fail.Fail];
```

## fun inline_site_add

```mach
pub fun inline_site_add(m: *Module, fn: *Function, callee: intern.StrId, call_loc: lang_source.Location, parent: u32) res[u32, fail.Fail];
```

## fun instr_inline_site_set

```mach
pub fun instr_inline_site_set(fn: *Function, iid: ir_id.InstructionId, site: u32) err[fail.Fail];
```

## fun instr_inline_site_of

```mach
pub fun instr_inline_site_of(fn: *Function, iid: ir_id.InstructionId) u32;
```

## fun clone_instr_inline_site

```mach
pub fun clone_instr_inline_site(src_fn: *Function, dst_fn: *Function, src_iid: ir_id.InstructionId, dst_iid: ir_id.InstructionId) err[fail.Fail];
```

## fun clone_dbg_metadata

```mach
pub fun clone_dbg_metadata(m: *Module, src_fn: *Function, dst_fn: *Function, src_iid: ir_id.InstructionId, dst_iid: ir_id.InstructionId) err[fail.Fail];
```

## fun clone_asm_payload

```mach
pub fun clone_asm_payload(m: *Module, src_fn: *Function, dst_fn: *Function,
src_iid: ir_id.InstructionId, dst_iid: ir_id.InstructionId,
instr_map: *u32, map_len: u32) err[fail.Fail];
```

## fun block_add

```mach
pub fun block_add(m: *Module, fn: *Function) res[ir_id.BlockId, fail.Fail];
```

## fun block_append_instr

```mach
pub fun block_append_instr(m: *Module, blk: *Block, i: ir_id.InstructionId) err[fail.Fail];
```

## fun block_prepend_instr

```mach
pub fun block_prepend_instr(m: *Module, blk: *Block, i: ir_id.InstructionId) err[fail.Fail];
```

## fun block_insert_instr

```mach
pub fun block_insert_instr(m: *Module, blk: *Block, pos: u32, i: ir_id.InstructionId) err[fail.Fail];
```

`i` listed at `pos` of the block, or at its end when `pos` is past it

## fun block_append_phi

```mach
pub fun block_append_phi(m: *Module, blk: *Block, i: ir_id.InstructionId) err[fail.Fail];
```

## fun block_append_pred

```mach
pub fun block_append_pred(m: *Module, blk: *Block, pred: ir_id.BlockId) err[fail.Fail];
```

## fun block_target_make

```mach
pub fun block_target_make(m: *Module, block: ir_id.BlockId) res[value.Value, fail.Fail];
```

## fun block_target

```mach
pub fun block_target(v: value.Value) ir_id.BlockId;
```

## fun block_target_set

```mach
pub fun block_target_set(v: *value.Value, b: ir_id.BlockId);
```

the block operand `v` made to name block `b`

## fun placement_fill

```mach
pub fun placement_fill(fn: *Function, blocks: *ir_id.BlockId, ords: *u32, len: u32);
```

where each of the first `len` instructions sits: `blocks[i]` is the block
listing instruction i, BLOCK_NIL for one no block lists, and `ords[i]`, when
`ords` is not nil, its position in that block counting phis, then
instructions, then the terminator

## fun block_successors

```mach
pub fun block_successors(fn: *Function, blk: *Block, out0: *ir_id.BlockId, out1: *ir_id.BlockId) u32;
```

## fun preds_rebuild

```mach
pub fun preds_rebuild(m: *Module, fn: *Function) err[fail.Fail];
```

