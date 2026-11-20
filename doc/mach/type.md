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
| [`TypeSecret`](#typesecret) | `rec` | the parts of a secret type |
| [`TypeHandle`](#typehandle) | `rec` | the parts of a target handle type |
| [`TypeArray`](#typearray) | `rec` | the parts of an array type |
| [`TypeFun`](#typefun) | `rec` | the parts of a function type |
| [`TypeOwner`](#typeowner) | `rec` | persistent source identity, independent of the current module registry |
| [`TypeNominal`](#typenominal) | `rec` | the identity of a record, union or tag |
| [`GenericOwner`](#genericowner) | `rec` | the declaration whose type parameters a substitution replaces: a parameter of any other declaration, an enclosing generic's for one, is left as it is |
| [`Type`](#type) | `rec` | one interned type |
| [`TypeInterner`](#typeinterner) | `rec` | the type table: every type interned once and named by its `TypeId` |
| [`Projected`](#projected) | `def` | a fact of a type that reads the field tables, so it holds for one projection |

| On `GenericOwner` | |
| :--- | :--- |
| [`generic_owner_equals`](#generic_owner_equals) | whether two generic owners name the same declaration |
| [`param_owner`](#param_owner) | the declaration that owns a generic parameter |
| [`param_owned_by`](#param_owned_by) | whether a generic parameter belongs to declaration `who` |

| On `TypeInterner` | |
| :--- | :--- |
| [`facts_of`](#facts_of) | the facts of `tid`, none for the nil and error types |
| [`init`](#init) | set up an empty type store with every primitive and seeded vector interned |
| [`dnit`](#dnit) | release a type store; a nil store holds nothing |
| [`get`](#get) | the type `tid` names |
| [`type_count`](#type_count) | how many types the store holds |
| [`get_param`](#get_param) | one entry of the pool of function parameter types and instance arguments |
| [`prim`](#prim) | the type of a primitive kind |
| [`is_integer`](#is_integer) | whether `tid`, with any secret qualifier stripped, is an integer primitive |
| [`is_numeric`](#is_numeric) | whether `tid`, with any secret qualifier stripped, is an integer or float primitive |
| [`is_float`](#is_float) | whether `tid`, with any secret qualifier stripped, is a float primitive |
| [`is_pointer_like`](#is_pointer_like) | whether `tid`, with any secret qualifier stripped, is held as an address: `ptr`, a typed pointer or a function |
| [`is_pointer`](#is_pointer) | whether `tid`, with any secret qualifier stripped, is a typed pointer |
| [`deref_one`](#deref_one) | the type a typed pointer points at, any secret qualifier on the pointer stripped |
| [`is_secret`](#is_secret) | whether `tid` is itself secret-qualified, not merely holding a secret |
| [`is_vector`](#is_vector) | whether `tid`, with any secret qualifier stripped, is a vector |
| [`vector_element`](#vector_element) | the lane kind of a vector, any secret qualifier stripped |
| [`vector_lanes`](#vector_lanes) | the lane count of a vector, any secret qualifier stripped |
| [`element_count`](#element_count) | the element count of an array or the lane count of a vector, any secret qualifier stripped |
| [`vector_mask`](#vector_mask) | the unsigned vector a lane comparison of vector `tid` gives: as many lanes, each as wide |
| [`strip_secret`](#strip_secret) | `tid` without its secret qualifier |
| [`mentions_generic_param`](#mentions_generic_param) | whether `tid` mentions a type parameter anywhere it is spelled; it never fails |
| [`shape_kind`](#shape_kind) | the kind that decides how `tid` is shaped: a secret's base's, and an instance's generic's |
| [`is_u8`](#is_u8) | whether `tid`, any secret qualifier stripped, is shaped as u8 |
| [`is_u1`](#is_u1) | whether `tid`, any secret qualifier stripped, is shaped as u1 |
| [`contains_secret`](#contains_secret) | true when any byte of a value of this type may be secret: the type itself, an element, a field or a case payload; a pointer's pointee is not the object's storage and does not count |
| [`type_scan_begin`](#type_scan_begin) | open a scan over the store's types at the next nesting level |
| [`type_scan_end`](#type_scan_end) | close a scan and every scan opened inside it |
| [`type_scan_mark`](#type_scan_mark) | mark a type seen in an open scan |
| [`pair_scan_begin`](#pair_scan_begin) | open a scan over pairs of the store's types at the next nesting level |
| [`pair_scan_end`](#pair_scan_end) | close a pair scan and every pair scan opened inside it |
| [`pair_scan_mark`](#pair_scan_mark) | mark an ordered pair of types seen in an open pair scan |
| [`intern_pointer`](#intern_pointer) | the pointer type to `base` |
| [`intern_secret`](#intern_secret) | the secret type over `base` |
| [`intern_array`](#intern_array) | the array type of `count` elements of `base` |
| [`intern_vector`](#intern_vector) | the vector type of `lanes` lanes of `element` |
| [`intern_window`](#intern_window) | the type of the window `object[start, count]`, its one owner: `count` lanes of a vector as a vector of that width, or `count` elements of an array or through a pointer as `[count]T`; a secret object gives a secret window, as a whole read of it is secret |
| [`intern_handle`](#intern_handle) | the handle type minted by `ctor` over `ops` a handle is identified by its constructor and operands alone, so an equal pair gives back the first handle whatever its name |
| [`is_handle`](#is_handle) | whether `tid` is itself a handle type |
| [`carries_handle`](#carries_handle) | a handle, or a pointer, array or secret over one: every shape that places a handle where the program would own its storage |
| [`handle_in_array`](#handle_in_array) | an array whose elements carry a handle, reached directly or through a pointer or secret |
| [`handle_pointer_pointer`](#handle_pointer_pointer) | a pointer to a pointer to a handle, through any secret: the innermost double indirection of every deeper one, so a chain of pointers over a handle matches at exactly one level |
| [`has_handle_pointer_pointer`](#has_handle_pointer_pointer) | a pointer to a pointer to a handle anywhere under the pointers, arrays and secrets of a type |
| [`intern_abi_type`](#intern_abi_type) | the opaque type an ABI defines, such as va_list |
| [`is_abi_type`](#is_abi_type) | whether `tid` is an opaque ABI type |
| [`handle_operand`](#handle_operand) | one operand of a handle type |
| [`intern_pack`](#intern_pack) | the one type of a variadic pack |
| [`intern_case_selector`](#intern_case_selector) | the type of a selector naming one case of a tag |
| [`is_case_selector`](#is_case_selector) | whether `tid` is a case selector type |
| [`is_pack`](#is_pack) | whether `tid` is the variadic pack type |
| [`case_selector_tag_type`](#case_selector_tag_type) | the tag a case selector type selects from |
| [`intern_nominal`](#intern_nominal) | the type of a named record, union or tag |
| [`intern_anonymous`](#intern_anonymous) | an anonymous recipe belongs to one source incarnation |
| [`intern_generic_param`](#intern_generic_param) | the type of one type parameter of a generic declaration |
| [`intern_function`](#intern_function) | the function type of a signature |
| [`with_timed`](#with_timed) | the function type `tid` with the constant-time mode `timed` in place of its own |
| [`is_fun_type`](#is_fun_type) | whether `tid` is a function type |
| [`is_markable_fun`](#is_markable_fun) | whether `tid` is a function type a constant-time mode may mark: a function type that is not C-variadic, as no constant-time function is |
| [`timed_of`](#timed_of) | the constant-time mode the function type `tid` states, TIMED_NONE for any other type |
| [`intern_instance`](#intern_instance) | the instance of a generic under type arguments |
| [`signature_converts`](#signature_converts) | whether a value of the function type `a` converts to the function type `b`: one signature, and `a` states a constant-time mode at least as strong as `b`'s, so the conversion only forgets a promise |
| [`field_projection_reset`](#field_projection_reset) | drop the current projection: every field table, discriminator, decoration and projected fact, and bump the projection generation |
| [`projected_get`](#projected_get) | the settled answer to projected fact `which` of `tid`, none while it is unsettled |
| [`projected_set`](#projected_set) | settle projected fact `which` of `tid` as `answer` |
| [`identity_get`](#identity_get) | the identity recipe gave `tid`, none until it is asked for |
| [`identity_set`](#identity_set) | record the identity recipe gave `tid` |
| [`field_table_stage`](#field_table_stage) | reserve room for field entries about to be staged in the current projection |
| [`field_table_stage_push`](#field_table_stage_push) | stage one field with no deprecation notice |
| [`field_table_stage_push_entry`](#field_table_stage_push_entry) | stage one field entry as given |
| [`field_table_publish`](#field_table_publish) | a table published over one holding other fields drops every projected fact, since any of them may have read the old one |
| [`decoration_of`](#decoration_of) | the layout decorations nominal `tid` declares under the current projection |
| [`decoration_set`](#decoration_set) | record the layout decorations nominal `tid` declares under the current projection |
| [`aggregate_nominal`](#aggregate_nominal) | the nominal a record, union, tag or instance is declared by |
| [`is_tag`](#is_tag) | whether `tid` is a tag or an instance of one |
| [`set_tag_discriminator`](#set_tag_discriminator) | record the discriminator type the tag behind `tid` declares, under the current projection |
| [`tag_discriminator`](#tag_discriminator) | the discriminator type the tag behind `tid` declares |
| [`tag_discriminator_bytes`](#tag_discriminator_bytes) | the declared discriminator width in bytes, or zero when the tag has no usable declaration |
| [`is_union`](#is_union) | whether `tid` is a union or an instance of one |
| [`is_record`](#is_record) | whether `tid` is a record or an instance of one |
| [`field_table_for`](#field_table_for) | the field table published for `ty` under the current projection |
| [`field_count`](#field_count) | how many fields or cases `ty`'s table holds |
| [`field_at`](#field_at) | one field or case of `ty`'s table |
| [`field_entry_at`](#field_entry_at) | entry `ix` of a published table, which holds more than `ix` entries |

| On `Projected` | |
| :--- | :--- |
| [`PROJECTED_HELD_SECRET`](#projected) | a value of the type holds a secret in its own storage (`contains_secret`) |
| [`PROJECTED_REACHES_SECRET`](#projected) | the type reaches a secret anywhere, through pointers, with an instance's arguments counted where its generic places them (`query.deep.secret`) |
| [`PROJECTED_SPELLS_SECRET`](#projected) | a secret is spelled anywhere the type reaches, through every edge and every instance argument (`query.reach.module_secret`) |

| Functions | |
| :--- | :--- |
| [`abi_type_c_name`](#abi_type_c_name) | the C type an ABI type tag stands for, absent for a tag outside the catalog |
| [`is_valid_discriminator_kind`](#is_valid_discriminator_kind) | whether a tag may declare kind `k` as its discriminator: u8, u16, u32 or u64 |

| Constants | |
| :--- | :--- |
| [`NO_HANDLE_CTOR`](#constants) | the constructor of a handle no target mints in this build: its operands are its declaration's site, so its identity is the declaration's |
| [`HANDLE_PART_WORD`](#constants) | a minted handle's constructor is the interned name of the kind the target's family mints, as `spirv.image`, and its operands are pairs, the part then its value: a word is the number itself, a type operand the type's id, and a named one the interned name of an enumerant its row's grammar states |
| [`HANDLE_PART_TYPE`](#constants), [`HANDLE_PART_NAME`](#constants), [`ABI_TYPE_VA_LIST`](#constants) | |
| [`TIMED_NONE`](#constants) | the constant-time modes a function type states, weakest first: a value of a marked type holds only a function declared in that mode or a stronger one, and the mode is spelled only by a `def` marked `#[constant_time]` |
| [`TIMED_SOFT`](#constants), [`TIMED_HARD`](#constants) | |
| [`FACT_PARAM`](#constants) | a generic parameter is spelled in the type: itself, or under a pointer, secret or array, in a signature or as an instance's argument |
| [`FACT_CARRIES_HANDLE`](#constants) | the type is a handle, or reaches one through pointers, arrays and secrets alone |

---

## TypeSecret

```mach
pub rec TypeSecret {
    base: TypeId;
}
```

the parts of a secret type

| Field | Type | |
| :--- | :--- | :--- |
| `base` | `TypeId` | the type the secret qualifier wraps |

## TypeHandle

```mach
pub rec TypeHandle {
    name:      mach.base.intern.StrId;
    ctor:      u32;
    ops_start: u32;
    ops_len:   u32;
}
```

the parts of a target handle type

| Field | Type | |
| :--- | :--- | :--- |
| `name` | `mach.base.intern.StrId` | the handle's interned name |
| `ctor` | `u32` | the constructor the target mints it by, NO_HANDLE_CTOR when none does |
| `ops_start` | `u32` | where its operands start in the store's handle operand pool |
| `ops_len` | `u32` | how many operands it has |

## TypeArray

```mach
pub rec TypeArray {
    base:  TypeId;
    count: u32;
}
```

the parts of an array type

| Field | Type | |
| :--- | :--- | :--- |
| `base` | `TypeId` | the element type |
| `count` | `u32` | the element count |

## TypeFun

```mach
pub rec TypeFun {
    ret_type:       TypeId;
    params_start:   u32;
    params_len:     u32;
    c_variadic:     bool;
    timed:          u8;
    next_in_bucket: TypeId;
}
```

the parts of a function type

| Field | Type | |
| :--- | :--- | :--- |
| `ret_type` | `TypeId` | the return type |
| `params_start` | `u32` | where its parameter types start in the store's parameter pool |
| `params_len` | `u32` | how many parameters it takes |
| `c_variadic` | `bool` | whether it takes C variadic arguments after its parameters |
| `timed` | `u8` | the constant-time mode it states, one of the `TIMED_` values, part of its identity |
| `next_in_bucket` | `TypeId` | the next function type whose signature hash shares its bucket, TYPE_NIL at the end |

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

the identity of a record, union or tag

| Field | Type | |
| :--- | :--- | :--- |
| `name` | `mach.base.intern.StrId` | the declared name |
| `owner` | `TypeOwner` | the module and file that declare it |
| `incarnation` | `u64` | the source incarnation of an anonymous aggregate, 0 for a named one |
| `syntax` | `mach.base.intern.StrId` | the structural recipe of an anonymous aggregate, STR_NIL for a named one |

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

whether two generic owners name the same declaration

| Parameter | |
| :--- | :--- |
| `a` | one owner |
| `b` | the other owner |
| **returns** | true when their module, file, name and kind all match |

### param_owner

```mach
pub fun param_owner(p: *TypeGenericParam) GenericOwner
```

the declaration that owns a generic parameter

| Parameter | |
| :--- | :--- |
| `p` | the parameter's parts |
| **returns** | its owning declaration |

### param_owned_by

```mach
pub fun param_owned_by(p: *TypeGenericParam, who: *GenericOwner) bool
```

whether a generic parameter belongs to declaration `who`

| Parameter | |
| :--- | :--- |
| `p` | the parameter's parts |
| `who` | the declaration |
| **returns** | true when `who` owns it |

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

one interned type

| Field | Type | |
| :--- | :--- | :--- |
| `kind` | `TypeKind` | which kind of type it is, which picks the member of `data` in use |
| `facts` | `u8` | what the type spells (`FACT_*`) |
| `data` | `uni { pointer: TypePointer; secret: TypeSecret; array: TypeArray; vector: TypeVector; handle: TypeHandle; abi: TypeAbi; fn: TypeFun; nominal: TypeNominal; generic_param: TypeGenericParam; instance: TypeInstance; case_selector: TypeCaseSelector; }` | the parts of the type, one member per kind that has any |

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

the type table: every type interned once and named by its `TypeId`

| Field | Type | |
| :--- | :--- | :--- |
| `a` | `*std.allocator.Allocator` | the allocator everything here is held in |
| `types` | `mach.base.handle.StableChunks[Type]` | the types, by index |
| `params` | `*TypeId` | the pool of function parameter types and instance arguments |
| `dedup` | `std.collections.map.MapBy[Type, TypeId]` | the type of each recipe, for every kind but functions, instances and handles |
| `fun_index` | `std.collections.map.Map[u64, TypeId]` | the newest function type per signature hash, heading its bucket chain |
| `instance_index` | `std.collections.map.Map[u64, TypeId]` | the newest instance per hash of its generic and arguments, heading its bucket chain |
| `handles` | `*TypeId` | every handle type, searched in full on each intern |
| `handle_ops` | `*u32` | the pool of handle operands |
| `prims` | `[type_prim.PRIM_COUNT]TypeId` | the type of each primitive, by catalog position |
| `vecs` | `[type_prim.VEC_COUNT]TypeId` | the seeded vector types, by position below VEC_COUNT |
| `fields` | `mach.type.field.Projection` | the current projection's field tables, discriminators and decorations |
| `scans` | `mach.type.scan.Sets` | the visited sets of the open scans |

### facts_of

```mach
pub fun facts_of(ti: *TypeInterner, tid: TypeId) u8
```

the facts of `tid`, none for the nil and error types

### init

```mach
pub fun init(ti: *TypeInterner, a: *std.allocator.Allocator) err[mach.base.fail.Fail]
```

set up an empty type store with every primitive and seeded vector interned

| Parameter | |
| :--- | :--- |
| `ti` | the store to fill, whose prior contents are not released |
| `a` | the allocator the store holds everything in |
| **returns** | ok, or the allocation failure, after which the store is already released |

### dnit

```mach
pub fun dnit(ti: *TypeInterner)
```

release a type store; a nil store holds nothing

### get

```mach
pub fun get(ti: *TypeInterner, tid: TypeId) opt[*Type]
```

the type `tid` names

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `tid` | the type |
| **returns** | the stored type, borrowed from the store, none for an id with no slot |

### type_count

```mach
pub fun type_count(ti: *TypeInterner) u32
```

how many types the store holds

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| **returns** | the count, one past the highest index issued |

### get_param

```mach
pub fun get_param(ti: *TypeInterner, index: u32) opt[TypeId]
```

one entry of the pool of function parameter types and instance arguments

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `index` | the pool position |
| **returns** | the type there, none past the pool's end |

### prim

```mach
pub fun prim(ti: *TypeInterner, kind: TypeKind) opt[TypeId]
```

the type of a primitive kind

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `kind` | the kind |
| **returns** | its interned type, none for a kind the catalog does not name |

### is_integer

```mach
pub fun is_integer(ti: *TypeInterner, tid: TypeId) bool
```

whether `tid`, with any secret qualifier stripped, is an integer primitive

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `tid` | the type asked about |
| **returns** | true when it is, false for every other type and an id the store does not hold |

### is_numeric

```mach
pub fun is_numeric(ti: *TypeInterner, tid: TypeId) bool
```

whether `tid`, with any secret qualifier stripped, is an integer or float primitive

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `tid` | the type asked about |
| **returns** | true when it is, false for every other type and an id the store does not hold |

### is_float

```mach
pub fun is_float(ti: *TypeInterner, tid: TypeId) bool
```

whether `tid`, with any secret qualifier stripped, is a float primitive

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `tid` | the type asked about |
| **returns** | true when it is, false for every other type and an id the store does not hold |

### is_pointer_like

```mach
pub fun is_pointer_like(ti: *TypeInterner, tid: TypeId) bool
```

whether `tid`, with any secret qualifier stripped, is held as an address: `ptr`, a typed pointer or a function

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `tid` | the type asked about |
| **returns** | true when it is, false for every other type and an id the store does not hold |

### is_pointer

```mach
pub fun is_pointer(ti: *TypeInterner, tid: TypeId) bool
```

whether `tid`, with any secret qualifier stripped, is a typed pointer

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `tid` | the type asked about |
| **returns** | true when it is, false for every other type and an id the store does not hold |

### deref_one

```mach
pub fun deref_one(ti: *TypeInterner, tid: TypeId) TypeId
```

the type a typed pointer points at, any secret qualifier on the pointer stripped

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `tid` | the pointer type |
| **returns** | the pointee, TYPE_ERROR when `tid` is not a typed pointer |

### is_secret

```mach
pub fun is_secret(ti: *TypeInterner, tid: TypeId) bool
```

whether `tid` is itself secret-qualified, not merely holding a secret

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `tid` | the type asked about |
| **returns** | true for a secret type, false for every other type and an id the store does not hold |

### is_vector

```mach
pub fun is_vector(ti: *TypeInterner, tid: TypeId) bool
```

whether `tid`, with any secret qualifier stripped, is a vector

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `tid` | the type asked about |
| **returns** | true when it is, false for every other type and an id the store does not hold |

### vector_element

```mach
pub fun vector_element(ti: *TypeInterner, tid: TypeId) TypeKind
```

the lane kind of a vector, any secret qualifier stripped

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `tid` | the vector type |
| **returns** | the lane kind, TYPE_VECTOR when `tid` is not a vector |

### vector_lanes

```mach
pub fun vector_lanes(ti: *TypeInterner, tid: TypeId) u32
```

the lane count of a vector, any secret qualifier stripped

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `tid` | the vector type |
| **returns** | the lane count, 0 when `tid` is not a vector |

### element_count

```mach
pub fun element_count(ti: *TypeInterner, tid: TypeId) opt[u32]
```

the element count of an array or the lane count of a vector, any secret qualifier stripped

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `tid` | the type asked about |
| **returns** | the count, none for any other type and for nil or the error type |

### vector_mask

```mach
pub fun vector_mask(ti: *TypeInterner, tid: TypeId) res[TypeId, mach.base.fail.Fail]
```

the unsigned vector a lane comparison of vector `tid` gives: as many lanes, each as wide

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `tid` | the vector type |
| **returns** | the mask type, u8 lanes for a lane width other than 8, 16, 32 or 64 bits, or the failure growing the store |

### strip_secret

```mach
pub fun strip_secret(ti: *TypeInterner, tid: TypeId) TypeId
```

`tid` without its secret qualifier

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `tid` | the type asked about |
| **returns** | the base of a secret type, `tid` itself for any other type or an id the store does not hold |

### mentions_generic_param

```mach
pub fun mentions_generic_param(ti: *TypeInterner, tid: TypeId) res[bool, mach.base.fail.Fail]
```

whether `tid` mentions a type parameter anywhere it is spelled; it never fails

### shape_kind

```mach
pub fun shape_kind(ti: *TypeInterner, tid: TypeId) TypeKind
```

the kind that decides how `tid` is shaped: a secret's base's, and an instance's generic's

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `tid` | the type asked about |
| **returns** | the kind, TYPE_U8 for nil, the error type or an id the store does not hold |

### is_u8

```mach
pub fun is_u8(ti: *TypeInterner, tid: TypeId) bool
```

whether `tid`, any secret qualifier stripped, is shaped as u8

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `tid` | the type asked about |
| **returns** | true when its shape kind is u8, false for nil, the error type and an id the store does not hold |

### is_u1

```mach
pub fun is_u1(ti: *TypeInterner, tid: TypeId) bool
```

whether `tid`, any secret qualifier stripped, is shaped as u1

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `tid` | the type asked about |
| **returns** | true when its shape kind is u1, false for nil, the error type and an id the store does not hold |

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

open a scan over the store's types at the next nesting level

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `scan` | the scan to open, closed again by type_scan_end |
| **returns** | ok, or the failure growing the visited sets, which leaves the scan closed |

### type_scan_end

```mach
pub fun type_scan_end(ti: *TypeInterner, scan: *TypeScan)
```

close a scan and every scan opened inside it

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `scan` | the scan to close |

### type_scan_mark

```mach
pub fun type_scan_mark(ti: *TypeInterner, scan: *TypeScan, tid: TypeId) res[VisitMark, mach.base.fail.Fail]
```

mark a type seen in an open scan

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `scan` | the open scan |
| `tid` | the type met |
| **returns** | VISIT_FRESH the first time, VISIT_SEEN after and for nil or the error type, or a failure for a closed scan or a set that cannot grow |

### pair_scan_begin

```mach
pub fun pair_scan_begin(ti: *TypeInterner, scan: *PairScan) err[mach.base.fail.Fail]
```

open a scan over pairs of the store's types at the next nesting level

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `scan` | the scan to open, closed again by pair_scan_end |
| **returns** | ok, or the failure growing the visited sets, which leaves the scan closed |

### pair_scan_end

```mach
pub fun pair_scan_end(ti: *TypeInterner, scan: *PairScan)
```

close a pair scan and every pair scan opened inside it

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `scan` | the scan to close |

### pair_scan_mark

```mach
pub fun pair_scan_mark(ti: *TypeInterner, scan: *PairScan, a: TypeId, b: TypeId) res[VisitMark, mach.base.fail.Fail]
```

mark an ordered pair of types seen in an open pair scan

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `scan` | the open scan |
| `a` | the first type |
| `b` | the second type |
| **returns** | VISIT_FRESH the first time, VISIT_SEEN after, or a failure for a closed scan or a set that cannot grow |

### intern_pointer

```mach
pub fun intern_pointer(ti: *TypeInterner, base: TypeId) res[TypeId, mach.base.fail.Fail]
```

the pointer type to `base`

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `base` | the pointee |
| **returns** | the interned type, or the failure growing the store |

### intern_secret

```mach
pub fun intern_secret(ti: *TypeInterner, base: TypeId) res[TypeId, mach.base.fail.Fail]
```

the secret type over `base`

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `base` | the type to qualify |
| **returns** | `base` itself when it is already secret, TYPE_ERROR for the error type, else the interned type or the failure growing the store |

### intern_array

```mach
pub fun intern_array(ti: *TypeInterner, base: TypeId, count: u32) res[TypeId, mach.base.fail.Fail]
```

the array type of `count` elements of `base`

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `base` | the element type |
| `count` | the element count |
| **returns** | the interned type, or the failure growing the store |

### intern_vector

```mach
pub fun intern_vector(ti: *TypeInterner, element: TypeKind, lanes: u32) res[TypeId, mach.base.fail.Fail]
```

the vector type of `lanes` lanes of `element`

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `element` | the lane kind |
| `lanes` | the lane count |
| **returns** | the interned type, or the failure growing the store |

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

the handle type minted by `ctor` over `ops`
a handle is identified by its constructor and operands alone, so an equal pair gives back the first handle whatever its name

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `name` | the handle's name, kept only with a new handle |
| `ctor` | its constructor, NO_HANDLE_CTOR for one no target mints |
| `ops` | its operands, copied into the store |
| `count` | how many operands |
| **returns** | the handle type, or the failure growing the store |

### is_handle

```mach
pub fun is_handle(ti: *TypeInterner, tid: TypeId) bool
```

whether `tid` is itself a handle type

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `tid` | the type asked about |
| **returns** | true when it is, false for every other type and an id the store does not hold |

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

the opaque type an ABI defines, such as va_list

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `name` | its name, kept with the first intern and no part of its identity |
| `tag` | which ABI type it is (`ABI_TYPE_*`) |
| `size` | its size in bytes |
| `align` | its alignment in bytes |
| **returns** | the interned type, or the failure growing the store |

### is_abi_type

```mach
pub fun is_abi_type(ti: *TypeInterner, tid: TypeId) bool
```

whether `tid` is an opaque ABI type

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `tid` | the type asked about |
| **returns** | true when it is, false for every other type and an id the store does not hold |

### handle_operand

```mach
pub fun handle_operand(ti: *TypeInterner, tid: TypeId, index: u32) u32
```

one operand of a handle type

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `tid` | the handle type |
| `index` | the operand's position |
| **returns** | the operand, 0 when `tid` is not a handle or has no operand at `index` |

### intern_pack

```mach
pub fun intern_pack(ti: *TypeInterner) res[TypeId, mach.base.fail.Fail]
```

the one type of a variadic pack

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| **returns** | the interned type, or the failure growing the store |

### intern_case_selector

```mach
pub fun intern_case_selector(ti: *TypeInterner, tag_type: TypeId, case_name: mach.base.intern.StrId, case_index: u32) res[TypeId, mach.base.fail.Fail]
```

the type of a selector naming one case of a tag

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `tag_type` | the tag |
| `case_name` | the case's name |
| `case_index` | the case's position in the tag |
| **returns** | the interned type, or the failure growing the store |

### is_case_selector

```mach
pub fun is_case_selector(ti: *TypeInterner, tid: TypeId) bool
```

whether `tid` is a case selector type

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `tid` | the type asked about |
| **returns** | true when it is, false for every other type and an id the store does not hold |

### is_pack

```mach
pub fun is_pack(ti: *TypeInterner, tid: TypeId) bool
```

whether `tid` is the variadic pack type

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `tid` | the type asked about |
| **returns** | true when it is, false for every other type and an id the store does not hold |

### case_selector_tag_type

```mach
pub fun case_selector_tag_type(ti: *TypeInterner, tid: TypeId) TypeId
```

the tag a case selector type selects from

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `tid` | the selector type |
| **returns** | the tag type, TYPE_NIL when `tid` is not a case selector |

### intern_nominal

```mach
pub fun intern_nominal(ti: *TypeInterner, name: mach.base.intern.StrId, owner: TypeOwner, kind: TypeKind) res[TypeId, mach.base.fail.Fail]
```

the type of a named record, union or tag

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `name` | its declared name |
| `owner` | the module and file that declare it |
| `kind` | TYPE_REC, TYPE_UNI or TYPE_TAG |
| **returns** | the interned type, or the failure growing the store |

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

the type of one type parameter of a generic declaration

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `name` | the parameter's name |
| `owner` | the module and file of the declaring generic |
| `owner_name` | the declaring generic's name |
| `owner_kind` | the declaring generic's kind |
| `index` | the parameter's position in its generic's list |
| **returns** | the interned type, or the failure growing the store |

### intern_function

```mach
pub fun intern_function(ti: *TypeInterner, ret_type: TypeId, params: *TypeId, count: u32, c_variadic: bool, timed: u8) res[TypeId, mach.base.fail.Fail]
```

the function type of a signature

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `ret_type` | the return type |
| `params` | the parameter types, copied, which may lie in the store's own pool |
| `count` | how many parameters |
| `c_variadic` | whether it takes C variadic arguments after them |
| `timed` | the constant-time mode it states, one of the `TIMED_` values |
| **returns** | the interned type, or a failure for nil `params` with a nonzero count or growing the store |

### with_timed

```mach
pub fun with_timed(ti: *TypeInterner, tid: TypeId, timed: u8) res[TypeId, mach.base.fail.Fail]
```

the function type `tid` with the constant-time mode `timed` in place of its own

### is_fun_type

```mach
pub fun is_fun_type(ti: *TypeInterner, tid: TypeId) bool
```

whether `tid` is a function type

### is_markable_fun

```mach
pub fun is_markable_fun(ti: *TypeInterner, tid: TypeId) bool
```

whether `tid` is a function type a constant-time mode may mark: a function type that is not C-variadic, as no
constant-time function is

### timed_of

```mach
pub fun timed_of(ti: *TypeInterner, tid: TypeId) u8
```

the constant-time mode the function type `tid` states, TIMED_NONE for any other type

### intern_instance

```mach
pub fun intern_instance(ti: *TypeInterner, nominal: TypeId, args: *TypeId, count: u32) res[TypeId, mach.base.fail.Fail]
```

the instance of a generic under type arguments

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `nominal` | the generic declaration |
| `args` | the type arguments, copied, which may lie in the store's own pool |
| `count` | how many arguments |
| **returns** | the interned type, or a failure for nil `args` with a nonzero count or growing the store |

### signature_converts

```mach
pub fun signature_converts(ti: *TypeInterner, a: TypeId, b: TypeId) bool
```

whether a value of the function type `a` converts to the function type `b`: one signature, and `a` states a
constant-time mode at least as strong as `b`'s, so the conversion only forgets a promise

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `a` | the type converted from |
| `b` | the type converted to |
| **returns** | true when `a` and `b` are the same type, or function types with one return, parameter list and variadic flag where `a`'s mode is at least `b`'s |

### field_projection_reset

```mach
pub fun field_projection_reset(ti: *TypeInterner)
```

drop the current projection: every field table, discriminator, decoration and projected fact, and bump the projection generation

| Parameter | |
| :--- | :--- |
| `ti` | the type store |

### projected_get

```mach
pub fun projected_get(ti: *TypeInterner, tid: TypeId, which: Projected) opt[bool]
```

the settled answer to projected fact `which` of `tid`, none while it is unsettled

### projected_set

```mach
pub fun projected_set(ti: *TypeInterner, tid: TypeId, which: Projected, answer: bool) err[mach.base.fail.Fail]
```

settle projected fact `which` of `tid` as `answer`

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `tid` | the type asked about |
| `which` | the fact (`PROJECTED_*`) |
| `answer` | its answer |
| **returns** | ok, with nothing recorded for nil or the error type, or the failure growing the record |

### identity_get

```mach
pub fun identity_get(ti: *TypeInterner, tid: TypeId) opt[u64]
```

the identity recipe gave `tid`, none until it is asked for

### identity_set

```mach
pub fun identity_set(ti: *TypeInterner, tid: TypeId, identity: u64) err[mach.base.fail.Fail]
```

record the identity recipe gave `tid`

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `tid` | the type asked about |
| `identity` | the identity, nonzero since 0 reads as unasked |
| **returns** | ok, with nothing recorded for nil or the error type, or the failure growing the record |

### field_table_stage

```mach
pub fun field_table_stage(ti: *TypeInterner, additional: u32) res[u32, mach.base.fail.Fail]
```

reserve room for field entries about to be staged in the current projection

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `additional` | how many entries to make room for |
| **returns** | where the next staged run starts, or the failure growing the pool |

### field_table_stage_push

```mach
pub fun field_table_stage_push(ti: *TypeInterner, name: mach.base.intern.StrId, ty: TypeId) err[mach.base.fail.Fail]
```

stage one field with no deprecation notice

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `name` | the field's name |
| `ty` | the field's type |
| **returns** | ok, or the failure growing the pool |

### field_table_stage_push_entry

```mach
pub fun field_table_stage_push_entry(ti: *TypeInterner, fe: FieldEntry) err[mach.base.fail.Fail]
```

stage one field entry as given

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `fe` | the entry |
| **returns** | ok, or the failure growing the pool |

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

record the layout decorations nominal `tid` declares under the current projection

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `tid` | the nominal |
| `d` | its decorations |
| **returns** | ok, or the failure growing the table |

### aggregate_nominal

```mach
pub fun aggregate_nominal(ti: *TypeInterner, tid: TypeId) TypeId
```

the nominal a record, union, tag or instance is declared by

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `tid` | the type asked about |
| **returns** | `tid` itself for a record, union or tag, an instance's generic, TYPE_ERROR for any other type |

### is_tag

```mach
pub fun is_tag(ti: *TypeInterner, tid: TypeId) bool
```

whether `tid` is a tag or an instance of one

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `tid` | the type asked about |
| **returns** | true when it is |

### set_tag_discriminator

```mach
pub fun set_tag_discriminator(ti: *TypeInterner, tid: TypeId, disc: TypeId) err[mach.base.fail.Fail]
```

record the discriminator type the tag behind `tid` declares, under the current projection

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `tid` | the tag, or an instance of one |
| `disc` | the discriminator type, TYPE_NIL to clear it |
| **returns** | ok, with nothing recorded when `tid` is no tag, or the failure growing the table |

### tag_discriminator

```mach
pub fun tag_discriminator(ti: *TypeInterner, tid: TypeId) TypeId
```

the discriminator type the tag behind `tid` declares

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `tid` | the tag, or an instance of one |
| **returns** | the discriminator type, TYPE_NIL when `tid` is no tag or declares none |

### tag_discriminator_bytes

```mach
pub fun tag_discriminator_bytes(ti: *TypeInterner, tid: TypeId) u32
```

the declared discriminator width in bytes, or zero when the tag has no usable declaration

### is_union

```mach
pub fun is_union(ti: *TypeInterner, tid: TypeId) bool
```

whether `tid` is a union or an instance of one

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `tid` | the type asked about |
| **returns** | true when it is |

### is_record

```mach
pub fun is_record(ti: *TypeInterner, tid: TypeId) bool
```

whether `tid` is a record or an instance of one

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `tid` | the type asked about |
| **returns** | true when it is |

### field_table_for

```mach
pub fun field_table_for(ti: *TypeInterner, ty: TypeId) opt[*FieldTable]
```

the field table published for `ty` under the current projection

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `ty` | the aggregate |
| **returns** | the table, borrowed, none while it has none |

### field_count

```mach
pub fun field_count(ti: *TypeInterner, ty: TypeId) u32
```

how many fields or cases `ty`'s table holds

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `ty` | the aggregate |
| **returns** | the count, 0 while it has no table |

### field_at

```mach
pub fun field_at(ti: *TypeInterner, ty: TypeId, ix: u32) opt[*FieldEntry]
```

one field or case of `ty`'s table

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `ty` | the aggregate |
| `ix` | the field's position |
| **returns** | the entry, borrowed, none when there is no table or no field at `ix` |

### field_entry_at

```mach
pub fun field_entry_at(ti: *TypeInterner, ft: *FieldTable, ix: u32) *FieldEntry
```

entry `ix` of a published table, which holds more than `ix` entries

| Parameter | |
| :--- | :--- |
| `ti` | the type store |
| `ft` | the table |
| `ix` | the entry's position |
| **returns** | the entry, borrowed |

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

whether a tag may declare kind `k` as its discriminator: u8, u16, u32 or u64

| Parameter | |
| :--- | :--- |
| `k` | the kind |
| **returns** | true when it may |

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `NO_HANDLE_CTOR` | `u32` | `0xFFFFFFFF` | the constructor of a handle no target mints in this build: its operands are its declaration's site, so its identity is the declaration's |
| `HANDLE_PART_WORD` | `u32` | `0` | a minted handle's constructor is the interned name of the kind the target's family mints, as `spirv.image`, and its operands are pairs, the part then its value: a word is the number itself, a type operand the type's id, and a named one the interned name of an enumerant its row's grammar states |
| `HANDLE_PART_TYPE` | `u32` | `1` |  |
| `HANDLE_PART_NAME` | `u32` | `2` |  |
| `ABI_TYPE_VA_LIST` | `u32` | `0` |  |
| `TIMED_NONE` | `u8` | `0` | the constant-time modes a function type states, weakest first: a value of a marked type holds only a function declared in that mode or a stronger one, and the mode is spelled only by a `def` marked `#[constant_time]` |
| `TIMED_SOFT` | `u8` | `1` |  |
| `TIMED_HARD` | `u8` | `2` |  |
| `FACT_PARAM` | `u8` | `1` | a generic parameter is spelled in the type: itself, or under a pointer, secret or array, in a signature or as an instance's argument |
| `FACT_CARRIES_HANDLE` | `u8` | `4` | the type is a handle, or reaches one through pointers, arrays and secrets alone |

