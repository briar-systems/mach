# mach.lang.me.ir.type

## def IrTypeId

```mach
pub def IrTypeId: u32
```

## val IRT_NIL

```mach
pub val IRT_NIL: IrTypeId = 0xFFFFFFFF
```

## def IrTypeKind

```mach
pub def IrTypeKind: u8
```

## val IRT_VOID

```mach
pub val IRT_VOID:   IrTypeKind = 0
```

## val IRT_INT

```mach
pub val IRT_INT:    IrTypeKind = 1
```

## val IRT_FLOAT

```mach
pub val IRT_FLOAT:  IrTypeKind = 2
```

## val IRT_PTR

```mach
pub val IRT_PTR:    IrTypeKind = 3
```

## val IRT_ARRAY

```mach
pub val IRT_ARRAY:  IrTypeKind = 4
```

## val IRT_STRUCT

```mach
pub val IRT_STRUCT: IrTypeKind = 5
```

## val IRT_FN

```mach
pub val IRT_FN:     IrTypeKind = 6
```

## val IRT_UNION

```mach
pub val IRT_UNION:  IrTypeKind = 7
```

## val IRT_VECTOR

```mach
pub val IRT_VECTOR: IrTypeKind = 8
```

## val IRT_HANDLE

```mach
pub val IRT_HANDLE: IrTypeKind = 9
```

## val IRT_TAG

```mach
pub val IRT_TAG:    IrTypeKind = 10
```

## rec IrTypeArray

```mach
pub rec IrTypeArray;
```

## rec IrTypeVector

```mach
pub rec IrTypeVector;
```

## rec IrTypeHandle

```mach
pub rec IrTypeHandle;
```

## rec IrTypeStruct

```mach
pub rec IrTypeStruct;
```

## rec IrTypeTag

```mach
pub rec IrTypeTag;
```

## rec IrTypeFn

```mach
pub rec IrTypeFn;
```

## rec IrType

```mach
pub rec IrType;
```

## rec IrTypeTable

```mach
pub rec IrTypeTable;
```

## fun init

```mach
pub fun init(t: *IrTypeTable, a: *A.Allocator) err[fail.Fail];
```

## fun dnit

```mach
pub fun dnit(t: *IrTypeTable);
```

## fun get

```mach
pub fun get(t: *IrTypeTable, id: IrTypeId) opt[*IrType];
```

## fun reference_valid

```mach
pub fun reference_valid(t: *IrTypeTable, id: IrTypeId) bool;
```

## fun array_count

```mach
pub fun array_count(t: *IrTypeTable, id: IrTypeId) u32;
```

## fun is_aggregate

```mach
pub fun is_aggregate(t: *IrTypeTable, id: IrTypeId) bool;
```

## fun is_float_scalar

```mach
pub fun is_float_scalar(t: *IrTypeTable, id: IrTypeId) bool;
```

## fun bit_width

```mach
pub fun bit_width(t: *IrTypeTable, id: IrTypeId) u32;
```

## fun is_vector

```mach
pub fun is_vector(t: *IrTypeTable, id: IrTypeId) bool;
```

## fun vector_lane

```mach
pub fun vector_lane(t: *IrTypeTable, id: IrTypeId, out_is_float: *bool, out_elem_bytes: *u8,
out_lanes: *u32) bool;
```

## fun is_tag

```mach
pub fun is_tag(t: *IrTypeTable, id: IrTypeId) bool;
```

## fun tag_case_count

```mach
pub fun tag_case_count(t: *IrTypeTable, id: IrTypeId) u32;
```

## fun tag_case_type

```mach
pub fun tag_case_type(t: *IrTypeTable, id: IrTypeId, case_ix: u32) IrTypeId;
```

## fun tag_case_has_payload

```mach
pub fun tag_case_has_payload(t: *IrTypeTable, id: IrTypeId, case_ix: u32) bool;
```

## fun tag_discriminator_bytes

```mach
pub fun tag_discriminator_bytes(t: *IrTypeTable, id: IrTypeId) u32;
```

## fun intern_void

```mach
pub fun intern_void(t: *IrTypeTable) res[IrTypeId, fail.Fail];
```

## fun intern_int

```mach
pub fun intern_int(t: *IrTypeTable, bits: u32) res[IrTypeId, fail.Fail];
```

## fun intern_float

```mach
pub fun intern_float(t: *IrTypeTable, bits: u32) res[IrTypeId, fail.Fail];
```

## fun intern_ptr

```mach
pub fun intern_ptr(t: *IrTypeTable) res[IrTypeId, fail.Fail];
```

## fun intern_reference

```mach
pub fun intern_reference(t: *IrTypeTable, pointee: IrTypeId) res[IrTypeId, fail.Fail];
```

## fun intern_array

```mach
pub fun intern_array(t: *IrTypeTable, element: IrTypeId, count: u32) res[IrTypeId, fail.Fail];
```

## fun intern_vector

```mach
pub fun intern_vector(t: *IrTypeTable, element: IrTypeId, lanes: u32) res[IrTypeId, fail.Fail];
```

## fun intern_handle

```mach
pub fun intern_handle(t: *IrTypeTable, ctor: u32, operands: *IrTypeId, count: u32) res[IrTypeId, fail.Fail];
```

## fun intern_struct

```mach
pub fun intern_struct(t: *IrTypeTable, fields: *IrTypeId, field_count: u32, align: u32, packed: bool) res[IrTypeId, fail.Fail];
```

## fun intern_union

```mach
pub fun intern_union(t: *IrTypeTable, fields: *IrTypeId, field_count: u32, align: u32, packed: bool) res[IrTypeId, fail.Fail];
```

## fun intern_fn

```mach
pub fun intern_fn(t: *IrTypeTable, ret_type: IrTypeId, params: *IrTypeId, param_count: u32, variadic: bool) res[IrTypeId, fail.Fail];
```

## fun intern_tag

```mach
pub fun intern_tag(t: *IrTypeTable, cases: *IrTypeId, case_count: u32, disc_bytes: u32, align: u32, packed: bool) res[IrTypeId, fail.Fail];
```

## fun byte_size_for_machine

```mach
pub fun byte_size_for_machine(t: *IrTypeTable, id: IrTypeId, machine: layout.Machine) u32;
```

## fun byte_align_for_machine

```mach
pub fun byte_align_for_machine(t: *IrTypeTable, id: IrTypeId, machine: layout.Machine) u32;
```

## fun byte_offset_for_machine

```mach
pub fun byte_offset_for_machine(t: *IrTypeTable, id: IrTypeId, field_ix: u32,
machine: layout.Machine) u32;
```

## fun tag_payload_offset_for_machine

```mach
pub fun tag_payload_offset_for_machine(t: *IrTypeTable, id: IrTypeId, machine: layout.Machine) u32;
```

## fun checked_extent_for_machine

```mach
pub fun checked_extent_for_machine(t: *IrTypeTable, id: IrTypeId, machine: layout.Machine) layout.Extent;
```

## fun checked_offset_for_machine

```mach
pub fun checked_offset_for_machine(t: *IrTypeTable, id: IrTypeId, field_ix: u32,
machine: layout.Machine) layout.Extent;
```

## fun content_equal

```mach
pub fun content_equal(a: *IrType, b: *IrType) bool;
```

