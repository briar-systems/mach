# mach.lang.target.isa.spirv.emit.arith

arithmetic, comparison, conversion and vector lane instructions

## fun chain_storage

```mach
pub fun chain_storage(vm: *emitter.VMaps, op: *lang_mir.MirOperand) u32;
```

## fun operand_matches_type

```mach
pub fun operand_matches_type(vm: *emitter.VMaps, op: *lang_mir.MirOperand, want_ty: u32) bool;
```

## fun store_vreg

```mach
pub fun store_vreg(e: *emitter.Emit, vm: *emitter.VMaps, vreg: u32, value: u32) err[fail.Fail];
```

## fun binary_emit

```mach
pub fun binary_emit(e: *emitter.Emit, vm: *emitter.VMaps, mi: *lang_mir.MirInstr, op_code: u32, shift: bool) err[fail.Fail];
```

`shift` names the second operand a shift count, which is typed by its own
operand rather than the result: a shift's count may be any integer width

## fun vector_shift_emit

```mach
pub fun vector_shift_emit(e: *emitter.Emit, vm: *emitter.VMaps, mi: *lang_mir.MirInstr, op_code: u32) err[fail.Fail];
```

a lane-wise shift saturates each lane as the scalar shift does, and
OpShift* leaves a component shifted by its width or more undefined.
the fill a lane saturates to is 0, or for an arithmetic shift the shift by
width - 1. a constant count is the bare shift, or the fill when it is out of
range. any other count keeps the shift where the count is below the width
and selects the fill elsewhere. a uniform count is one scalar, splatted
since OpShift* shifts by a count per component

## fun unary_emit

```mach
pub fun unary_emit(e: *emitter.Emit, vm: *emitter.VMaps, mi: *lang_mir.MirInstr, op_code: u32) err[fail.Fail];
```

## fun compare_emit

```mach
pub fun compare_emit(e: *emitter.Emit, vm: *emitter.VMaps, mi: *lang_mir.MirInstr, op_code: u32) err[fail.Fail];
```

## fun vec_extract_emit

```mach
pub fun vec_extract_emit(e: *emitter.Emit, vm: *emitter.VMaps, mi: *lang_mir.MirInstr) err[fail.Fail];
```

## fun vec_insert_emit

```mach
pub fun vec_insert_emit(e: *emitter.Emit, vm: *emitter.VMaps, mi: *lang_mir.MirInstr) err[fail.Fail];
```

## fun vec_build_emit

```mach
pub fun vec_build_emit(e: *emitter.Emit, vm: *emitter.VMaps, mi: *lang_mir.MirInstr) err[fail.Fail];
```

## fun float_compare_emit

```mach
pub fun float_compare_emit(e: *emitter.Emit, vm: *emitter.VMaps, mi: *lang_mir.MirInstr) err[fail.Fail];
```

## fun select_emit

```mach
pub fun select_emit(e: *emitter.Emit, vm: *emitter.VMaps, mi: *lang_mir.MirInstr) err[fail.Fail];
```

a select is OpSelect on a boolean: the folded compare's, a boolean
condition as it is, or an integer condition tested against zero

## fun compare_mask_emit

```mach
pub fun compare_mask_emit(e: *emitter.Emit, vm: *emitter.VMaps, mi: *lang_mir.MirInstr) err[fail.Fail];
```

the unsigned compare selects all ones or zero of the result type

## fun convert_emit

```mach
pub fun convert_emit(e: *emitter.Emit, mf: *lang_mir.MirFunction, vm: *emitter.VMaps, mi: *lang_mir.MirInstr,
op_code: u32, src_is_float: bool) err[fail.Fail];
```

## fun mov_emit

```mach
pub fun mov_emit(e: *emitter.Emit, vm: *emitter.VMaps, mi: *lang_mir.MirInstr) err[fail.Fail];
```

