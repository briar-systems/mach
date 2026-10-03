# mach.lang.me.ir.builder

## def Place

```mach
pub def Place: u8
```

where the next instruction goes

## val PLACE_BUILD

```mach
pub val PLACE_BUILD: Place = 0
```

appended to the cursor block, which must not be terminated yet: building

## val PLACE_AT

```mach
pub val PLACE_AT: Place = 1
```

inserted into the cursor block's list at the cursor position, which then
moves past it, so a run of instructions lands in order: editing

## val PLACE_NONE

```mach
pub val PLACE_NONE: Place = 2
```

made in the function and listed in no block, for the caller to place

## rec Builder

```mach
pub rec Builder;
```

the one maker of ir instructions. it holds the insertion point and the
attributes every instruction it makes carries (flags, secret, location,
inline site), and in its sticky mode the first failure of a run of emits

## fun init

```mach
pub fun init(m: *me_ir.Module) Builder;
```

## fun on

```mach
pub fun on(m: *me_ir.Module, fn: *me_ir.Function) Builder;
```

a builder over function `fn` of `m`, its instructions made detached

## fun set_function

```mach
pub fun set_function(b: *Builder, fn: *me_ir.Function);
```

## fun set_loc

```mach
pub fun set_loc(b: *Builder, loc: lang_source.Location);
```

## fun current_loc

```mach
pub fun current_loc(b: *Builder) lang_source.Location;
```

## fun set_flags

```mach
pub fun set_flags(b: *Builder, flags: u16);
```

the flags the next instructions carry

## fun set_secret

```mach
pub fun set_secret(b: *Builder, secret: bool);
```

whether the next instructions are secret

## fun set_site

```mach
pub fun set_site(b: *Builder, site: u32);
```

the inline site the next instructions belong to, 0 for none

## fun derive

```mach
pub fun derive(b: *Builder, origin: ir_id.InstructionId);
```

the next instructions read as `origin`, the instruction they rewrite: its
location, inline site and secrecy, and no flags

## fun plain

```mach
pub fun plain(b: *Builder);
```

the next instructions carry no location, flags, secrecy or inline site

## fun set_block

```mach
pub fun set_block(b: *Builder, blk: ir_id.BlockId);
```

build at the end of block `blk`

## fun at

```mach
pub fun at(b: *Builder, blk: ir_id.BlockId, pos: u32);
```

insert into block `blk`'s list at `pos`, after its phis whatever `pos`

## fun after_phis

```mach
pub fun after_phis(b: *Builder, blk: ir_id.BlockId);
```

insert at the head of block `blk`'s list, right after its phis

## fun at_end

```mach
pub fun at_end(b: *Builder, blk: ir_id.BlockId);
```

insert at the end of block `blk`'s list, whether or not it is terminated

## fun detach

```mach
pub fun detach(b: *Builder);
```

make instructions listed in no block

## fun position

```mach
pub fun position(b: *Builder) u32;
```

where an insertion point has moved to, the position after the last insert

## fun new_block

```mach
pub fun new_block(b: *Builder) res[ir_id.BlockId, fail.Fail];
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

## fun emit_select

```mach
pub fun emit_select(b: *Builder, cond: value.Value, a: value.Value, other: value.Value) res[value.Value, fail.Fail];
```

`a` when `cond` is nonzero, else `other`, typed as `a`

## fun emit_trunc

```mach
pub fun emit_trunc(b: *Builder, operand: value.Value, to: ir_type.IrTypeId) res[value.Value, fail.Fail];
```

## fun emit_sext

```mach
pub fun emit_sext(b: *Builder, operand: value.Value, to: ir_type.IrTypeId) res[value.Value, fail.Fail];
```

## fun emit_zext

```mach
pub fun emit_zext(b: *Builder, operand: value.Value, to: ir_type.IrTypeId) res[value.Value, fail.Fail];
```

## fun emit_fp_trunc

```mach
pub fun emit_fp_trunc(b: *Builder, operand: value.Value, to: ir_type.IrTypeId) res[value.Value, fail.Fail];
```

## fun emit_fp_ext

```mach
pub fun emit_fp_ext(b: *Builder, operand: value.Value, to: ir_type.IrTypeId) res[value.Value, fail.Fail];
```

## fun emit_fp_to_si

```mach
pub fun emit_fp_to_si(b: *Builder, operand: value.Value, to: ir_type.IrTypeId) res[value.Value, fail.Fail];
```

## fun emit_fp_to_ui

```mach
pub fun emit_fp_to_ui(b: *Builder, operand: value.Value, to: ir_type.IrTypeId) res[value.Value, fail.Fail];
```

## fun emit_si_to_fp

```mach
pub fun emit_si_to_fp(b: *Builder, operand: value.Value, to: ir_type.IrTypeId) res[value.Value, fail.Fail];
```

## fun emit_ui_to_fp

```mach
pub fun emit_ui_to_fp(b: *Builder, operand: value.Value, to: ir_type.IrTypeId) res[value.Value, fail.Fail];
```

## fun emit_bitcast

```mach
pub fun emit_bitcast(b: *Builder, operand: value.Value, to: ir_type.IrTypeId) res[value.Value, fail.Fail];
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
pub fun emit_alloca(b: *Builder, ty: ir_type.IrTypeId, count: value.Value) res[value.Value, fail.Fail];
```

## fun emit_load

```mach
pub fun emit_load(b: *Builder, ptr: value.Value, ty: ir_type.IrTypeId) res[value.Value, fail.Fail];
```

## fun emit_store

```mach
pub fun emit_store(b: *Builder, value_arg: value.Value, ptr: value.Value) err[fail.Fail];
```

## fun emit_memzero

```mach
pub fun emit_memzero(b: *Builder, ptr: value.Value, pointee_ty: ir_type.IrTypeId) err[fail.Fail];
```

## fun emit_gep

```mach
pub fun emit_gep(b: *Builder, base: value.Value, src_ty: ir_type.IrTypeId, indices: *value.Value, index_count: u32) res[value.Value, fail.Fail];
```

## fun emit_vec_extract

```mach
pub fun emit_vec_extract(b: *Builder, vec: value.Value, lane: value.Value, ty: ir_type.IrTypeId) res[value.Value, fail.Fail];
```

## fun emit_vec_insert

```mach
pub fun emit_vec_insert(b: *Builder, vec: value.Value, lane: value.Value, value_: value.Value) res[value.Value, fail.Fail];
```

## fun emit_vec_widen_half

```mach
pub fun emit_vec_widen_half(b: *Builder, signed: bool, src: value.Value, base: value.Value, ty: ir_type.IrTypeId) res[value.Value, fail.Fail];
```

the lane-halving extension of the half of `src` whose first lane is `base`:
a vector of half the lanes at twice the width, sign- or zero-extended

## fun emit_vec_widen_sum_u

```mach
pub fun emit_vec_widen_sum_u(b: *Builder, src: value.Value, ty: ir_type.IrTypeId) res[value.Value, fail.Fail];
```

each lane of `ty` the zero-extended sum of the lanes of `src` it covers

## fun emit_vec_range

```mach
pub fun emit_vec_range(b: *Builder, src: value.Value, start: value.Value, ty: ir_type.IrTypeId) res[value.Value, fail.Fail];
```

the `ty` lanes of `src` from constant lane `start` on

## fun lanes_reserve

```mach
pub fun lanes_reserve(b: *Builder, n: u32) res[*value.Value, fail.Fail];
```

a buffer for the `n` lanes of a vector build, sized from the lane count since a
target that builds vectors from lanes has no register ceiling on it; released
with `lanes_release`

## fun lanes_release

```mach
pub fun lanes_release(b: *Builder, lanes: *value.Value, n: u32);
```

## fun emit_vec_build

```mach
pub fun emit_vec_build(b: *Builder, ty: ir_type.IrTypeId, lanes: *value.Value, n: u32) res[value.Value, fail.Fail];
```

## fun emit_phi

```mach
pub fun emit_phi(b: *Builder, ty: ir_type.IrTypeId) res[value.Value, fail.Fail];
```

## fun phi_add_incoming

```mach
pub fun phi_add_incoming(b: *Builder, phi: value.Value, source: ir_id.BlockId, value_arg: value.Value) err[fail.Fail];
```

## fun emit_call

```mach
pub fun emit_call(b: *Builder, callee: value.Value, args: *value.Value, arg_count: u32, ret_type: ir_type.IrTypeId) res[value.Value, fail.Fail];
```

## fun emit_br

```mach
pub fun emit_br(b: *Builder, target: ir_id.BlockId) err[fail.Fail];
```

## fun emit_cbr

```mach
pub fun emit_cbr(b: *Builder, cond: value.Value, then_b: ir_id.BlockId, else_b: ir_id.BlockId) err[fail.Fail];
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
pub fun emit_asm(b: *Builder, operands: *value.Value, operand_count: u32) res[ir_id.InstructionId, fail.Fail];
```

## fun emit_dbg_value

```mach
pub fun emit_dbg_value(b: *Builder, val: value.Value, name: intern.StrId, ty: ir_type.IrTypeId,
ty_sem: type.TypeId, is_param: bool, scope: u32) err[fail.Fail];
```

## fun emit_vcompare

```mach
pub fun emit_vcompare(b: *Builder, k: ir_instruction.InstrKind, lhs: value.Value, rhs: value.Value, result_ty: ir_type.IrTypeId) res[value.Value, fail.Fail];
```

## fun emit_instr

```mach
pub fun emit_instr(b: *Builder, k: ir_instruction.InstrKind, ty: ir_type.IrTypeId, aux: ir_type.IrTypeId,
ops: *value.Value, n: u32) res[ir_id.InstructionId, fail.Fail];
```

instruction `k` of type `ty` with auxiliary type `aux` over a copy of the
`n` operands at `ops`, made at the insertion point with the builder's
attributes

## fun emit_instr_owned

```mach
pub fun emit_instr_owned(b: *Builder, k: ir_instruction.InstrKind, ty: ir_type.IrTypeId, aux: ir_type.IrTypeId,
owned: *value.Value, n: u32) res[ir_id.InstructionId, fail.Fail];
```

as emit_instr, taking ownership of the `n` operands allocated at `owned`,
which are released when the instruction cannot be made

## fun take

```mach
pub fun take(b: *Builder, r: res[value.Value, fail.Fail]) value.Value;
```

the value of an emit in sticky mode: a failure is kept when it is the
first, and the nil value stands in for the result, never to be used, since
the caller reads `done` before its result

## fun take_type

```mach
pub fun take_type(b: *Builder, r: res[ir_type.IrTypeId, fail.Fail]) ir_type.IrTypeId;
```

a type in sticky mode, the nil type standing in for a failure

## fun fault

```mach
pub fun fault(b: *Builder, f: fail.Fail);
```

records `f` unless an earlier failure is already the first

## fun done

```mach
pub fun done(b: *Builder, v: value.Value) res[value.Value, fail.Fail];
```

the end of a sticky run: `v`, or the run's first failure, which is cleared

## fun add

```mach
pub fun add(b: *Builder, x: value.Value, y: value.Value) value.Value;
```

## fun sub

```mach
pub fun sub(b: *Builder, x: value.Value, y: value.Value) value.Value;
```

## fun mul

```mach
pub fun mul(b: *Builder, x: value.Value, y: value.Value) value.Value;
```

## fun band

```mach
pub fun band(b: *Builder, x: value.Value, y: value.Value) value.Value;
```

## fun bor

```mach
pub fun bor(b: *Builder, x: value.Value, y: value.Value) value.Value;
```

## fun bxor

```mach
pub fun bxor(b: *Builder, x: value.Value, y: value.Value) value.Value;
```

## fun shl

```mach
pub fun shl(b: *Builder, x: value.Value, y: value.Value) value.Value;
```

## fun shr_u

```mach
pub fun shr_u(b: *Builder, x: value.Value, y: value.Value) value.Value;
```

## fun shr_s

```mach
pub fun shr_s(b: *Builder, x: value.Value, y: value.Value) value.Value;
```

## fun eq

```mach
pub fun eq(b: *Builder, x: value.Value, y: value.Value) value.Value;
```

## fun ne

```mach
pub fun ne(b: *Builder, x: value.Value, y: value.Value) value.Value;
```

## fun lt_s

```mach
pub fun lt_s(b: *Builder, x: value.Value, y: value.Value) value.Value;
```

## fun lt_u

```mach
pub fun lt_u(b: *Builder, x: value.Value, y: value.Value) value.Value;
```

## fun trunc

```mach
pub fun trunc(b: *Builder, x: value.Value, to: ir_type.IrTypeId) value.Value;
```

## fun zext

```mach
pub fun zext(b: *Builder, x: value.Value, to: ir_type.IrTypeId) value.Value;
```

## fun sext

```mach
pub fun sext(b: *Builder, x: value.Value, to: ir_type.IrTypeId) value.Value;
```

## fun bitcast

```mach
pub fun bitcast(b: *Builder, x: value.Value, to: ir_type.IrTypeId) value.Value;
```

## fun fp_ext

```mach
pub fun fp_ext(b: *Builder, x: value.Value, to: ir_type.IrTypeId) value.Value;
```

## fun ui_to_fp

```mach
pub fun ui_to_fp(b: *Builder, x: value.Value, to: ir_type.IrTypeId) value.Value;
```

## fun fp_to_ui

```mach
pub fun fp_to_ui(b: *Builder, x: value.Value, to: ir_type.IrTypeId) value.Value;
```

## fun call

```mach
pub fun call(b: *Builder, callee: u32, args: *value.Value, count: u32, ret_ty: ir_type.IrTypeId) value.Value;
```

a direct call of function `callee` of the module

