# mach.lang.type

## rec TypeSecret

```mach
pub rec TypeSecret;
```

## rec TypeHandle

```mach
pub rec TypeHandle;
```

## val NO_HANDLE_CTOR

```mach
pub val NO_HANDLE_CTOR: u32 = 0xFFFFFFFF
```

the constructor of a handle no target mints in this build: its operands are its
declaration's site, so its identity is the declaration's

## val HANDLE_PART_WORD

```mach
pub val HANDLE_PART_WORD: u32 = 0
```

a minted handle's constructor is the interned name of the kind the target's family
mints, as `spirv.image`, and its operands are pairs, the part then its value: a word
is the number itself, a type operand the type's id, and a named one the interned name
of an enumerant its row's grammar states

## val HANDLE_PART_TYPE

```mach
pub val HANDLE_PART_TYPE: u32 = 1
```

## val HANDLE_PART_NAME

```mach
pub val HANDLE_PART_NAME: u32 = 2
```

## val ABI_TYPE_VA_LIST

```mach
pub val ABI_TYPE_VA_LIST: u32 = 0
```

## fun abi_type_c_name

```mach
pub fun abi_type_c_name(tag: u32) opt[str];
```

the C type an ABI type tag stands for, absent for a tag outside the catalog

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

## rec GenericOwner

```mach
pub rec GenericOwner;
```

the declaration whose type parameters a substitution replaces: a parameter
of any other declaration, an enclosing generic's for one, is left as it is

## fun generic_owner_equals

```mach
pub fun generic_owner_equals(a: *GenericOwner, b: *GenericOwner) bool;
```

## fun param_owner

```mach
pub fun param_owner(p: *TypeGenericParam) GenericOwner;
```

## fun param_owned_by

```mach
pub fun param_owned_by(p: *TypeGenericParam, who: *GenericOwner) bool;
```

## rec Type

```mach
pub rec Type;
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

release a type store; a nil store holds nothing

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

## fun vector_mask

```mach
pub fun vector_mask(ti: *TypeInterner, tid: TypeId) res[TypeId, fail.Fail];
```

## fun strip_secret

```mach
pub fun strip_secret(ti: *TypeInterner, tid: TypeId) TypeId;
```

## fun mentions_generic_param

```mach
pub fun mentions_generic_param(ti: *TypeInterner, tid: TypeId) res[bool, fail.Fail];
```

whether `tid` mentions a type parameter anywhere it is spelled. the walk keeps the types it
has yet to visit on a worklist rather than the stack, so a type nested through flat defs
deeper than any stack is still answered; a worklist that cannot grow is the failure

## fun shape_kind

```mach
pub fun shape_kind(ti: *TypeInterner, tid: TypeId) TypeKind;
```

## fun is_u8

```mach
pub fun is_u8(ti: *TypeInterner, tid: TypeId) bool;
```

## fun is_u1

```mach
pub fun is_u1(ti: *TypeInterner, tid: TypeId) bool;
```

## fun carries_secret

```mach
pub fun carries_secret(ti: *TypeInterner, tid: TypeId) bool;
```

whether `tid` is secret or reaches a secret through pointers and array elements alone

## fun contains_secret

```mach
pub fun contains_secret(ti: *TypeInterner, tid: TypeId) res[bool, fail.Fail];
```

true when any byte of a value of this type may be secret: the type itself, an element, a field or a case
payload; a pointer's pointee is not the object's storage and does not count. the walk keeps the
fields it has yet to visit on a worklist, so a type nested deeper than any stack is still
answered; a worklist that cannot grow is the failure

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

## fun intern_window

```mach
pub fun intern_window(ti: *TypeInterner, object: TypeId, count: u32) res[TypeId, fail.Fail];
```

the type of the window `object[start, count]`, its one owner: `count` lanes of a vector as a
vector of that width, or `count` elements of an array or through a pointer as `[count]T`; a
secret object gives a secret window, as a whole read of it is secret

## fun intern_handle

```mach
pub fun intern_handle(ti: *TypeInterner, name: intern.StrId, ctor: u32, ops: *u32, count: u32) res[TypeId, fail.Fail];
```

## fun is_handle

```mach
pub fun is_handle(ti: *TypeInterner, tid: TypeId) bool;
```

## fun carries_handle

```mach
pub fun carries_handle(ti: *TypeInterner, tid: TypeId) bool;
```

a handle, or a pointer, array or secret over one: every shape that places a handle
where the program would own its storage

## fun handle_in_array

```mach
pub fun handle_in_array(ti: *TypeInterner, tid: TypeId) bool;
```

an array whose elements carry a handle, reached directly or through a pointer or secret

## fun handle_pointer_pointer

```mach
pub fun handle_pointer_pointer(ti: *TypeInterner, tid: TypeId) bool;
```

a pointer to a pointer to a handle, through any secret: the innermost double indirection
of every deeper one, so a chain of pointers over a handle matches at exactly one level

## fun has_handle_pointer_pointer

```mach
pub fun has_handle_pointer_pointer(ti: *TypeInterner, tid: TypeId) bool;
```

a pointer to a pointer to a handle anywhere under the pointers, arrays and secrets of a type

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

## fun is_pack

```mach
pub fun is_pack(ti: *TypeInterner, tid: TypeId) bool;
```

## fun case_selector_tag_type

```mach
pub fun case_selector_tag_type(ti: *TypeInterner, tid: TypeId) TypeId;
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

## fun decoration_of

```mach
pub fun decoration_of(ti: *TypeInterner, tid: TypeId) Decoration;
```

the layout decorations nominal `tid` declares under the current projection

## fun decoration_set

```mach
pub fun decoration_set(ti: *TypeInterner, tid: TypeId, d: Decoration) err[fail.Fail];
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

