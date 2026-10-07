# mach.lang.target.isa.spirv.emit.instruction

one MIR instruction emitted, and the calls between functions

## fun is_image_type

```mach
pub fun is_image_type(e: *emitter.Emit, gt: *ir_type.IrTypeTable, id: ir_type.IrTypeId) bool;
```

## fun require_wide_texels

```mach
pub fun require_wide_texels(e: *emitter.Emit, gt: *ir_type.IrTypeTable, gx: u32) err[fail.Fail];
```

an image of 64-bit texels is declared under Int64ImageEXT, which Vulkan enables only with
the shaderImageInt64Atomics feature

## fun handle_role

```mach
pub fun handle_role(e: *emitter.Emit, gt: *ir_type.IrTypeTable, id: ir_type.IrTypeId) u8;
```

the interface role a handle type binds through: a storage image `#[storage]`, any other handle `#[sampler]`

## fun instr_emit

```mach
pub fun instr_emit(e: *emitter.Emit, mf: *lang_mir.MirFunction, vm: *emitter.VMaps, mi: *lang_mir.MirInstr) err[fail.Fail];
```

## fun pointer_operand

```mach
pub fun pointer_operand(vm: *emitter.VMaps, op: *lang_mir.MirOperand) bool;
```

a register that holds a pointer, logical or physical

## fun path_param_types

```mach
pub fun path_param_types(e: *emitter.Emit, at: u32, out: *u32);
```

the SPIR-V parameter types a path binding takes: its root when Function-storage, then u32 indices

