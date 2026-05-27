# mach.lang.type

Semantic type universe shared across sema, middle-end, and backend.
[`TypeInterner`](#typeinterner) lives on the [session](session.md) next
to [`intern.Interner`](intern.md#interner). It owns every
[`Type`](#type) record produced during sema and hands out stable
[`TypeId`](#typeid) handles; equality of two [`TypeId`](#typeid)s is
identity equality.

## Types

### `TypeId`

```mach
pub def TypeId: u32;
```

Stable handle into a [`TypeInterner`](#typeinterner)'s universe.

### `TypeKind`

```mach
pub def TypeKind: u8;
```

Discriminator for [`Type.data`](#type). See [Constants](#constants) for
the enumerated values.

### `TypePointer`

```mach
pub rec TypePointer {
    base: TypeId;
}
```

Payload for a `TYPE_POINTER`.

| Field | Type                  | Description    |
|-------|-----------------------|----------------|
| base  | [`TypeId`](#typeid)   | Pointee type.  |

### `TypeArray`

```mach
pub rec TypeArray {
    base:  TypeId;
    count: u32;
}
```

Payload for a `TYPE_ARRAY`.

| Field | Type                  | Description           |
|-------|-----------------------|-----------------------|
| base  | [`TypeId`](#typeid)   | Element type.         |
| count | `u32`                 | Static element count. |

### `TypeFun`

```mach
pub rec TypeFun {
    ret:          TypeId;
    params_start: u32;
    params_len:   u32;
    variadic:     bool;
}
```

Payload for a `TYPE_FUN`. [`ret`](#typefun) `== `[`TYPE_NIL`](#type_nil)
signals "no return value". Parameter types live in the
[`ParamPool`](#parampool) at
`[params_start..params_start+params_len)`.

| Field        | Type                  | Description                                      |
|--------------|-----------------------|--------------------------------------------------|
| ret          | [`TypeId`](#typeid)   | Return type. [`TYPE_NIL`](#type_nil) means "no return". |
| params_start | `u32`                 | Index into the [`ParamPool`](#parampool).        |
| params_len   | `u32`                 | Number of fixed parameter types.                 |
| variadic     | `bool`                | `true` for a variadic signature (`fun(T, ...) R`). Participates in identity — a variadic and non-variadic signature with the same `ret` / params are distinct types. |

### `TypeNominal`

```mach
pub rec TypeNominal {
    name:   intern.StrId;
    origin: sess.ModuleId;
    decl:   id.DeclId;
}
```

Payload for `TYPE_REC` and `TYPE_UNI`. Two nominal types are identical
iff `(origin, decl)` match. `name` is carried for diagnostics only and
does not participate in identity.

| Field  | Type                                          | Description                              |
|--------|-----------------------------------------------|------------------------------------------|
| name   | [`intern.StrId`](intern.md#strid)             | Interned identifier (diagnostic only).   |
| origin | [`sess.ModuleId`](session.md#moduleid)        | Declaring module.                        |
| decl   | [`id.DeclId`](fe/ast/id.md#declid)            | Declaration in `origin`'s `Ast`.         |

### `TypeGenericParam`

```mach
pub rec TypeGenericParam {
    name:  intern.StrId;
    owner: id.DeclId;
    index: u32;
}
```

Payload for `TYPE_GENERIC_PARAM`. Identity is `(owner, index)`.

| Field | Type                                       | Description                                      |
|-------|--------------------------------------------|--------------------------------------------------|
| name  | [`intern.StrId`](intern.md#strid)          | Interned identifier (diagnostic only).           |
| owner | [`id.DeclId`](fe/ast/id.md#declid)         | Enclosing generic decl.                          |
| index | `u32`                                      | Ordinal among the owner's generic parameters.    |

### `TypeInstance`

```mach
pub rec TypeInstance {
    target_origin: sess.ModuleId;
    target_decl:   id.DeclId;
    args_start:    u32;
    args_len:      u32;
}
```

Payload for `TYPE_INSTANCE`. A concrete instantiation of a generic
declaration: the generic identified by `(target_origin, target_decl)`
applied to the argument `TypeId`s at
`[args_start, args_start+args_len)` in the
[`TypeInterner.params`](#typeinterner) pool. Two instances are
identical iff `(target_origin, target_decl)` match and the args
sequences are element-wise equal.

| Field         | Type                                          | Description                                                |
|---------------|-----------------------------------------------|------------------------------------------------------------|
| target_origin | [`sess.ModuleId`](session.md#moduleid)        | Module that declared the generic.                          |
| target_decl   | [`id.DeclId`](fe/ast/id.md#declid)            | Declaration in `target_origin`'s [`Ast`](fe/ast.md#ast).   |
| args_start    | `u32`                                         | Index into [`TypeInterner.params`](#typeinterner).         |
| args_len      | `u32`                                         | Number of concrete argument types.                         |

### `Type`

```mach
pub rec Type {
    kind: TypeKind;
    data: uni {
        pointer:       TypePointer;
        array:         TypeArray;
        fun:           TypeFun;
        nominal:       TypeNominal;
        generic_param: TypeGenericParam;
        instance:      TypeInstance;
    };
}
```

One type entry in the universe. Primitives use no payload.

| Field | Type                                | Description                                |
|-------|-------------------------------------|--------------------------------------------|
| kind  | [`TypeKind`](#typekind)             | Active discriminator.                      |
| data  | `uni { ... }`                         | Kind-specific payload (see field below).   |

| `data` variant | Type                                            | Active when `kind` is ...          |
|----------------|-------------------------------------------------|----------------------------------|
| pointer        | [`TypePointer`](#typepointer)                   | `TYPE_POINTER`                   |
| array          | [`TypeArray`](#typearray)                       | `TYPE_ARRAY`                     |
| fun            | [`TypeFun`](#typefun)                           | `TYPE_FUN`                       |
| nominal        | [`TypeNominal`](#typenominal)                   | `TYPE_REC`, `TYPE_UNI`           |
| generic_param  | [`TypeGenericParam`](#typegenericparam)         | `TYPE_GENERIC_PARAM`             |
| instance       | [`TypeInstance`](#typeinstance)                 | `TYPE_INSTANCE`                  |

### `TypeInterner`

```mach
pub rec TypeInterner {
    alloc: *Allocator;

    types:    *Type;
    type_len: u32;
    type_cap: u32;

    params:     ParamPool;

    dedup:     map.Map[Type, TypeId];
    funs:      map.Map[FunKey, TypeId];
    instances: map.Map[InstanceKey, TypeId];

    prims: [12]TypeId;
}
```

| Field      | Type                                  | Description                                                              |
|------------|---------------------------------------|--------------------------------------------------------------------------|
| alloc      | `*Allocator`                          | Allocator backing every owned array.                                     |
| types      | [`*Type`](#type)                      | Universe of [`Type`](#type) records indexed by [`TypeId`](#typeid).      |
| type_len   | `u32`                                 | Number of types stored.                                                  |
| type_cap   | `u32`                                 | Allocated slots in `types`.                                              |
| params     | [`ParamPool`](#parampool)             | Chunked pool of [`TypeId`](#typeid)s referenced by [`TypeFun`](#typefun) and [`TypeInstance`](#typeinstance). |
| dedup      | `map.Map[Type, TypeId]`               | Dedup table for self-contained compound kinds (`TYPE_POINTER`, `TYPE_ARRAY`, `TYPE_REC`, `TYPE_UNI`, `TYPE_GENERIC_PARAM`). Key is the [`Type`](#type) record itself. |
| funs       | `map.Map[FunKey, TypeId]`             | Dedup table for [`TYPE_FUN`](#typefun) — key shape forces structural identity through the params pool. |
| instances  | `map.Map[InstanceKey, TypeId]`        | Dedup table for [`TYPE_INSTANCE`](#typeinstance) — key shape forces structural identity through the args pool. |
| prims      | `[12]TypeId`                          | Pre-allocated [`TypeId`](#typeid)s for primitive kinds, indexed by [`TypeKind`](#typekind) value. |

[`TypeId`](#typeid) equality always means structural equality. The
three dedup tables guarantee this — every shape is canonicalised at
intern time regardless of which kind family it belongs to.

### `ParamPool`

```mach
pub rec ParamPool {
    chunks:      *Chunk;
    chunk_count: u32;
    chunk_cap:   u32;
    next_free:   u32;
}

rec Chunk {
    data: *TypeId;
    len:  u32;
}
```

Chunked storage for the parameter / argument sequences used by
[`TypeFun`](#typefun) and [`TypeInstance`](#typeinstance). The pool
guarantees that **a [`TypeId`](#typeid) once placed never moves**:
each chunk's `data` allocation is fixed for the chunk's lifetime, and
chunks are never freed or compacted while the interner is alive. New
chunks are appended when the active chunk fills.

The descriptor array (`chunks` itself) may reallocate as new chunks
are appended — only the bytes each `Chunk.data` points to are
stable. [`FunKey`](#funkey) and [`InstanceKey`](#instancekey) hold
logical pool offsets that the hash / equality callbacks dereference;
the callbacks reach the live data through the current `chunks` array,
which is always available via the owning `TypeInterner` context.

| Field       | Type        | Description                                                |
|-------------|-------------|------------------------------------------------------------|
| chunks      | `*Chunk`    | Array of chunk descriptors. May reallocate when a new chunk is appended; per-chunk `data` allocations are stable. |
| chunk_count | `u32`       | Number of allocated chunks.                                |
| chunk_cap   | `u32`       | Allocated slots in `chunks`.                               |
| next_free   | `u32`       | Logical pool offset of the next free slot — also the total `TypeId` count across all chunks. |

A logical offset `n` is resolved to physical storage by computing
`chunk_id = n / PARAMS_CHUNK_SIZE`, `slot = n % PARAMS_CHUNK_SIZE`,
then reading `chunks[chunk_id].data[slot]`.

### `FunKey`

```mach
rec FunKey {
    ret_ty:       TypeId;
    params_start: u32;
    params_len:   u32;
    variadic:     bool;
}
```

Lookup key for the [`funs`](#typeinterner) map.
`(params_start, params_len)` mirror the same fields on
[`TypeFun`](#typefun) — a slice into [`TypeInterner.params`](#typeinterner).
The map's hash and equality callbacks dereference the slice through
the [`ParamPool`](#parampool), which guarantees that placed `TypeId`s
never move. `variadic` participates in the key so variadic and
non-variadic signatures dedup to distinct [`TypeId`](#typeid)s.

### `InstanceKey`

```mach
rec InstanceKey {
    target_origin: sess.ModuleId;
    target_decl:   id.DeclId;
    args_start:    u32;
    args_len:      u32;
}
```

Lookup key for the [`instances`](#typeinterner) map.
`(args_start, args_len)` mirror the same fields on
[`TypeInstance`](#typeinstance) — a slice into the
[`ParamPool`](#parampool).

## Constants

### `TYPE_NIL`

```mach
pub val TYPE_NIL: TypeId = 0xFFFFFFFF;
```

Absent-type sentinel. Also used in [`TypeFun.ret`](#typefun) to signal
"no return".

### `TYPE_*` kinds

```mach
pub val TYPE_U8:            TypeKind = 0;
pub val TYPE_U16:           TypeKind = 1;
pub val TYPE_U32:           TypeKind = 2;
pub val TYPE_U64:           TypeKind = 3;
pub val TYPE_I8:            TypeKind = 4;
pub val TYPE_I16:           TypeKind = 5;
pub val TYPE_I32:           TypeKind = 6;
pub val TYPE_I64:           TypeKind = 7;
pub val TYPE_F32:           TypeKind = 8;
pub val TYPE_F64:           TypeKind = 9;
pub val TYPE_PTR:           TypeKind = 10;
pub val TYPE_STR:           TypeKind = 11;
pub val TYPE_POINTER:       TypeKind = 12;
pub val TYPE_ARRAY:         TypeKind = 13;
pub val TYPE_FUN:           TypeKind = 14;
pub val TYPE_REC:           TypeKind = 15;
pub val TYPE_UNI:           TypeKind = 16;
pub val TYPE_GENERIC_PARAM: TypeKind = 17;
pub val TYPE_INSTANCE:      TypeKind = 18;
pub val TYPE_ERROR:         TypeKind = 255;
```

[`TypeKind`](#typekind) values. Primitives occupy ids 0..11 in the
interner; their kind is also their pre-allocated [`TypeId`](#typeid).

| Constant                | Value | Category  | Notes                                          |
|-------------------------|-------|-----------|------------------------------------------------|
| `TYPE_U8` .. `TYPE_U64` | 0..3  | primitive | Unsigned integers.                             |
| `TYPE_I8` .. `TYPE_I64` | 4..7  | primitive | Signed integers.                               |
| `TYPE_F32`, `TYPE_F64`  | 8..9  | primitive | Floating point.                                |
| `TYPE_PTR`              | 10    | primitive | Untyped raw pointer (`ptr` keyword).           |
| `TYPE_STR`              | 11    | primitive | Compiler-builtin fat-string record (singleton).|
| `TYPE_POINTER`          | 12    | compound  | Typed `*T`.                                    |
| `TYPE_ARRAY`            | 13    | compound  | `[N]T` at declaration sites.                   |
| `TYPE_FUN`              | 14    | compound  | Function signature.                            |
| `TYPE_REC`              | 15    | compound  | Nominal record.                                |
| `TYPE_UNI`              | 16    | compound  | Nominal union.                                 |
| `TYPE_GENERIC_PARAM`    | 17    | compound  | Generic parameter in a declaration site.       |
| `TYPE_INSTANCE`         | 18    | compound  | Concrete instantiation of a generic.           |
| (reserved)              | 19..254 | —       | Reserved for future kinds; do not assign.      |
| `TYPE_ERROR`            | 255   | poison    | Sema-produced absorbing type.                  |

```mach
val PRIM_COUNT:         u32 = 12;
val INITIAL_TYPE_CAP:   u32 = 64;
val PARAMS_CHUNK_SIZE:  u32 = 64;
```

| Constant              | Description                                                |
|-----------------------|------------------------------------------------------------|
| `PRIM_COUNT`          | Number of primitive [`TypeKind`](#typekind) values; matches the size of [`TypeInterner.prims`](#typeinterner). |
| `INITIAL_TYPE_CAP`    | Starting capacity for [`TypeInterner.types`](#typeinterner). The array doubles on overflow. |
| `PARAMS_CHUNK_SIZE`   | Number of [`TypeId`](#typeid) slots per chunk in [`ParamPool`](#parampool). New chunks are appended on overflow; existing chunks never move. |

## Functions

### `init`

```mach
pub fun init(alloc: *Allocator) Result[TypeInterner, str]
```

Constructs an empty interner and pre-allocates the 12 primitive
[`TypeId`](#typeid)s. Errors on allocation failure; on failure no
partial state is observable.

| Param | Type         | Description                              |
|-------|--------------|------------------------------------------|
| alloc | `*Allocator` | Allocator used for all owned storage.    |

Returns the populated [`TypeInterner`](#typeinterner), or an allocation
error.

### `dnit`

```mach
pub fun dnit(ti: *TypeInterner)
```

Releases the types array, the [`ParamPool`](#parampool) (every chunk
plus the chunks array), and all three dedup tables
([`dedup`](#typeinterner), [`funs`](#typeinterner),
[`instances`](#typeinterner)). `nil` is a no-op. After `dnit` every
[`TypeId`](#typeid) previously issued is invalid.

| Param | Type                             | Description                          |
|-------|----------------------------------|--------------------------------------|
| ti    | [`*TypeInterner`](#typeinterner) | Interner to tear down. `nil` is a no-op.|

### `prim`

```mach
pub fun prim(ti: *TypeInterner, kind: TypeKind) Option[TypeId]
```

Returns the pre-allocated [`TypeId`](#typeid) for a primitive kind, or
`none` when `kind` is not a primitive (i.e., `kind >= 12`).

| Param | Type                             | Description                                       |
|-------|----------------------------------|---------------------------------------------------|
| ti    | [`*TypeInterner`](#typeinterner) | The interner.                                     |
| kind  | [`TypeKind`](#typekind)          | A primitive kind (`TYPE_U8` .. `TYPE_STR`).       |

Returns `some(TypeId)` for a primitive kind, `none` otherwise.

### `intern_pointer`

```mach
pub fun intern_pointer(ti: *TypeInterner, base: TypeId) Result[TypeId, str]
```

Returns the [`TypeId`](#typeid) for `*base`. Deduped — the same `base`
always returns the same [`TypeId`](#typeid).

| Param | Type                             | Description       |
|-------|----------------------------------|-------------------|
| ti    | [`*TypeInterner`](#typeinterner) | The interner.     |
| base  | [`TypeId`](#typeid)              | Pointee type.     |

Returns the [`TypeId`](#typeid) for `*base`, or an allocation error.

### `intern_array`

```mach
pub fun intern_array(ti: *TypeInterner, base: TypeId, count: u32) Result[TypeId, str]
```

Returns the [`TypeId`](#typeid) for `[count]base`. Deduped on
`(base, count)`.

| Param | Type                             | Description       |
|-------|----------------------------------|-------------------|
| ti    | [`*TypeInterner`](#typeinterner) | The interner.     |
| base  | [`TypeId`](#typeid)              | Element type.     |
| count | `u32`                            | Element count.    |

Returns the [`TypeId`](#typeid) for `[count]base`, or an allocation
error.

### `intern_nominal`

```mach
pub fun intern_nominal(ti: *TypeInterner, name: intern.StrId, origin: sess.ModuleId, decl: id.DeclId, kind: TypeKind) Result[TypeId, str]
```

Returns the [`TypeId`](#typeid) for a nominal `rec` or `uni` referenced
at `(origin, decl)`. Deduped on `(origin, decl)`. `kind` must be
`TYPE_REC` or `TYPE_UNI`; `name` is stored for diagnostics only.

| Param  | Type                                          | Description                                      |
|--------|-----------------------------------------------|--------------------------------------------------|
| ti     | [`*TypeInterner`](#typeinterner)              | The interner.                                    |
| name   | [`intern.StrId`](intern.md#strid)             | Interned identifier (diagnostic only).           |
| origin | [`sess.ModuleId`](session.md#moduleid)        | Declaring module.                                |
| decl   | [`id.DeclId`](fe/ast/id.md#declid)            | Declaration in `origin`'s `Ast`.                 |
| kind   | [`TypeKind`](#typekind)                       | `TYPE_REC` or `TYPE_UNI`.                        |

Returns the [`TypeId`](#typeid) for the nominal type, or an allocation
error.

### `intern_generic_param`

```mach
pub fun intern_generic_param(ti: *TypeInterner, name: intern.StrId, owner: id.DeclId, index: u32) Result[TypeId, str]
```

Returns the [`TypeId`](#typeid) for a generic parameter placeholder.
Deduped on `(owner, index)`. `name` is stored for diagnostics only.

| Param | Type                                       | Description                                       |
|-------|--------------------------------------------|---------------------------------------------------|
| ti    | [`*TypeInterner`](#typeinterner)           | The interner.                                     |
| name  | [`intern.StrId`](intern.md#strid)          | Interned identifier (diagnostic only).            |
| owner | [`id.DeclId`](fe/ast/id.md#declid)         | Enclosing generic decl.                           |
| index | `u32`                                      | Ordinal among the owner's generic parameters.     |

Returns the [`TypeId`](#typeid) for the generic param, or an allocation
error.

### `intern_instance`

```mach
pub fun intern_instance(ti: *TypeInterner, target_origin: sess.ModuleId, target_decl: id.DeclId, args: *TypeId, count: u32) Result[TypeId, str]
```

Returns the [`TypeId`](#typeid) for a generic instantiation of
`(target_origin, target_decl)` with the given concrete argument types.
Looks up [`InstanceKey`](#instancekey) in
[`ti.instances`](#typeinterner); on hit returns the cached
[`TypeId`](#typeid). On miss, copies `args` into the
[`params`](#typeinterner) pool, appends a fresh
[`TypeInstance`](#typeinstance) record, and registers the new key.
Identity is structural across `(target_origin, target_decl, args*)`.

| Param         | Type                                          | Description                                              |
|---------------|-----------------------------------------------|----------------------------------------------------------|
| ti            | [`*TypeInterner`](#typeinterner)              | The interner.                                            |
| target_origin | [`sess.ModuleId`](session.md#moduleid)        | Module that declared the generic.                        |
| target_decl   | [`id.DeclId`](fe/ast/id.md#declid)            | Declaration in `target_origin`'s [`Ast`](fe/ast.md#ast). |
| args          | [`*TypeId`](#typeid)                          | Pointer to a contiguous array of concrete arg types.     |
| count         | `u32`                                         | Number of argument types.                                |

Returns the (deduped) [`TypeId`](#typeid) for the instance, or an
allocation error.

### `intern_function`

```mach
pub fun intern_function(ti: *TypeInterner, ret_ty: TypeId, params: *TypeId, count: u32, variadic: bool) Result[TypeId, str]
```

Returns the [`TypeId`](#typeid) for a function signature with the
given return type, parameter list, and variadic flag. Looks up
[`FunKey`](#funkey) in [`ti.funs`](#typeinterner); on hit returns the
cached [`TypeId`](#typeid). On miss, copies `params` into the
[`ParamPool`](#parampool), appends a fresh [`TypeFun`](#typefun)
record, and registers the new key. Identity is structural across
`(ret_ty, params*, variadic)`.

| Param    | Type                             | Description                                      |
|----------|----------------------------------|--------------------------------------------------|
| ti       | [`*TypeInterner`](#typeinterner) | The interner.                                    |
| ret_ty   | [`TypeId`](#typeid)              | Return type. [`TYPE_NIL`](#type_nil) for "no return value". |
| params   | [`*TypeId`](#typeid)             | Pointer to a contiguous array of parameter ids.  |
| count    | `u32`                            | Number of fixed parameter types.                 |
| variadic | `bool`                           | `true` for a variadic signature.                 |

Returns the (deduped) [`TypeId`](#typeid) for the function, or an
allocation error.

### `get`

```mach
pub fun get(ti: *TypeInterner, tid: TypeId) Option[*Type]
```

Returns a pointer into `ti.types`, or `none` when `tid` is out of
range. Invalidated by a later intern that grows the types array.

| Param | Type                             | Description                              |
|-------|----------------------------------|------------------------------------------|
| ti    | [`*TypeInterner`](#typeinterner) | The interner.                            |
| tid   | [`TypeId`](#typeid)              | TypeId returned by a previous intern call.|

Returns `some([`*Type`](#type))` into `ti.types` when `tid` is in
range, `none` otherwise.

### `get_param`

```mach
pub fun get_param(ti: *TypeInterner, index: u32) Option[TypeId]
```

Returns the parameter [`TypeId`](#typeid) at the given logical pool
offset (typically `fun.params_start + i`), or `none` when `index` is
out of range. Wraps [`params_get`](#internal-helpers); the internal
chunk walk is transparent to callers.

| Param | Type                             | Description                                      |
|-------|----------------------------------|--------------------------------------------------|
| ti    | [`*TypeInterner`](#typeinterner) | The interner.                                    |
| index | `u32`                            | Logical pool offset (typically `fun.params_start + i`). |

Returns `some(TypeId)` for a valid slot, `none` otherwise.

## Internal helpers

File-private; listed for reference.

| Function          | Role                                                                                              |
|-------------------|---------------------------------------------------------------------------------------------------|
| `intern_dedup`    | Common path for [`intern_pointer`](#intern_pointer) / [`intern_array`](#intern_array) / [`intern_nominal`](#intern_nominal) / [`intern_generic_param`](#intern_generic_param): looks up `t` in [`ti.dedup`](#typeinterner); on hit returns the cached [`TypeId`](#typeid); on miss appends via `append_type` and registers in the map. |
| `append_type`     | Appends a [`Type`](#type) record to [`ti.types`](#typeinterner), growing on demand. Skips the dedup map. |
| `params_reserve`  | Ensures [`ti.params`](#typeinterner) has at least `need` free slots starting at `next_free`. Appends new [`Chunk`](#parampool)s as needed; never reallocates an existing chunk. |
| `params_push`     | Writes one [`TypeId`](#typeid) at `next_free` (allocating a new chunk if necessary) and increments `next_free`. |
| `params_get`      | Resolves a logical offset to a `*TypeId` by walking the chunk list. |
| `hash_type`       | Structural FNV-1a hash over a [`Type`](#type) record's identity-bearing fields, built on `std.crypto.hash.fnv1a` primitives. Used as the `hash_fn` for `ti.dedup`. |
| `eq_type`         | Structural equality over [`Type`](#type) records, compatible with `hash_type`. Used as the `eq_fn` for `ti.dedup`. |

## Dependencies

`std.types.bool`, `std.types.option`, `std.types.size`,
`std.types.string`, `std.types.result`, `std.allocator`,
`std.collections.map`, `std.crypto.hash.fnv1a`,
[`mach.lang.intern`](intern.md), [`mach.lang.session`](session.md),
[`mach.lang.fe.ast.id`](fe/ast/id.md).
