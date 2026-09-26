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

## def IntExt

```mach
pub def IntExt: u8
```

the extension a caller owes an integer argument narrower than 64 bits where
its platform asks the caller for one: the declared signedness the signless
integer type does not carry, as LLVM's zeroext and signext (#3927)

## val EXT_NONE

```mach
pub val EXT_NONE: IntExt = 0
```

## val EXT_ZERO

```mach
pub val EXT_ZERO: IntExt = 1
```

## val EXT_SIGN

```mach
pub val EXT_SIGN: IntExt = 2
```

## def ExtListId

```mach
pub def ExtListId: u32
```

an interned list of one IntExt per parameter, carried by a function's
declaration and by each call rather than by the signless function type.
trailing EXT_NONE entries are dropped, so a list with none is EXT_LIST_NONE

## val EXT_LIST_NONE

```mach
pub val EXT_LIST_NONE: ExtListId = 0
```

## rec ExtList

```mach
pub rec ExtList;
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

## fun intern_ext_list

```mach
pub fun intern_ext_list(t: *IrTypeTable, ext: *IntExt, count: u32) res[ExtListId, fail.Fail];
```

the list id of `count` extensions, one per parameter in order

## fun ext_at

```mach
pub fun ext_at(t: *IrTypeTable, xid: ExtListId, index: u32) IntExt;
```

the extension of parameter `index` in list `xid`: EXT_NONE past its end

## fun copy_ext_list

```mach
pub fun copy_ext_list(dst: *IrTypeTable, src: *IrTypeTable, xid: ExtListId) res[ExtListId, fail.Fail];
```

the list `xid` of table `src` interned in `dst`

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

## fun tag_disc_field

```mach
pub fun tag_disc_field() u32;
```

a tag is addressed like a struct: field 0 is the discriminator and field
k + 1 is the payload of case k, so a gep into a tag needs no spelling of
its own. every case payload shares the payload offset

## fun tag_case_field

```mach
pub fun tag_case_field(case_ix: u32) u32;
```

## fun tag_field_count

```mach
pub fun tag_field_count(t: *IrTypeTable, id: IrTypeId) u32;
```

## fun tag_disc_type

```mach
pub fun tag_disc_type(t: *IrTypeTable, id: IrTypeId) IrTypeId;
```

## fun tag_field_type

```mach
pub fun tag_field_type(t: *IrTypeTable, id: IrTypeId, field_ix: u32) IrTypeId;
```

## fun tag_field_offset_for_machine

```mach
pub fun tag_field_offset_for_machine(t: *IrTypeTable, id: IrTypeId, field_ix: u32, machine: layout.Machine) u32;
```

## fun member_offset_for_machine

```mach
pub fun member_offset_for_machine(t: *IrTypeTable, id: IrTypeId, member_ix: u32, machine: layout.Machine) u32;
```

the byte offset of a declared member as the source numbers it: a struct
field by its index, a tag case by its case index

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

## fun natural_align_for_machine

```mach
pub fun natural_align_for_machine(t: *IrTypeTable, id: IrTypeId, machine: layout.Machine) u32;
```

the alignment an aggregate's members give it, before its own #[align] raises
it: a member's alignment counts whole, #[align] included, and a packed
aggregate's members are aligned to 1. anything else has its own alignment.
AAPCS64 places a composite argument by this one (#3929)

## fun byte_offset_for_machine

```mach
pub fun byte_offset_for_machine(t: *IrTypeTable, id: IrTypeId, field_ix: u32,
machine: layout.Machine) u32;
```

the byte offset of an aggregate's field as a gep addresses it; for a tag
that is the discriminator at field 0 and the shared payload offset for
every case field after it

## fun tag_payload_offset_for_machine

```mach
pub fun tag_payload_offset_for_machine(t: *IrTypeTable, id: IrTypeId, machine: layout.Machine) u32;
```

## fun content_equal

```mach
pub fun content_equal(a: *IrType, b: *IrType) bool;
```

