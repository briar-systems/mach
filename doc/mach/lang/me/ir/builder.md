# mach.lang.me.ir.builder

## rec Builder

```mach
pub rec Builder;
```

## fun init

```mach
pub fun init(m: *ir.Module) Builder;
```

## fun set_function

```mach
pub fun set_function(b: *Builder, fn: *ir.Function);
```

## fun set_loc

```mach
pub fun set_loc(b: *Builder, loc: source.SrcLoc);
```

## fun current_loc

```mach
pub fun current_loc(b: *Builder) source.SrcLoc;
```

## fun set_block

```mach
pub fun set_block(b: *Builder, blk: id.BlockId);
```

## fun new_block

```mach
pub fun new_block(b: *Builder) res[id.BlockId, fail.Fail];
```

## fun emit_add

```mach
pub fun emit_add(b: *Builder, lhs: value.Value, rhs: value.Value) res[value.Value, fail.Fail];
```

## fun emit_sub

```mach
pub fun emit_sub(b: *Builder, lhs: value.Value, rhs: value.Value) res[value.Value, fail.Fail];
```

## fun emit_mul

```mach
pub fun emit_mul(b: *Builder, lhs: value.Value, rhs: value.Value) res[value.Value, fail.Fail];
```

## fun emit_div_s

```mach
pub fun emit_div_s(b: *Builder, lhs: value.Value, rhs: value.Value) res[value.Value, fail.Fail];
```

## fun emit_div_u

```mach
pub fun emit_div_u(b: *Builder, lhs: value.Value, rhs: value.Value) res[value.Value, fail.Fail];
```

## fun emit_rem_s

```mach
pub fun emit_rem_s(b: *Builder, lhs: value.Value, rhs: value.Value) res[value.Value, fail.Fail];
```

## fun emit_rem_u

```mach
pub fun emit_rem_u(b: *Builder, lhs: value.Value, rhs: value.Value) res[value.Value, fail.Fail];
```

## fun emit_and

```mach
pub fun emit_and(b: *Builder, lhs: value.Value, rhs: value.Value) res[value.Value, fail.Fail];
```

## fun emit_or

```mach
pub fun emit_or(b: *Builder, lhs: value.Value, rhs: value.Value) res[value.Value, fail.Fail];
```

## fun emit_xor

```mach
pub fun emit_xor(b: *Builder, lhs: value.Value, rhs: value.Value) res[value.Value, fail.Fail];
```

## fun emit_shl

```mach
pub fun emit_shl(b: *Builder, lhs: value.Value, rhs: value.Value) res[value.Value, fail.Fail];
```

## fun emit_shr_s

```mach
pub fun emit_shr_s(b: *Builder, lhs: value.Value, rhs: value.Value) res[value.Value, fail.Fail];
```

## fun emit_shr_u

```mach
pub fun emit_shr_u(b: *Builder, lhs: value.Value, rhs: value.Value) res[value.Value, fail.Fail];
```

## fun emit_neg

```mach
pub fun emit_neg(b: *Builder, operand: value.Value) res[value.Value, fail.Fail];
```

## fun emit_not

```mach
pub fun emit_not(b: *Builder, operand: value.Value) res[value.Value, fail.Fail];
```

## fun emit_cmp_eq

```mach
pub fun emit_cmp_eq(b: *Builder, lhs: value.Value, rhs: value.Value) res[value.Value, fail.Fail];
```

## fun emit_cmp_ne

```mach
pub fun emit_cmp_ne(b: *Builder, lhs: value.Value, rhs: value.Value) res[value.Value, fail.Fail];
```

## fun emit_cmp_lt_s

```mach
pub fun emit_cmp_lt_s(b: *Builder, lhs: value.Value, rhs: value.Value) res[value.Value, fail.Fail];
```

## fun emit_cmp_lt_u

```mach
pub fun emit_cmp_lt_u(b: *Builder, lhs: value.Value, rhs: value.Value) res[value.Value, fail.Fail];
```

## fun emit_cmp_le_s

```mach
pub fun emit_cmp_le_s(b: *Builder, lhs: value.Value, rhs: value.Value) res[value.Value, fail.Fail];
```

## fun emit_cmp_le_u

```mach
pub fun emit_cmp_le_u(b: *Builder, lhs: value.Value, rhs: value.Value) res[value.Value, fail.Fail];
```

## fun emit_trunc

```mach
pub fun emit_trunc(b: *Builder, operand: value.Value, to: type.IrTypeId) res[value.Value, fail.Fail];
```

## fun emit_sext

```mach
pub fun emit_sext(b: *Builder, operand: value.Value, to: type.IrTypeId) res[value.Value, fail.Fail];
```

## fun emit_zext

```mach
pub fun emit_zext(b: *Builder, operand: value.Value, to: type.IrTypeId) res[value.Value, fail.Fail];
```

## fun emit_fp_trunc

```mach
pub fun emit_fp_trunc(b: *Builder, operand: value.Value, to: type.IrTypeId) res[value.Value, fail.Fail];
```

## fun emit_fp_ext

```mach
pub fun emit_fp_ext(b: *Builder, operand: value.Value, to: type.IrTypeId) res[value.Value, fail.Fail];
```

## fun emit_fp_to_si

```mach
pub fun emit_fp_to_si(b: *Builder, operand: value.Value, to: type.IrTypeId) res[value.Value, fail.Fail];
```

## fun emit_fp_to_ui

```mach
pub fun emit_fp_to_ui(b: *Builder, operand: value.Value, to: type.IrTypeId) res[value.Value, fail.Fail];
```

## fun emit_si_to_fp

```mach
pub fun emit_si_to_fp(b: *Builder, operand: value.Value, to: type.IrTypeId) res[value.Value, fail.Fail];
```

## fun emit_ui_to_fp

```mach
pub fun emit_ui_to_fp(b: *Builder, operand: value.Value, to: type.IrTypeId) res[value.Value, fail.Fail];
```

## fun emit_bitcast

```mach
pub fun emit_bitcast(b: *Builder, operand: value.Value, to: type.IrTypeId) res[value.Value, fail.Fail];
```

## fun mark_last_secret

```mach
pub fun mark_last_secret(b: *Builder);
```

## fun mark_last_volatile

```mach
pub fun mark_last_volatile(b: *Builder);
```

## fun emit_declassify

```mach
pub fun emit_declassify(b: *Builder, operand: value.Value) res[value.Value, fail.Fail];
```

## fun emit_alloca

```mach
pub fun emit_alloca(b: *Builder, ty: type.IrTypeId, count: value.Value) res[value.Value, fail.Fail];
```

## fun emit_load

```mach
pub fun emit_load(b: *Builder, ptr: value.Value, ty: type.IrTypeId) res[value.Value, fail.Fail];
```

## fun emit_store

```mach
pub fun emit_store(b: *Builder, value_arg: value.Value, ptr: value.Value) err[fail.Fail];
```

## fun emit_memzero

```mach
pub fun emit_memzero(b: *Builder, ptr: value.Value, pointee_ty: type.IrTypeId) err[fail.Fail];
```

## fun emit_gep

```mach
pub fun emit_gep(b: *Builder, base: value.Value, src_ty: type.IrTypeId, indices: *value.Value, index_count: u32) res[value.Value, fail.Fail];
```

## fun emit_extract

```mach
pub fun emit_extract(b: *Builder, agg: value.Value, index: value.Value, ty: type.IrTypeId) res[value.Value, fail.Fail];
```

## fun emit_insert

```mach
pub fun emit_insert(b: *Builder, agg: value.Value, index: value.Value, element: value.Value) res[value.Value, fail.Fail];
```

## fun emit_vec_extract

```mach
pub fun emit_vec_extract(b: *Builder, vec: value.Value, lane: value.Value, ty: type.IrTypeId) res[value.Value, fail.Fail];
```

## fun emit_vec_insert

```mach
pub fun emit_vec_insert(b: *Builder, vec: value.Value, lane: value.Value, value_: value.Value) res[value.Value, fail.Fail];
```

## fun emit_vec_build

```mach
pub fun emit_vec_build(b: *Builder, ty: type.IrTypeId, lanes: *value.Value, n: u32) res[value.Value, fail.Fail];
```

## fun emit_phi

```mach
pub fun emit_phi(b: *Builder, ty: type.IrTypeId) res[value.Value, fail.Fail];
```

## fun phi_add_incoming

```mach
pub fun phi_add_incoming(b: *Builder, phi: value.Value, source: id.BlockId, value_arg: value.Value) err[fail.Fail];
```

## fun emit_call

```mach
pub fun emit_call(b: *Builder, callee: value.Value, args: *value.Value, arg_count: u32, ret_type: type.IrTypeId) res[value.Value, fail.Fail];
```

## fun emit_br

```mach
pub fun emit_br(b: *Builder, target: id.BlockId) err[fail.Fail];
```

## fun emit_cbr

```mach
pub fun emit_cbr(b: *Builder, cond: value.Value, then_b: id.BlockId, else_b: id.BlockId) err[fail.Fail];
```

## fun emit_ret

```mach
pub fun emit_ret(b: *Builder, value_arg: value.Value) err[fail.Fail];
```

## fun emit_ret_void

```mach
pub fun emit_ret_void(b: *Builder) err[fail.Fail];
```

## fun emit_unreachable

```mach
pub fun emit_unreachable(b: *Builder) err[fail.Fail];
```

## fun emit_asm

```mach
pub fun emit_asm(b: *Builder, operands: *value.Value, operand_count: u32) res[id.InstructionId, fail.Fail];
```

## fun emit_dbg_value

```mach
pub fun emit_dbg_value(b: *Builder, val: value.Value, name: intern.StrId, ty: type.IrTypeId,
ty_sem: semtype.TypeId, is_param: bool, scope: u32) err[fail.Fail];
```

## fun emit_vcompare

```mach
pub fun emit_vcompare(b: *Builder, k: instruction.InstrKind, lhs: value.Value, rhs: value.Value, result_ty: type.IrTypeId) res[value.Value, fail.Fail];
```

