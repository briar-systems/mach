# mach.lang.target.isa.spirv.emit.builtin

the builtin variables a stage reads or writes: their types, stages and capabilities

## fun type_refusal

```mach
pub fun type_refusal(tt: *ir_type.IrTypeTable, alloc: *A.Allocator, which: u32,
ty: ir_type.IrTypeId) res[str, fail.Fail];
```

why a `#[builtin]` variable's type is not the one SPIR-V specifies, nil when it is

## fun spelling_of

```mach
pub fun spelling_of(which: u32) str;
```

## fun ir_scalar_shape

```mach
pub fun ir_scalar_shape(tt: *ir_type.IrTypeTable, id: ir_type.IrTypeId, out_lanes: *u32,
out_bits: *u32, out_is_float: *bool) bool;
```

## fun ir_type_spelling

```mach
pub fun ir_type_spelling(tt: *ir_type.IrTypeTable, alloc: *A.Allocator, id: ir_type.IrTypeId) res[str, fail.Fail];
```

## fun selector

```mach
pub fun selector(which: u32) u32;
```

## fun stage_bit

```mach
pub fun stage_bit(stage: u8) u8;
```

a set of pipeline stages, one bit per ir stage

## val STAGES_GRAPHICS

```mach
pub val STAGES_GRAPHICS: u8 = 0x06
```

## fun capability

```mach
pub fun capability(which: u32) u32;
```

the capability a built-in needs declared, NO_OP_CAPABILITY for one the Shader capability covers

## fun stages_spelling

```mach
pub fun stages_spelling(e: *emitter.Emit, stages: u8) res[str, fail.Fail];
```

"compute", "vertex or fragment"

## fun stage_refusal

```mach
pub fun stage_refusal(e: *emitter.Emit, fx: u32, gi: u32) res[str, fail.Fail];
```

the refusal of a built-in used by a stage SPIR-V does not define it in, nil when it does

## fun storage

```mach
pub fun storage(which: u32) u32;
```

