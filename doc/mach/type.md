[mach](README.md) / **type**

# mach.type

the type table: every type interned once, with its facts and its field tables

a `TypeInterner` interns each type once and names it by a `TypeId`. each type carries the facts
its parts spell, read when it is interned. the field tables, tag discriminators and layout
decorations belong to the current projection (`mach.type.field`), and a fact read from them is
kept until the projection resets or a table is published over one that held other fields.

Source: [`src/type.mach`](../../src/type.mach)

Uses types from [`mach.base.fail`](base/fail.md), [`mach.base.handle`](base/handle.md), [`mach.base.intern`](base/intern.md), [`mach.base.source`](base/source.md), [`mach.session.module`](session/module.md), [`mach.type.field`](type/field.md), [`mach.type.scan`](type/scan.md).

| Type | | |
| :--- | :--- | :--- |
| [`TypeSecret`](#typesecret) | `rec` |  |
| [`TypeHandle`](#typehandle) | `rec` |  |
| [`TypeArray`](#typearray) | `rec` |  |
| [`TypeFun`](#typefun) | `rec` |  |
| [`TypeOwner`](#typeowner) | `rec` | persistent source identity, independent of the current module registry |
| [`TypeNominal`](#typenominal) | `rec` |  |
| [`GenericOwner`](#genericowner) | `rec` | the declaration whose type parameters a substitution replaces: a parameter of any other declaration, an enclosing generic's for one, is left as it is |
| [`Type`](#type) | `rec` |  |
| [`TypeInterner`](#typeinterner) | `rec` |  |
| [`Projected`](#projected) | `def` | a fact of a type that reads the field tables, so it holds for one projection |

| On `GenericOwner` | |
| :--- | :--- |
| [`generic_owner_equals`](#generic_owner_equals), [`param_owner`](#param_owner), [`param_owned_by`](#param_owned_by) | |

| On `TypeInterner` | |
| :--- | :--- |
| [`facts_of`](#facts_of) | the facts of `tid`, none for the nil and error types |
| [`init`](#init) | |
| [`dnit`](#dnit) | release a type store; a nil store holds nothing |
| [`get`](#get), [`type_count`](#type_count), [`get_param`](#get_param), [`prim`](#prim), [`is_integer`](#is_integer), [`is_numeric`](#is_numeric), [`is_float`](#is_float), [`is_pointer_like`](#is_pointer_like), [`is_pointer`](#is_pointer), [`deref_one`](#deref_one), [`is_secret`](#is_secret), [`is_vector`](#is_vector), [`vector_element`](#vector_element), [`vector_lanes`](#vector_lanes), [`element_count`](#element_count), [`vector_mask`](#vector_mask), [`strip_secret`](#strip_secret) | |
| [`mentions_generic_param`](#mentions_generic_param) | whether `tid` mentions a type parameter anywhere it is spelled; it never fails |
| [`shape_kind`](#shape_kind), [`is_u8`](#is_u8), [`is_u1`](#is_u1) | |
| [`carries_secret`](#carries_secret) | whether `tid` is secret or reaches a secret through pointers and array elements alone |
| [`contains_secret`](#contains_secret) | true when any byte of a value of this type may be secret: the type itself, an element, a field or a case payload; a pointer's pointee is not the object's storage and does not count |
| [`type_scan_begin`](#type_scan_begin), [`type_scan_end`](#type_scan_end), [`type_scan_mark`](#type_scan_mark), [`pair_scan_begin`](#pair_scan_begin), [`pair_scan_end`](#pair_scan_end), [`pair_scan_mark`](#pair_scan_mark), [`intern_pointer`](#intern_pointer), [`intern_secret`](#intern_secret), [`intern_array`](#intern_array), [`intern_vector`](#intern_vector) | |
| [`intern_window`](#intern_window) | the type of the window `object[start, count]`, its one owner: `count` lanes of a vector as a vector of that width, or `count` elements of an array or through a pointer as `[count]T`; a secret object gives a secret window, as a whole read of it is secret |
| [`intern_handle`](#intern_handle), [`is_handle`](#is_handle) | |
| [`carries_handle`](#carries_handle) | a handle, or a pointer, array or secret over one: every shape that places a handle where the program would own its storage |
| [`handle_in_array`](#handle_in_array) | an array whose elements carry a handle, reached directly or through a pointer or secret |
| [`handle_pointer_pointer`](#handle_pointer_pointer) | a pointer to a pointer to a handle, through any secret: the innermost double indirection of every deeper one, so a chain of pointers over a handle matches at exactly one level |
| [`has_handle_pointer_pointer`](#has_handle_pointer_pointer) | a pointer to a pointer to a handle anywhere under the pointers, arrays and secrets of a type |
| [`intern_abi_type`](#intern_abi_type), [`is_abi_type`](#is_abi_type), [`handle_operand`](#handle_operand), [`intern_pack`](#intern_pack), [`intern_case_selector`](#intern_case_selector), [`is_case_selector`](#is_case_selector), [`is_pack`](#is_pack), [`case_selector_tag_type`](#case_selector_tag_type), [`intern_nominal`](#intern_nominal) | |
| [`intern_anonymous`](#intern_anonymous) | an anonymous recipe belongs to one source incarnation |
| [`intern_generic_param`](#intern_generic_param), [`intern_function`](#intern_function), [`intern_instance`](#intern_instance), [`type_equals_signatures`](#type_equals_signatures), [`field_projection_reset`](#field_projection_reset) | |
| [`projected_get`](#projected_get) | the settled answer to projected fact `which` of `tid`, none while it is unsettled |
| [`projected_set`](#projected_set) | |
| [`identity_get`](#identity_get) | the identity recipe gave `tid`, none until it is asked for |
| [`identity_set`](#identity_set), [`field_table_stage`](#field_table_stage), [`field_table_stage_push`](#field_table_stage_push), [`field_table_stage_push_entry`](#field_table_stage_push_entry) | |
| [`field_table_publish`](#field_table_publish) | a table published over one holding other fields drops every projected fact, since any of them may have read the old one |
| [`decoration_of`](#decoration_of) | the layout decorations nominal `tid` declares under the current projection |
| [`decoration_set`](#decoration_set), [`aggregate_nominal`](#aggregate_nominal), [`is_tag`](#is_tag), [`set_tag_discriminator`](#set_tag_discriminator), [`tag_discriminator`](#tag_discriminator) | |
| [`tag_discriminator_bytes`](#tag_discriminator_bytes) | the declared discriminator width in bytes, or zero when the tag has no usable declaration |
| [`is_union`](#is_union), [`is_record`](#is_record), [`field_table_for`](#field_table_for), [`field_count`](#field_count), [`field_at`](#field_at), [`field_entry_at`](#field_entry_at) | |

| On `Projected` | |
| :--- | :--- |
| [`PROJECTED_HELD_SECRET`](#projected) | a value of the type holds a secret in its own storage (`contains_secret`) |
| [`PROJECTED_REACHES_SECRET`](#projected) | the type reaches a secret anywhere, through pointers, with an instance's arguments counted where its generic places them (`query.deep.secret`) |
| [`PROJECTED_SPELLS_SECRET`](#projected) | a secret is spelled anywhere the type reaches, through every edge and every instance argument (`query.reach.module_secret`) |

| Functions | |
| :--- | :--- |
| [`abi_type_c_name`](#abi_type_c_name) | the C type an ABI type tag stands for, absent for a tag outside the catalog |
| [`is_valid_discriminator_kind`](#is_valid_discriminator_kind) | |

| Constants | |
| :--- | :--- |
| [`NO_HANDLE_CTOR`](#constants) | the constructor of a handle no target mints in this build: its operands are its declaration's site, so its identity is the declaration's |
| [`HANDLE_PART_WORD`](#constants) | a minted handle's constructor is the interned name of the kind the target's family mints, as `spirv.image`, and its operands are pairs, the part then its value: a word is the number itself, a type operand the type's id, and a named one the interned name of an enumerant its row's grammar states |
| [`HANDLE_PART_TYPE`](#constants), [`HANDLE_PART_NAME`](#constants), [`ABI_TYPE_VA_LIST`](#constants) | |
| [`FACT_PARAM`](#constants) | a generic parameter is spelled in the type: itself, or under a pointer, secret or array, in a signature or as an instance's argument |
| [`FACT_CARRIES_SECRET`](#constants) | the type is secret, or reaches a secret through pointers and array elements alone |
| [`FACT_CARRIES_HANDLE`](#constants) | the type is a handle, or reaches one through pointers, arrays and secrets alone |

---

## TypeSecret

```mach
pub rec TypeSecret {
    base: TypeId;
}
```

## TypeHandle

```mach
pub rec TypeHandle {
    name:      mach.base.intern.StrId;
    ctor:      u32;
    ops_start: u32;
    ops_len:   u32;
}
```

## TypeArray

```mach
pub rec TypeArray {
    base:  TypeId;
    count: u32;
}
```

## TypeFun

```mach
pub rec TypeFun {
    ret_type:       TypeId;
    params_start:   u32;
    params_len:     u32;
    c_variadic:     bool;
    next_in_bucket: TypeId;
}
```

## TypeOwner

```mach
pub rec TypeOwner {
    module: mach.session.module.StableModuleId;
    file:   mach.base.source.FileId;
}
```

persistent source identity, independent of the current module registry

## TypeNominal

```mach
pub rec TypeNominal {
    name:        mach.base.intern.StrId;
    owner:       TypeOwner;
    incarnation: u64;
    syntax:      mach.base.intern.StrId;
}
```

## GenericOwner

```mach
pub rec GenericOwner {
    owner: TypeOwner;
    name:  mach.base.intern.StrId;
    kind:  TypeKind;
}
```

the declaration whose type parameters a substitution replaces: a parameter
of any other declaration, an enclosing generic's for one, is left as it is

### generic_owner_equals

```mach
pub fun generic_owner_equals(a: *GenericOwner, b: *GenericOwner) bool
```

### param_owner

```mach
pub fun param_owner(p: *TypeGenericParam) GenericOwner
```

### param_owned_by

```mach
pub fun param_owned_by(p: *TypeGenericParam, who: *GenericOwner) bool
```

## Type

```mach
pub rec Type {
    kind: TypeKind;
    # what the type spells, read from its parts when it is interned (`FACT_*`); never
    # part of its identity
    facts: u8;
    data:  uni {
        pointer: TypePointer;
        secret: TypeSecret;
        array: TypeArray;
        vector: TypeVector;
        handle: TypeHandle;
        abi: TypeAbi;
        fn: TypeFun;
        nominal: TypeNominal;
        generic_param: TypeGenericParam;
        instance: TypeInstance;
        case_selector: TypeCaseSelector;
    };
}
```

## TypeInterner

```mach
pub rec TypeInterner {
    a: *std.allocator.Allocator;

    types: mach.base.handle.StableChunks[Type];

    params:     *TypeId;
    params_len: u32;
    params_cap: u32;

    dedup: std.collections.map.MapBy[Type, TypeId];

    fun_index:      std.collections.map.Map[u64, TypeId];
    instance_index: std.collections.map.Map[u64, TypeId];

    handles:     *TypeId;
    handles_len: u32;
    handles_cap: u32;

    handle_ops:     *u32;
    handle_ops_len: u32;
    handle_ops_cap: u32;

    prims: [type_prim.PRIM_COUNT]TypeId;

    vecs: [type_prim.VEC_COUNT]TypeId;

    fields: mach.type.field.Projection;

    # bumped by every projection reset, which drops every fact derived from the tables;
    # published tables never change otherwise
    projection_generation: u64;

    # by type index, the projected facts settled under the current projection (`PROJECTED_*`)
    projected: Vector[u8];

    # by type index, the identity recipe gave the type, zero until it is asked for. it
    # names the type by its recipe alone, so no projection drops it
    identities: Vector[u64];

    scans: mach.type.scan.Sets;
}
```

### facts_of

```mach
pub fun facts_of(ti: *TypeInterner, tid: TypeId) u8
```

the facts of `tid`, none for the nil and error types

### init

```mach
pub fun init(ti: *TypeInterner, a: *std.allocator.Allocator) err[mach.base.fail.Fail]
```

### dnit

```mach
pub fun dnit(ti: *TypeInterner)
```

release a type store; a nil store holds nothing

### get

```mach
pub fun get(ti: *TypeInterner, tid: TypeId) opt[*Type]
```

### type_count

```mach
pub fun type_count(ti: *TypeInterner) u32
```

### get_param

```mach
pub fun get_param(ti: *TypeInterner, index: u32) opt[TypeId]
```

### prim

```mach
pub fun prim(ti: *TypeInterner, kind: TypeKind) opt[TypeId]
```

### is_integer

```mach
pub fun is_integer(ti: *TypeInterner, tid: TypeId) bool
```

### is_numeric

```mach
pub fun is_numeric(ti: *TypeInterner, tid: TypeId) bool
```

### is_float

```mach
pub fun is_float(ti: *TypeInterner, tid: TypeId) bool
```

### is_pointer_like

```mach
pub fun is_pointer_like(ti: *TypeInterner, tid: TypeId) bool
```

### is_pointer

```mach
pub fun is_pointer(ti: *TypeInterner, tid: TypeId) bool
```

### deref_one

```mach
pub fun deref_one(ti: *TypeInterner, tid: TypeId) TypeId
```

### is_secret

```mach
pub fun is_secret(ti: *TypeInterner, tid: TypeId) bool
```

### is_vector

```mach
pub fun is_vector(ti: *TypeInterner, tid: TypeId) bool
```

### vector_element

```mach
pub fun vector_element(ti: *TypeInterner, tid: TypeId) TypeKind
```

### vector_lanes

```mach
pub fun vector_lanes(ti: *TypeInterner, tid: TypeId) u32
```

### element_count

```mach
pub fun element_count(ti: *TypeInterner, tid: TypeId) opt[u32]
```

### vector_mask

```mach
pub fun vector_mask(ti: *TypeInterner, tid: TypeId) res[TypeId, mach.base.fail.Fail]
```

### strip_secret

```mach
pub fun strip_secret(ti: *TypeInterner, tid: TypeId) TypeId
```

### mentions_generic_param

```mach
pub fun mentions_generic_param(ti: *TypeInterner, tid: TypeId) res[bool, mach.base.fail.Fail]
```

whether `tid` mentions a type parameter anywhere it is spelled; it never fails

### shape_kind

```mach
pub fun shape_kind(ti: *TypeInterner, tid: TypeId) TypeKind
```

### is_u8

```mach
pub fun is_u8(ti: *TypeInterner, tid: TypeId) bool
```

### is_u1

```mach
pub fun is_u1(ti: *TypeInterner, tid: TypeId) bool
```

### carries_secret

```mach
pub fun carries_secret(ti: *TypeInterner, tid: TypeId) bool
```

whether `tid` is secret or reaches a secret through pointers and array elements alone

### contains_secret

```mach
pub fun contains_secret(ti: *TypeInterner, tid: TypeId) res[bool, mach.base.fail.Fail]
```

true when any byte of a value of this type may be secret: the type itself, an element, a field or a case
payload; a pointer's pointee is not the object's storage and does not count. the walk keeps the
fields it has yet to visit on a worklist, so a type nested deeper than any stack is still
answered; a worklist that cannot grow is the failure. an aggregate with no table yet adds
nothing, so a clear answer that met one is not recorded

### type_scan_begin

```mach
pub fun type_scan_begin(ti: *TypeInterner, scan: *TypeScan) err[mach.base.fail.Fail]
```

### type_scan_end

```mach
pub fun type_scan_end(ti: *TypeInterner, scan: *TypeScan)
```

### type_scan_mark

```mach
pub fun type_scan_mark(ti: *TypeInterner, scan: *TypeScan, tid: TypeId) res[VisitMark, mach.base.fail.Fail]
```

### pair_scan_begin

```mach
pub fun pair_scan_begin(ti: *TypeInterner, scan: *PairScan) err[mach.base.fail.Fail]
```

### pair_scan_end

```mach
pub fun pair_scan_end(ti: *TypeInterner, scan: *PairScan)
```

### pair_scan_mark

```mach
pub fun pair_scan_mark(ti: *TypeInterner, scan: *PairScan, a: TypeId, b: TypeId) res[VisitMark, mach.base.fail.Fail]
```

### intern_pointer

```mach
pub fun intern_pointer(ti: *TypeInterner, base: TypeId) res[TypeId, mach.base.fail.Fail]
```

### intern_secret

```mach
pub fun intern_secret(ti: *TypeInterner, base: TypeId) res[TypeId, mach.base.fail.Fail]
```

### intern_array

```mach
pub fun intern_array(ti: *TypeInterner, base: TypeId, count: u32) res[TypeId, mach.base.fail.Fail]
```

### intern_vector

```mach
pub fun intern_vector(ti: *TypeInterner, element: TypeKind, lanes: u32) res[TypeId, mach.base.fail.Fail]
```

### intern_window

```mach
pub fun intern_window(ti: *TypeInterner, object: TypeId, count: u32) res[TypeId, mach.base.fail.Fail]
```

the type of the window `object[start, count]`, its one owner: `count` lanes of a vector as a
vector of that width, or `count` elements of an array or through a pointer as `[count]T`; a
secret object gives a secret window, as a whole read of it is secret

### intern_handle

```mach
pub fun intern_handle(ti: *TypeInterner, name: mach.base.intern.StrId, ctor: u32, ops: *u32, count: u32) res[TypeId, mach.base.fail.Fail]
```

### is_handle

```mach
pub fun is_handle(ti: *TypeInterner, tid: TypeId) bool
```

### carries_handle

```mach
pub fun carries_handle(ti: *TypeInterner, tid: TypeId) bool
```

a handle, or a pointer, array or secret over one: every shape that places a handle
where the program would own its storage

### handle_in_array

```mach
pub fun handle_in_array(ti: *TypeInterner, tid: TypeId) bool
```

an array whose elements carry a handle, reached directly or through a pointer or secret

### handle_pointer_pointer

```mach
pub fun handle_pointer_pointer(ti: *TypeInterner, tid: TypeId) bool
```

a pointer to a pointer to a handle, through any secret: the innermost double indirection
of every deeper one, so a chain of pointers over a handle matches at exactly one level

### has_handle_pointer_pointer

```mach
pub fun has_handle_pointer_pointer(ti: *TypeInterner, tid: TypeId) bool
```

a pointer to a pointer to a handle anywhere under the pointers, arrays and secrets of a type

### intern_abi_type

```mach
pub fun intern_abi_type(ti: *TypeInterner, name: mach.base.intern.StrId, tag: u32, size: u32,
    align: u32) res[TypeId, mach.base.fail.Fail]
```

### is_abi_type

```mach
pub fun is_abi_type(ti: *TypeInterner, tid: TypeId) bool
```

### handle_operand

```mach
pub fun handle_operand(ti: *TypeInterner, tid: TypeId, index: u32) u32
```

### intern_pack

```mach
pub fun intern_pack(ti: *TypeInterner) res[TypeId, mach.base.fail.Fail]
```

### intern_case_selector

```mach
pub fun intern_case_selector(ti: *TypeInterner, tag_type: TypeId, case_name: mach.base.intern.StrId, case_index: u32) res[TypeId, mach.base.fail.Fail]
```

### is_case_selector

```mach
pub fun is_case_selector(ti: *TypeInterner, tid: TypeId) bool
```

### is_pack

```mach
pub fun is_pack(ti: *TypeInterner, tid: TypeId) bool
```

### case_selector_tag_type

```mach
pub fun case_selector_tag_type(ti: *TypeInterner, tid: TypeId) TypeId
```

### intern_nominal

```mach
pub fun intern_nominal(ti: *TypeInterner, name: mach.base.intern.StrId, owner: TypeOwner, kind: TypeKind) res[TypeId, mach.base.fail.Fail]
```

### intern_anonymous

```mach
pub fun intern_anonymous(ti: *TypeInterner, name: mach.base.intern.StrId, owner: TypeOwner,
    incarnation: u64, syntax: mach.base.intern.StrId, kind: TypeKind) res[TypeId, mach.base.fail.Fail]
```

an anonymous recipe belongs to one source incarnation

### intern_generic_param

```mach
pub fun intern_generic_param(ti: *TypeInterner, name: mach.base.intern.StrId, owner: TypeOwner,
    owner_name: mach.base.intern.StrId, owner_kind: TypeKind, index: u32) res[TypeId, mach.base.fail.Fail]
```

### intern_function

```mach
pub fun intern_function(ti: *TypeInterner, ret_type: TypeId, params: *TypeId, count: u32, c_variadic: bool) res[TypeId, mach.base.fail.Fail]
```

### intern_instance

```mach
pub fun intern_instance(ti: *TypeInterner, nominal: TypeId, args: *TypeId, count: u32) res[TypeId, mach.base.fail.Fail]
```

### type_equals_signatures

```mach
pub fun type_equals_signatures(ti: *TypeInterner, a: TypeId, b: TypeId) bool
```

### field_projection_reset

```mach
pub fun field_projection_reset(ti: *TypeInterner)
```

### projected_get

```mach
pub fun projected_get(ti: *TypeInterner, tid: TypeId, which: Projected) opt[bool]
```

the settled answer to projected fact `which` of `tid`, none while it is unsettled

### projected_set

```mach
pub fun projected_set(ti: *TypeInterner, tid: TypeId, which: Projected, answer: bool) err[mach.base.fail.Fail]
```

### identity_get

```mach
pub fun identity_get(ti: *TypeInterner, tid: TypeId) opt[u64]
```

the identity recipe gave `tid`, none until it is asked for

### identity_set

```mach
pub fun identity_set(ti: *TypeInterner, tid: TypeId, identity: u64) err[mach.base.fail.Fail]
```

### field_table_stage

```mach
pub fun field_table_stage(ti: *TypeInterner, additional: u32) res[u32, mach.base.fail.Fail]
```

### field_table_stage_push

```mach
pub fun field_table_stage_push(ti: *TypeInterner, name: mach.base.intern.StrId, ty: TypeId) err[mach.base.fail.Fail]
```

### field_table_stage_push_entry

```mach
pub fun field_table_stage_push_entry(ti: *TypeInterner, fe: FieldEntry) err[mach.base.fail.Fail]
```

### field_table_publish

```mach
pub fun field_table_publish(ti: *TypeInterner, ty: TypeId, fields_start: u32, fields_len: u32) err[mach.base.fail.Fail]
```

a table published over one holding other fields drops every projected fact, since any of
them may have read the old one

### decoration_of

```mach
pub fun decoration_of(ti: *TypeInterner, tid: TypeId) Decoration
```

the layout decorations nominal `tid` declares under the current projection

### decoration_set

```mach
pub fun decoration_set(ti: *TypeInterner, tid: TypeId, d: Decoration) err[mach.base.fail.Fail]
```

### aggregate_nominal

```mach
pub fun aggregate_nominal(ti: *TypeInterner, tid: TypeId) TypeId
```

### is_tag

```mach
pub fun is_tag(ti: *TypeInterner, tid: TypeId) bool
```

### set_tag_discriminator

```mach
pub fun set_tag_discriminator(ti: *TypeInterner, tid: TypeId, disc: TypeId) err[mach.base.fail.Fail]
```

### tag_discriminator

```mach
pub fun tag_discriminator(ti: *TypeInterner, tid: TypeId) TypeId
```

### tag_discriminator_bytes

```mach
pub fun tag_discriminator_bytes(ti: *TypeInterner, tid: TypeId) u32
```

the declared discriminator width in bytes, or zero when the tag has no usable declaration

### is_union

```mach
pub fun is_union(ti: *TypeInterner, tid: TypeId) bool
```

### is_record

```mach
pub fun is_record(ti: *TypeInterner, tid: TypeId) bool
```

### field_table_for

```mach
pub fun field_table_for(ti: *TypeInterner, ty: TypeId) opt[*FieldTable]
```

### field_count

```mach
pub fun field_count(ti: *TypeInterner, ty: TypeId) u32
```

### field_at

```mach
pub fun field_at(ti: *TypeInterner, ty: TypeId, ix: u32) opt[*FieldEntry]
```

### field_entry_at

```mach
pub fun field_entry_at(ti: *TypeInterner, ft: *FieldTable, ix: u32) *FieldEntry
```

## Projected

```mach
pub def Projected: u8
```

a fact of a type that reads the field tables, so it holds for one projection. it is recorded
only once every table it read was published. a reset of the projection drops them all, and
so does a table published over one that held other fields. each takes two bits: settled, then its answer

| Name | Value | |
| :--- | :--- | :--- |
| `PROJECTED_HELD_SECRET` | `0` | a value of the type holds a secret in its own storage (`contains_secret`) |
| `PROJECTED_REACHES_SECRET` | `1` | the type reaches a secret anywhere, through pointers, with an instance's arguments counted where its generic places them (`query.deep.secret`) |
| `PROJECTED_SPELLS_SECRET` | `2` | a secret is spelled anywhere the type reaches, through every edge and every instance argument (`query.reach.module_secret`) |

## Functions

### abi_type_c_name

```mach
pub fun abi_type_c_name(tag: u32) opt[str]
```

the C type an ABI type tag stands for, absent for a tag outside the catalog

### is_valid_discriminator_kind

```mach
pub fun is_valid_discriminator_kind(k: TypeKind) bool
```

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `NO_HANDLE_CTOR` | `u32` | `0xFFFFFFFF` | the constructor of a handle no target mints in this build: its operands are its declaration's site, so its identity is the declaration's |
| `HANDLE_PART_WORD` | `u32` | `0` | a minted handle's constructor is the interned name of the kind the target's family mints, as `spirv.image`, and its operands are pairs, the part then its value: a word is the number itself, a type operand the type's id, and a named one the interned name of an enumerant its row's grammar states |
| `HANDLE_PART_TYPE` | `u32` | `1` |  |
| `HANDLE_PART_NAME` | `u32` | `2` |  |
| `ABI_TYPE_VA_LIST` | `u32` | `0` |  |
| `FACT_PARAM` | `u8` | `1` | a generic parameter is spelled in the type: itself, or under a pointer, secret or array, in a signature or as an instance's argument |
| `FACT_CARRIES_SECRET` | `u8` | `2` | the type is secret, or reaches a secret through pointers and array elements alone |
| `FACT_CARRIES_HANDLE` | `u8` | `4` | the type is a handle, or reaches one through pointers, arrays and secrets alone |

