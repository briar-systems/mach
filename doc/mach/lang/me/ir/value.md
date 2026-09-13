# mach.lang.me.ir.value

## def ValueKind

```mach
pub def ValueKind: u8
```

## val VAL_INSTR

```mach
pub val VAL_INSTR:       ValueKind = 0
```

## val VAL_PARAM

```mach
pub val VAL_PARAM:       ValueKind = 1
```

## val VAL_CONST_INT

```mach
pub val VAL_CONST_INT:   ValueKind = 2
```

## val VAL_CONST_FLOAT

```mach
pub val VAL_CONST_FLOAT: ValueKind = 3
```

## val VAL_CONST_NULL

```mach
pub val VAL_CONST_NULL:  ValueKind = 4
```

## val VAL_CONST_BYTES

```mach
pub val VAL_CONST_BYTES: ValueKind = 5
```

## val VAL_GLOBAL

```mach
pub val VAL_GLOBAL:      ValueKind = 6
```

## val VAL_FN

```mach
pub val VAL_FN:          ValueKind = 7
```

## val VAL_CONST_AGG

```mach
pub val VAL_CONST_AGG:   ValueKind = 8
```

## rec ValueBytes

```mach
pub rec ValueBytes;
```

## rec AggReloc

```mach
pub rec AggReloc;
```

## rec ValueAgg

```mach
pub rec ValueAgg;
```

## rec Value

```mach
pub rec Value;
```

## fun instr

```mach
pub fun instr(instr: id.InstructionId, ty: ir_type.IrTypeId) Value;
```

## fun param

```mach
pub fun param(index: u32, ty: ir_type.IrTypeId) Value;
```

## fun const_int

```mach
pub fun const_int(bits: u64, ty: ir_type.IrTypeId) Value;
```

## fun const_float

```mach
pub fun const_float(value: f64, ty: ir_type.IrTypeId) Value;
```

## fun const_null

```mach
pub fun const_null(ty: ir_type.IrTypeId) Value;
```

## fun const_bytes

```mach
pub fun const_bytes(ptr: *u8, len: u32, ty: ir_type.IrTypeId) Value;
```

## fun global

```mach
pub fun global(index: u32, ty: ir_type.IrTypeId) Value;
```

## fun fn

```mach
pub fun fn(index: u32, ty: ir_type.IrTypeId) Value;
```

## fun const_agg

```mach
pub fun const_agg(bytes: *u8, len: u32, relocs: *AggReloc, reloc_count: u32, reloc_cap: u32, ty: ir_type.IrTypeId) Value;
```

## fun retype

```mach
pub fun retype(v: Value, ty: ir_type.IrTypeId) Value;
```

## fun is_constant

```mach
pub fun is_constant(v: Value) bool;
```

