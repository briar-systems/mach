# mach.lang.target.isa.spirv.emit.value

the value maps of one function: which SPIR-V id and type each virtual register holds

## fun param_is_pointer

```mach
pub fun param_is_pointer(e: *emitter.Emit, fx: u32, k: u32) bool;
```

## fun key_param

```mach
pub fun key_param(e: *emitter.Emit, at: u32, k: u32) u32;
```

the offset in `keys` of the binding of source parameter `k` in the key at `at`

## fun instr_defines_vreg

```mach
pub fun instr_defines_vreg(mi: *lang_mir.MirInstr, v: u32) bool;
```

## fun call_op_row_in

```mach
pub fun call_op_row_in(e: *emitter.Emit, m: *me_ir.Module, mi: *lang_mir.MirInstr) *target_definition.Op;
```

## fun decorate_bare

```mach
pub fun decorate_bare(e: *emitter.Emit, target: u32, decor: u32);
```

## fun build_vmaps

```mach
pub fun build_vmaps(e: *emitter.Emit, mf: *lang_mir.MirFunction, irfn: *me_ir.Function, out: *emitter.VMaps) err[fail.Fail];
```

## fun vreg_type

```mach
pub fun vreg_type(mf: *lang_mir.MirFunction, v: u32) ir_type.IrTypeId;
```

the IR type of vreg `v`, IRT_NIL for one that carries none

## fun prov_of

```mach
pub fun prov_of(prov: *u8, n: u32, op: *lang_mir.MirOperand) u8;
```

## fun classify_pointers

```mach
pub fun classify_pointers(e: *emitter.Emit, cx: u32, mf: *lang_mir.MirFunction, prov: *u8);
```

where each pointer register of copy `cx` points, from its definitions: a local, an
interface variable and a logically bound parameter name declared memory, and so does
an access chain into one. a pointer read from memory, made from an address, returned
by a function or bound as a physical parameter is a physical pointer, since memory
holds no logical one. a copy or a select takes its operands' provenance, so a register
both reach is one the emitter refuses. a pointer only ever `nil` is physical

## fun find_slot

```mach
pub fun find_slot(mf: *lang_mir.MirFunction, key: u32) *lang_mir.MirSlot;
```

## fun is_reg_copy

```mach
pub fun is_reg_copy(mi: *lang_mir.MirInstr) bool;
```

## fun vmaps_dnit

```mach
pub fun vmaps_dnit(e: *emitter.Emit, vm: *emitter.VMaps);
```

## fun vector_type_of

```mach
pub fun vector_type_of(e: *emitter.Emit, lane: u32, as_int: bool) u32;
```

## fun value_spv_type

```mach
pub fun value_spv_type(e: *emitter.Emit, tys: *ir_type.IrTypeTable, id: ir_type.IrTypeId) u32;
```

the spirv type a value of an ir type has in a function: a scalar integer, and each
integer lane of a vector, at its carrier, an f16 without float16 included since it is
carried as its bits, a pointer to one pointing at the carrier, and every
other type as declared, since a composite member keeps its width wherever it lives

## fun narrow_extend

```mach
pub fun narrow_extend(e: *emitter.Emit, value: u32, ty: u32, bits: u32, signed: bool) u32;
```

a carried integer, or a vector of them lane for lane, read as one of `bits`: the
arithmetic leaves the carrier's upper bits unspecified, so the value is zero- or
sign-extended from `bits` wherever they would be observed. a value at its own width
is as it is

## fun read_narrow

```mach
pub fun read_narrow(e: *emitter.Emit, vm: *emitter.VMaps, op: *lang_mir.MirOperand, ty: u32, bits: u32, signed: bool) u32;
```

an integer operand read at `bits`, in the carrier of that width

## fun ir_value_spv_type

```mach
pub fun ir_value_spv_type(e: *emitter.Emit, id: ir_type.IrTypeId) u32;
```

## fun global_spv_type

```mach
pub fun global_spv_type(e: *emitter.Emit, gt: *ir_type.IrTypeTable, id: ir_type.IrTypeId) u32;
```

## fun op_signed_data

```mach
pub fun op_signed_data(e: *emitter.Emit, vm: *emitter.VMaps, mi: *lang_mir.MirInstr, row: *target_definition.Op, sig: *ir_type.IrType) bool;
```

whether an `op` call's data type is signed: the sampled type of an image of a signed
texel class, which a texel pointer into one points to. its related values are the
unsigned integers the module computes on, so they cross into the instruction bitcast

## fun op_relation_is_data

```mach
pub fun op_relation_is_data(rel: target_definition.OpRelation) bool;
```

whether relation `rel` ties a value to an `op` call's data type

## fun ir_return_spv_type

```mach
pub fun ir_return_spv_type(e: *emitter.Emit, irfn: *me_ir.Function, out_void: *bool) u32;
```

## fun unsupported_value_type

```mach
pub fun unsupported_value_type(e: *emitter.Emit, mi: *lang_mir.MirInstr) str;
```

## fun leading_leaf_type

```mach
pub fun leading_leaf_type(e: *emitter.Emit, ty: u32) u32;
```

## fun interface_value_type

```mach
pub fun interface_value_type(e: *emitter.Emit, op: *lang_mir.MirOperand) u32;
```

## fun chain_target

```mach
pub fun chain_target(vm: *emitter.VMaps, op: *lang_mir.MirOperand, out_ty: *u32) u32;
```

## fun sampled_image_handle

```mach
pub fun sampled_image_handle(e: *emitter.Emit, param: ir_type.IrTypeId) *ir_type.IrType;
```

the `image` handle type a `sampled_image` handle type `param` composes over, nil when it is none

## fun image_handle

```mach
pub fun image_handle(e: *emitter.Emit, param: ir_type.IrTypeId) *ir_type.IrType;
```

the `image` handle type `param` is, nil when it is no image

## fun ir_function_record

```mach
pub fun ir_function_record(irmod: *me_ir.Module, name: intern.StrId) *me_ir.Function;
```

## fun read_operand

```mach
pub fun read_operand(e: *emitter.Emit, vm: *emitter.VMaps, op: *lang_mir.MirOperand, want_ty: u32) u32;
```

## fun immediate

```mach
pub fun immediate(e: *emitter.Emit, want_ty: u32, imm: i64) u32;
```

## fun physical_type

```mach
pub fun physical_type(e: *emitter.Emit, ty: u32) bool;
```

## fun pointer_constant

```mach
pub fun pointer_constant(e: *emitter.Emit, ty: u32, imm: i64) u32;
```

a physical pointer constant, `nil` or an integer literal, made from the address it is

## fun mask_splat

```mach
pub fun mask_splat(e: *emitter.Emit, vec_ty: u32, lanes: u32, lo: u32, hi: u32) u32;
```

