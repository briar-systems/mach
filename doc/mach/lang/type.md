# mach.lang.type

## def TypeId

```mach
pub def TypeId: u32
```

## val TYPE_NIL

```mach
pub val TYPE_NIL:   TypeId = 0xFFFFFFFF
```

## val TYPE_ERROR

```mach
pub val TYPE_ERROR: TypeId = 0xFFFFFFFE
```

## def TypeKind

```mach
pub def TypeKind: u8
```

## val TYPE_U8

```mach
pub val TYPE_U8:    TypeKind = 0
```

## val TYPE_U16

```mach
pub val TYPE_U16:   TypeKind = 1
```

## val TYPE_U32

```mach
pub val TYPE_U32:   TypeKind = 2
```

## val TYPE_U64

```mach
pub val TYPE_U64:   TypeKind = 3
```

## val TYPE_I8

```mach
pub val TYPE_I8:    TypeKind = 4
```

## val TYPE_I16

```mach
pub val TYPE_I16:   TypeKind = 5
```

## val TYPE_I32

```mach
pub val TYPE_I32:   TypeKind = 6
```

## val TYPE_I64

```mach
pub val TYPE_I64:   TypeKind = 7
```

## val TYPE_F32

```mach
pub val TYPE_F32:   TypeKind = 8
```

## val TYPE_F64

```mach
pub val TYPE_F64:   TypeKind = 9
```

## val TYPE_PTR

```mach
pub val TYPE_PTR:   TypeKind = 10
```

## val PRIM_COUNT

```mach
pub val PRIM_COUNT: u32      = 11
```

## val TYPE_POINTER

```mach
pub val TYPE_POINTER:       TypeKind = 11
```

## val TYPE_ARRAY

```mach
pub val TYPE_ARRAY:         TypeKind = 12
```

## val TYPE_FUN

```mach
pub val TYPE_FUN:           TypeKind = 13
```

## val TYPE_REC

```mach
pub val TYPE_REC:           TypeKind = 14
```

## val TYPE_UNI

```mach
pub val TYPE_UNI:           TypeKind = 15
```

## val TYPE_GENERIC_PARAM

```mach
pub val TYPE_GENERIC_PARAM: TypeKind = 16
```

## val TYPE_INSTANCE

```mach
pub val TYPE_INSTANCE:      TypeKind = 17
```

## val TYPE_PACK

```mach
pub val TYPE_PACK: TypeKind = 18
```

## val TYPE_SECRET

```mach
pub val TYPE_SECRET: TypeKind = 19
```

## val TYPE_VECTOR

```mach
pub val TYPE_VECTOR: TypeKind = 20
```

## val VEC_COUNT

```mach
pub val VEC_COUNT: u32 = 10
```

## val TYPE_HANDLE

```mach
pub val TYPE_HANDLE: TypeKind = 21
```

## val TYPE_ABI

```mach
pub val TYPE_ABI: TypeKind = 22
```

## val TYPE_TAG

```mach
pub val TYPE_TAG: TypeKind = 23
```

## val TYPE_CASE_SELECTOR

```mach
pub val TYPE_CASE_SELECTOR: TypeKind = 24
```

## def PrimClass

```mach
pub def PrimClass: u8
```

## val PRIM_CLASS_NONE

```mach
pub val PRIM_CLASS_NONE:  PrimClass = 0
```

## val PRIM_CLASS_INT

```mach
pub val PRIM_CLASS_INT:   PrimClass = 1
```

## val PRIM_CLASS_FLOAT

```mach
pub val PRIM_CLASS_FLOAT: PrimClass = 2
```

## val PRIM_CLASS_PTR

```mach
pub val PRIM_CLASS_PTR:   PrimClass = 3
```

## rec PrimDesc

```mach
pub rec PrimDesc;
```

## fun prim_desc

```mach
pub fun prim_desc(kind: TypeKind) PrimDesc;
```

## rec TypePointer

```mach
pub rec TypePointer;
```

## rec TypeSecret

```mach
pub rec TypeSecret;
```

## rec TypeVector

```mach
pub rec TypeVector;
```

## rec TypeHandle

```mach
pub rec TypeHandle;
```

## rec TypeAbi

```mach
pub rec TypeAbi;
```

## val ABI_TYPE_VA_LIST

```mach
pub val ABI_TYPE_VA_LIST: u32 = 0
```

## rec VecShape

```mach
pub rec VecShape;
```

## fun vec_shape

```mach
pub fun vec_shape(index: u32) VecShape;
```

## fun prim_name

```mach
pub fun prim_name(kind: TypeKind) str;
```

## fun prim_spelling_len

```mach
pub fun prim_spelling_len(kind: TypeKind) usize;
```

## fun prim_from_name

```mach
pub fun prim_from_name(name: str) opt[TypeKind];
```

## fun prim_from_view

```mach
pub fun prim_from_view(name: View) opt[TypeKind];
```

## rec IntRange

```mach
pub rec IntRange;
```

## fun int_range

```mach
pub fun int_range(kind: TypeKind) opt[IntRange];
```

## fun int_fits

```mach
pub fun int_fits(value: u64, negated: bool, r: IntRange) bool;
```

## fun int_fits_either_sign

```mach
pub fun int_fits_either_sign(value: u64, kind: TypeKind) bool;
```

## fun float_width_of

```mach
pub fun float_width_of(kind: TypeKind) float.FloatWidth;
```

the width of a float kind; FLOAT_W_NONE for every other kind (a partition,
enumerated by its test)

## def VecFormStatus

```mach
pub def VecFormStatus: u8
```

## val MAX_VEC_LANES

```mach
pub val MAX_VEC_LANES: u32 = 65535
```

## val VEC_FORM_NONE

```mach
pub val VEC_FORM_NONE:      VecFormStatus = 0
```

## val VEC_FORM_OK

```mach
pub val VEC_FORM_OK:        VecFormStatus = 1
```

## val VEC_FORM_TOO_WIDE

```mach
pub val VEC_FORM_TOO_WIDE:  VecFormStatus = 2
```

## val VEC_FORM_BAD_LANES

```mach
pub val VEC_FORM_BAD_LANES: VecFormStatus = 3
```

## rec VecForm

```mach
pub rec VecForm;
```

## fun is_vector_spelling

```mach
pub fun is_vector_spelling(name: str) bool;
```

## fun vector_form

```mach
pub fun vector_form(name: str) VecForm;
```

## rec TypeArray

```mach
pub rec TypeArray;
```

## rec TypeFun

```mach
pub rec TypeFun;
```

## rec TypeOwner

```mach
pub rec TypeOwner;
```

persistent source identity, independent of the current module registry

## rec TypeNominal

```mach
pub rec TypeNominal;
```

## rec TypeGenericParam

```mach
pub rec TypeGenericParam;
```

## rec TypeInstance

```mach
pub rec TypeInstance;
```

## rec TypeCaseSelector

```mach
pub rec TypeCaseSelector;
```

## rec Type

```mach
pub rec Type;
```

## rec FieldEntry

```mach
pub rec FieldEntry;
```

## rec FieldTable

```mach
pub rec FieldTable;
```

## rec TypeInterner

```mach
pub rec TypeInterner;
```

## fun init

```mach
pub fun init(ti: *TypeInterner, a: *A.Allocator) err[fail.Fail];
```

## fun dnit

```mach
pub fun dnit(ti: *TypeInterner);
```

## fun get

```mach
pub fun get(ti: *TypeInterner, tid: TypeId) opt[*Type];
```

## fun type_count

```mach
pub fun type_count(ti: *TypeInterner) u32;
```

## fun get_param

```mach
pub fun get_param(ti: *TypeInterner, index: u32) opt[TypeId];
```

## fun prim

```mach
pub fun prim(ti: *TypeInterner, kind: TypeKind) opt[TypeId];
```

## fun is_integer

```mach
pub fun is_integer(ti: *TypeInterner, tid: TypeId) bool;
```

## fun is_numeric

```mach
pub fun is_numeric(ti: *TypeInterner, tid: TypeId) bool;
```

## fun is_float

```mach
pub fun is_float(ti: *TypeInterner, tid: TypeId) bool;
```

## fun is_pointer_like

```mach
pub fun is_pointer_like(ti: *TypeInterner, tid: TypeId) bool;
```

## fun is_pointer

```mach
pub fun is_pointer(ti: *TypeInterner, tid: TypeId) bool;
```

## fun deref_one

```mach
pub fun deref_one(ti: *TypeInterner, tid: TypeId) TypeId;
```

## fun is_secret

```mach
pub fun is_secret(ti: *TypeInterner, tid: TypeId) bool;
```

## fun is_vector

```mach
pub fun is_vector(ti: *TypeInterner, tid: TypeId) bool;
```

## fun vector_element

```mach
pub fun vector_element(ti: *TypeInterner, tid: TypeId) TypeKind;
```

## fun vector_lanes

```mach
pub fun vector_lanes(ti: *TypeInterner, tid: TypeId) u32;
```

## fun element_count

```mach
pub fun element_count(ti: *TypeInterner, tid: TypeId) opt[u32];
```

## fun vec

```mach
pub fun vec(ti: *TypeInterner, index: u32) opt[TypeId];
```

## fun vector_mask

```mach
pub fun vector_mask(ti: *TypeInterner, tid: TypeId) res[TypeId, fail.Fail];
```

## fun strip_secret

```mach
pub fun strip_secret(ti: *TypeInterner, tid: TypeId) TypeId;
```

## fun shape_kind

```mach
pub fun shape_kind(ti: *TypeInterner, tid: TypeId) TypeKind;
```

## fun is_u8

```mach
pub fun is_u8(ti: *TypeInterner, tid: TypeId) bool;
```

## fun carries_secret

```mach
pub fun carries_secret(ti: *TypeInterner, tid: TypeId) bool;
```

## fun contains_secret

```mach
pub fun contains_secret(ti: *TypeInterner, tid: TypeId) bool;
```

true when any byte of a value of this type may be secret: the type itself, an element, a field or a case
payload; a pointer's pointee is not the object's storage and does not count

## rec TypeScan

```mach
pub rec TypeScan;
```

## def VisitMark

```mach
pub def VisitMark: u8
```

## val VISIT_FRESH

```mach
pub val VISIT_FRESH: VisitMark = 0
```

## val VISIT_SEEN

```mach
pub val VISIT_SEEN:  VisitMark = 1
```

## fun type_scan_begin

```mach
pub fun type_scan_begin(ti: *TypeInterner, scan: *TypeScan) err[fail.Fail];
```

## fun type_scan_end

```mach
pub fun type_scan_end(ti: *TypeInterner, scan: *TypeScan);
```

## fun type_scan_mark

```mach
pub fun type_scan_mark(ti: *TypeInterner, scan: *TypeScan, tid: TypeId) res[VisitMark, fail.Fail];
```

## rec PairScan

```mach
pub rec PairScan;
```

## fun pair_scan_begin

```mach
pub fun pair_scan_begin(ti: *TypeInterner, scan: *PairScan) err[fail.Fail];
```

## fun pair_scan_end

```mach
pub fun pair_scan_end(ti: *TypeInterner, scan: *PairScan);
```

## fun pair_scan_mark

```mach
pub fun pair_scan_mark(ti: *TypeInterner, scan: *PairScan, a: TypeId, b: TypeId) res[VisitMark, fail.Fail];
```

## def GParamMemo

```mach
pub def GParamMemo: u8
```

## val GPARAM_UNKNOWN

```mach
pub val GPARAM_UNKNOWN: GParamMemo = 0
```

## val GPARAM_NO

```mach
pub val GPARAM_NO:      GParamMemo = 1
```

## val GPARAM_YES

```mach
pub val GPARAM_YES:     GParamMemo = 2
```

## fun gparam_memo_get

```mach
pub fun gparam_memo_get(ti: *TypeInterner, tid: TypeId) GParamMemo;
```

## fun gparam_memo_set

```mach
pub fun gparam_memo_set(ti: *TypeInterner, tid: TypeId, mentions: bool);
```

## fun secrecy_structure_equal

```mach
pub fun secrecy_structure_equal(ti: *TypeInterner, a: TypeId, b: TypeId) bool;
```

## fun intern_pointer

```mach
pub fun intern_pointer(ti: *TypeInterner, base: TypeId) res[TypeId, fail.Fail];
```

## fun intern_secret

```mach
pub fun intern_secret(ti: *TypeInterner, base: TypeId) res[TypeId, fail.Fail];
```

## fun intern_array

```mach
pub fun intern_array(ti: *TypeInterner, base: TypeId, count: u32) res[TypeId, fail.Fail];
```

## fun intern_vector

```mach
pub fun intern_vector(ti: *TypeInterner, element: TypeKind, lanes: u32) res[TypeId, fail.Fail];
```

## fun intern_handle

```mach
pub fun intern_handle(ti: *TypeInterner, name: intern.StrId, ctor: u32, ops: *u32, count: u32) res[TypeId, fail.Fail];
```

## fun is_handle

```mach
pub fun is_handle(ti: *TypeInterner, tid: TypeId) bool;
```

## fun intern_abi_type

```mach
pub fun intern_abi_type(ti: *TypeInterner, name: intern.StrId, tag: u32, size: u32,
align: u32) res[TypeId, fail.Fail];
```

## fun is_abi_type

```mach
pub fun is_abi_type(ti: *TypeInterner, tid: TypeId) bool;
```

## fun handle_operand

```mach
pub fun handle_operand(ti: *TypeInterner, tid: TypeId, index: u32) u32;
```

## fun intern_pack

```mach
pub fun intern_pack(ti: *TypeInterner) res[TypeId, fail.Fail];
```

## fun intern_case_selector

```mach
pub fun intern_case_selector(ti: *TypeInterner, tag_type: TypeId, case_name: intern.StrId, case_index: u32) res[TypeId, fail.Fail];
```

## fun is_case_selector

```mach
pub fun is_case_selector(ti: *TypeInterner, tid: TypeId) bool;
```

## fun case_selector_tag_type

```mach
pub fun case_selector_tag_type(ti: *TypeInterner, tid: TypeId) TypeId;
```

## fun case_selector_case_index

```mach
pub fun case_selector_case_index(ti: *TypeInterner, tid: TypeId) u32;
```

## fun case_selector_case_name

```mach
pub fun case_selector_case_name(ti: *TypeInterner, tid: TypeId) intern.StrId;
```

## fun intern_nominal

```mach
pub fun intern_nominal(ti: *TypeInterner, name: intern.StrId, owner: TypeOwner, kind: TypeKind) res[TypeId, fail.Fail];
```

## fun intern_anonymous

```mach
pub fun intern_anonymous(ti: *TypeInterner, name: intern.StrId, owner: TypeOwner,
incarnation: u64, syntax: intern.StrId, kind: TypeKind) res[TypeId, fail.Fail];
```

an anonymous recipe belongs to one source incarnation

## fun intern_generic_param

```mach
pub fun intern_generic_param(ti: *TypeInterner, name: intern.StrId, owner: TypeOwner,
owner_name: intern.StrId, owner_kind: TypeKind, index: u32) res[TypeId, fail.Fail];
```

## fun intern_function

```mach
pub fun intern_function(ti: *TypeInterner, ret_type: TypeId, params: *TypeId, count: u32, c_variadic: bool) res[TypeId, fail.Fail];
```

## fun intern_instance

```mach
pub fun intern_instance(ti: *TypeInterner, nominal: TypeId, args: *TypeId, count: u32) res[TypeId, fail.Fail];
```

## fun type_equals_signatures

```mach
pub fun type_equals_signatures(ti: *TypeInterner, a: TypeId, b: TypeId) bool;
```

## fun field_projection_reset

```mach
pub fun field_projection_reset(ti: *TypeInterner);
```

## fun field_epoch_bump

```mach
pub fun field_epoch_bump(ti: *TypeInterner);
```

## fun field_epoch

```mach
pub fun field_epoch(ti: *TypeInterner) u32;
```

## fun field_table_stage

```mach
pub fun field_table_stage(ti: *TypeInterner, additional: u32) res[u32, fail.Fail];
```

## fun field_table_stage_push

```mach
pub fun field_table_stage_push(ti: *TypeInterner, name: intern.StrId, ty: TypeId) err[fail.Fail];
```

## fun field_table_stage_push_entry

```mach
pub fun field_table_stage_push_entry(ti: *TypeInterner, fe: FieldEntry) err[fail.Fail];
```

## fun field_table_publish

```mach
pub fun field_table_publish(ti: *TypeInterner, ty: TypeId, fields_start: u32, fields_len: u32) err[fail.Fail];
```

## fun aggregate_nominal

```mach
pub fun aggregate_nominal(ti: *TypeInterner, tid: TypeId) TypeId;
```

## fun is_tag

```mach
pub fun is_tag(ti: *TypeInterner, tid: TypeId) bool;
```

## fun set_tag_discriminator

```mach
pub fun set_tag_discriminator(ti: *TypeInterner, tid: TypeId, disc: TypeId) err[fail.Fail];
```

## fun tag_discriminator

```mach
pub fun tag_discriminator(ti: *TypeInterner, tid: TypeId) TypeId;
```

## fun tag_discriminator_bytes

```mach
pub fun tag_discriminator_bytes(ti: *TypeInterner, tid: TypeId) u32;
```

the declared discriminator width in bytes, or zero when the tag has no usable declaration

## fun is_valid_discriminator_kind

```mach
pub fun is_valid_discriminator_kind(k: TypeKind) bool;
```

## fun is_union

```mach
pub fun is_union(ti: *TypeInterner, tid: TypeId) bool;
```

## fun is_record

```mach
pub fun is_record(ti: *TypeInterner, tid: TypeId) bool;
```

## fun field_table_for

```mach
pub fun field_table_for(ti: *TypeInterner, ty: TypeId) opt[*FieldTable];
```

## fun field_count

```mach
pub fun field_count(ti: *TypeInterner, ty: TypeId) u32;
```

## fun field_at

```mach
pub fun field_at(ti: *TypeInterner, ty: TypeId, ix: u32) opt[*FieldEntry];
```

## fun field_entry_at

```mach
pub fun field_entry_at(ti: *TypeInterner, ft: *FieldTable, ix: u32) *FieldEntry;
```

