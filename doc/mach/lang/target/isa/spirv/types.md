# mach.lang.target.isa.spirv.types

## val MAX_VECTOR_COMPONENTS

```mach
pub val MAX_VECTOR_COMPONENTS: u32 = 4
```

## val MAX_STRUCT_MEMBERS

```mach
pub val MAX_STRUCT_MEMBERS: u32 = 16
```

## rec TypeTable

```mach
pub rec TypeTable;
```

## fun types_init

```mach
pub fun types_init(b: *spirv.Builder) TypeTable;
```

## fun types_dnit

```mach
pub fun types_dnit(tt: *TypeTable);
```

## fun composite_member

```mach
pub fun composite_member(tt: *TypeTable, id: u32, index: u32) u32;
```

## fun logically_match

```mach
pub fun logically_match(tt: *TypeTable, a: u32, b: u32) bool;
```

## fun type_int

```mach
pub fun type_int(tt: *TypeTable, bits: u32) u32;
```

## fun type_float

```mach
pub fun type_float(tt: *TypeTable, bits: u32) u32;
```

## fun type_bool

```mach
pub fun type_bool(tt: *TypeTable) u32;
```

## fun type_void

```mach
pub fun type_void(tt: *TypeTable) u32;
```

## fun type_vector

```mach
pub fun type_vector(tt: *TypeTable, elem: u32, count: u32) u32;
```

## fun vector_shape

```mach
pub fun vector_shape(tt: *TypeTable, id: u32, out_count: *u32) u32;
```

## fun type_vector_value

```mach
pub fun type_vector_value(tt: *TypeTable, elem: u32, count: u32) u32;
```

## fun vector_value_shape

```mach
pub fun vector_value_shape(tt: *TypeTable, id: u32, out_count: *u32) u32;
```

## fun type_array

```mach
pub fun type_array(tt: *TypeTable, elem: u32, count: u32, explicit_layout: bool) u32;
```

## fun type_struct

```mach
pub fun type_struct(tt: *TypeTable, members: *u32, n: u32, explicit_layout: bool) u32;
```

## fun type_tag_struct

```mach
pub fun type_tag_struct(tt: *TypeTable, members: *u32, n: u32) u32;
```

a tag's composite: member 0 is the declared discriminator and member 1 + c is case c's payload
(the unit composite when the case has none). it is interned apart from a record with the
same member list so the emitter can map a case index to its ordinal from the type alone.

## fun struct_is_tag

```mach
pub fun struct_is_tag(tt: *TypeTable, id: u32) bool;
```

## fun type_unit

```mach
pub fun type_unit(tt: *TypeTable) u32;
```

the empty composite a payloadless case holds

## fun is_composite

```mach
pub fun is_composite(tt: *TypeTable, id: u32) bool;
```

a struct or array: something an access chain descends into

## fun type_image

```mach
pub fun type_image(tt: *TypeTable, sampled: u32, dim: u32, depth: u32, arrayed: u32,
ms: u32, sampled_op: u32) u32;
```

## fun type_sampled_image

```mach
pub fun type_sampled_image(tt: *TypeTable, image: u32) u32;
```

## fun type_sampler

```mach
pub fun type_sampler(tt: *TypeTable) u32;
```

## fun type_is_opaque

```mach
pub fun type_is_opaque(tt: *TypeTable, id: u32) bool;
```

## fun type_ptr

```mach
pub fun type_ptr(tt: *TypeTable, storage: u32, pointee: u32) u32;
```

## fun pointer_shape

```mach
pub fun pointer_shape(tt: *TypeTable, id: u32, storage: *u32) u32;
```

## fun instruction_operands_fit

```mach
pub fun instruction_operands_fit(fixed: u32, variable: u32) bool;
```

## fun type_fn

```mach
pub fun type_fn(tt: *TypeTable, ret_id: u32, params: *u32, n: u32) u32;
```

## fun type_bits

```mach
pub fun type_bits(tt: *TypeTable, id: u32) u32;
```

## fun type_is_float

```mach
pub fun type_is_float(tt: *TypeTable, id: u32) bool;
```

## fun const_scalar

```mach
pub fun const_scalar(tt: *TypeTable, type_id: u32, lo: u32, hi: u32) u32;
```

## fun const_composite

```mach
pub fun const_composite(tt: *TypeTable, type_id: u32, parts: *u32, n: u32) u32;
```

## fun type_is_bool

```mach
pub fun type_is_bool(tt: *TypeTable, id: u32) bool;
```

## fun const_bool

```mach
pub fun const_bool(tt: *TypeTable, value: bool) u32;
```

## fun const_null

```mach
pub fun const_null(tt: *TypeTable, type_id: u32) u32;
```

## fun const_int

```mach
pub fun const_int(tt: *TypeTable, bits: u32, value: i64) u32;
```

## fun const_float

```mach
pub fun const_float(tt: *TypeTable, bits: u32, pattern: u64) u32;
```

