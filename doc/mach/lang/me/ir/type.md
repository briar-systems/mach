# mach.lang.me.ir.type

IR-level type system. A small, machine-oriented subset of types
that the source-level [`lang.type`](../../type.md) lowers into.
Source-level features that don't survive the lowering boundary
(generics, comptime parameters, type instances, type aliases) are
resolved away before IR construction; the IR only sees concrete
machine shapes.

Source is `new/lang/me/ir/type.mach` (currently empty).

## Types

### `IrTypeId`

```mach
pub def IrTypeId: u32;
```

Module-local handle into the per-module
[`IrTypeTable`](#irtypetable). Stable for the lifetime of the
[`ir.Module`](../ir.md#module).

### `IrTypeKind`

```mach
pub def IrTypeKind: u8;
```

Discriminator for what kind of IR type an entry encodes.

### `IrTypeStruct`

```mach
pub rec IrTypeStruct {
    fields:      *IrTypeId;
    field_count: u32;
}
```

Payload for an `IRT_STRUCT` — an aggregate with positional fields.

| Field       | Type                     | Description              |
|-------------|--------------------------|--------------------------|
| fields      | [`*IrTypeId`](#irtypeid) | Aggregate field types.   |
| field_count | `u32`                    | Length of `fields`.      |

### `IrTypeFn`

```mach
pub rec IrTypeFn {
    ret:         IrTypeId;
    params:      *IrTypeId;
    param_count: u32;
    varargs:     bool;
}
```

Payload for an `IRT_FN` — a function signature.

| Field       | Type                     | Description                                          |
|-------------|--------------------------|------------------------------------------------------|
| ret         | [`IrTypeId`](#irtypeid)  | Return type ([`IRT_VOID`](#constants) for void).     |
| params      | [`*IrTypeId`](#irtypeid) | Parameter types.                                     |
| param_count | `u32`                    | Length of `params`.                                  |
| varargs     | `bool`                   | `true` if the signature accepts varargs.             |

### `IrType`

```mach
pub rec IrType {
    kind: IrTypeKind;
    data: uni {
        bits:    u32;
        element: IrTypeId;
        struct_: IrTypeStruct;
        fn:      IrTypeFn;
    };
}
```

One entry in the IR type universe. `IRT_VOID` uses no payload.

| Field | Type                          | Description                          |
|-------|-------------------------------|--------------------------------------|
| kind  | [`IrTypeKind`](#irtypekind)   | Active discriminator.                |
| data  | `uni { ... }`                   | Kind-specific payload (see below).   |

| `data` variant | Type                                | Active when `kind` is ... |
|----------------|-------------------------------------|---------------------------|
| (none)         | —                                   | `IRT_VOID`                |
| bits           | `u32`                               | `IRT_INT`, `IRT_FLOAT`    |
| element        | [`IrTypeId`](#irtypeid)             | `IRT_PTR`, `IRT_ARRAY`    |
| struct_        | [`IrTypeStruct`](#irtypestruct)     | `IRT_STRUCT`              |
| fn             | [`IrTypeFn`](#irtypefn)             | `IRT_FN`                  |

### `IrTypeTable`

```mach
pub rec IrTypeTable {
    alloc:   *Allocator;
    types:   *IrType;
    len:     u32;
    cap:     u32;
    dedup:   map.Map[IrType, IrTypeId];
}
```

Per-module interner. Identity is structural — calling
[`intern_*`](#functions) with the same shape always returns the
same [`IrTypeId`](#irtypeid).

`dedup` is keyed by the [`IrType`](#irtype) record itself, with
structural `hash` / `eq` callbacks: the callbacks dereference the
`*IrTypeId` arrays inside [`IrTypeStruct`](#irtypestruct) /
[`IrTypeFn`](#irtypefn) and compare element-wise, so two distinct
shapes can never collide onto one [`IrTypeId`](#irtypeid). This
mirrors the [`hash_type`](../../type.md#internal-helpers) /
[`eq_type`](../../type.md#internal-helpers) pattern in
[`lang.type`](../../type.md) — a raw hash would be unsafe as a key.

## Constants

```mach
pub val IRT_VOID:   IrTypeKind = 0;
pub val IRT_INT:    IrTypeKind = 1;
pub val IRT_FLOAT:  IrTypeKind = 2;
pub val IRT_PTR:    IrTypeKind = 3;
pub val IRT_ARRAY:  IrTypeKind = 4;
pub val IRT_STRUCT: IrTypeKind = 5;
pub val IRT_FN:     IrTypeKind = 6;
```

| Constant      | Meaning                                              |
|---------------|------------------------------------------------------|
| `IRT_VOID`    | Empty type (used for void-returning functions).       |
| `IRT_INT`     | Signed-or-unsigned integer of `bits` width. Signedness lives on the consuming instruction, not the type. |
| `IRT_FLOAT`   | IEEE float of `bits` width (32 / 64).                |
| `IRT_PTR`     | Untyped pointer to `element`. Pointers do not encode mutability; aliasing is the lowerer's concern. |
| `IRT_ARRAY`   | Fixed-length array (length encoded externally on the [`Instruction`](instruction.md) when needed). |
| `IRT_STRUCT`  | Aggregate with positional fields.                    |
| `IRT_FN`      | Function signature.                                  |

## Functions

### `intern_void`, `intern_int`, `intern_float`, `intern_ptr`, `intern_array`, `intern_struct`, `intern_fn`

```mach
pub fun intern_int(t: *IrTypeTable, bits: u32) Result[IrTypeId, str]
pub fun intern_float(t: *IrTypeTable, bits: u32) Result[IrTypeId, str]
pub fun intern_ptr(t: *IrTypeTable, element: IrTypeId) Result[IrTypeId, str]
# ... etc.
```

Each interner returns the existing [`IrTypeId`](#irtypeid) for a
matching shape or appends a new entry.

### `get`

```mach
pub fun get(t: *IrTypeTable, id: IrTypeId) Option[*IrType]
```

Lookup by id; returns the entry or `none` for an out-of-range id.

### `size_of`, `align_of`

```mach
pub fun size_of(t: *IrTypeTable, id: IrTypeId, tgt: *target.Target) u32
pub fun align_of(t: *IrTypeTable, id: IrTypeId, tgt: *target.Target) u32
```

Target-aware size / alignment in bytes. Pointer width comes from
[`target.arch.pointer_width`](../../target/isa.md#isavtable). For
structs, alignment is the max of the field alignments and size is
laid out with padding for natural alignment.

## Dependencies

`std.types.bool`, `std.types.size`, `std.types.result`,
`std.allocator`, `std.collections.map`, `std.crypto.hash.fnv1a`,
[`mach.lang.target`](../../target.md).
