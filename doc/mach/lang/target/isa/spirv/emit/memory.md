# mach.lang.target.isa.spirv.emit.memory

loads, stores, access chains and memory copies over the module's storage classes

## fun scalar_literal

```mach
pub fun scalar_literal(v: *value.Value, bits: u32) u64;
```

the bits of a scalar constant as a `bits`-wide literal

## fun output_initializer

```mach
pub fun output_initializer(e: *emitter.Emit, gt: *ir_type.IrTypeTable, g: *me_ir.Global, value_ty: u32) u32;
```

the constant an Output variable starts at: its initializer, spelled as
OpConstantNull where that is zero, which is also what an output declared without one
holds, as every mach `var` does. 0 where the initializer has no constant form

## fun mark_memory_scope

```mach
pub fun mark_memory_scope(e: *emitter.Emit, ptr: u32, scope: u32) err[fail.Fail];
```

## fun scope_of

```mach
pub fun scope_of(e: *emitter.Emit, ptr: u32) u32;
```

## fun chain_memory_scope

```mach
pub fun chain_memory_scope(e: *emitter.Emit, result: u32, base: u32);
```

an access chain's accesses share the scope of the pointer it walks from

## fun decorate

```mach
pub fun decorate(e: *emitter.Emit, target: u32, decor: u32, operand: u32);
```

## fun holds_physical

```mach
pub fun holds_physical(e: *emitter.Emit, ty: u32) bool;
```

## fun zero_value

```mach
pub fun zero_value(e: *emitter.Emit, ty: u32) u32;
```

the zero of a type, built from its parts where it holds a physical pointer

## fun store_emit

```mach
pub fun store_emit(e: *emitter.Emit, ptr: u32, value: u32, flags: u8);
```

## fun global_load_emit

```mach
pub fun global_load_emit(e: *emitter.Emit, mf: *lang_mir.MirFunction, vm: *emitter.VMaps, mi: *lang_mir.MirInstr) err[fail.Fail];
```

## fun global_store_emit

```mach
pub fun global_store_emit(e: *emitter.Emit, mf: *lang_mir.MirFunction, vm: *emitter.VMaps, mi: *lang_mir.MirInstr) err[fail.Fail];
```

## fun access_chain_emit

```mach
pub fun access_chain_emit(e: *emitter.Emit, mf: *lang_mir.MirFunction, vm: *emitter.VMaps,
mi: *lang_mir.MirInstr) err[fail.Fail];
```

## fun physical_value

```mach
pub fun physical_value(vm: *emitter.VMaps, op: *lang_mir.MirOperand) u32;
```

the physical pointer register an address operand holds, the register itself or the
memory at it, MIR_VREG_NIL when it holds none

## fun address

```mach
pub fun address(e: *emitter.Emit, vm: *emitter.VMaps, op: *lang_mir.MirOperand, out_ty: *u32, out_sc: *u32) u32;
```

the pointer an address operand names, with its pointee and storage class: an access
chain's, or the physical pointer a register holds, read here

## fun memzero_emit

```mach
pub fun memzero_emit(e: *emitter.Emit, mf: *lang_mir.MirFunction, vm: *emitter.VMaps, mi: *lang_mir.MirInstr) err[fail.Fail];
```

## fun memcpy_emit

```mach
pub fun memcpy_emit(e: *emitter.Emit, mf: *lang_mir.MirFunction, vm: *emitter.VMaps, mi: *lang_mir.MirInstr) err[fail.Fail];
```

## fun read_value

```mach
pub fun read_value(e: *emitter.Emit, mf: *lang_mir.MirFunction, vm: *emitter.VMaps, op: *lang_mir.MirOperand,
want_ty: u32, flags: u8) res[u32, fail.Fail];
```

## fun write_value

```mach
pub fun write_value(e: *emitter.Emit, mf: *lang_mir.MirFunction, vm: *emitter.VMaps, op: *lang_mir.MirOperand,
held: u32, held_ty: u32, flags: u8) err[fail.Fail];
```

## fun param_emit

```mach
pub fun param_emit(e: *emitter.Emit, mf: *lang_mir.MirFunction, vm: *emitter.VMaps, mi: *lang_mir.MirInstr) err[fail.Fail];
```

## fun path_root_type

```mach
pub fun path_root_type(e: *emitter.Emit, at: u32) u32;
```

## fun constant_copy

```mach
pub fun constant_copy(e: *emitter.Emit, mi: *lang_mir.MirInstr, ty: u32) u32;
```

the constant of `ty` the copy `mi` reads, 0 unless it copies the whole of a module-scope `val`

## fun constant_global

```mach
pub fun constant_global(e: *emitter.Emit, sym: intern.StrId, want_ty: u32) u32;
```

the constant id of `want_ty` a module-scope `val` named `sym` spells, 0 when it is none:
such a value reaches a shader only as a constant, never as a variable it addresses

## fun constant_load

```mach
pub fun constant_load(e: *emitter.Emit, op: *lang_mir.MirOperand, want_ty: u32) u32;
```

the constant a whole-value load from `op` reads, 0 unless it names a module-scope `val`

