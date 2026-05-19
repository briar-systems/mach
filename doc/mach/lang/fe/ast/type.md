# mach.lang.fe.ast.type

Syntactic type AST nodes — the parser's representation of types as
written in source. Each [`Type`](#type) is stored by value in
[`Ast.types`](../ast.md#ast). Distinct from the semantic
[`mach.lang.type.Type`](../../type.md#type), which is the *interned*
type sema produces from these syntactic nodes.

## Types

### `TypeKind`

```mach
pub def TypeKind: u8;
```

Discriminator for [`Type.data`](#type). See [Constants](#constants) for
the enumerated values.

### `TypeNamed`

```mach
pub rec TypeNamed {
    name:       token.Span;
    args_start: u32;
    args_len:   u32;
}
```

A named type reference, optionally with generic arguments.

| Field      | Type                                  | Description                                                |
|------------|---------------------------------------|------------------------------------------------------------|
| name       | [`token.Span`](../token.md#span)      | Span of the identifier or dotted path as written in source.|
| args_start | `u32`                                 | Start index into [`Ast.type_ids`](../ast.md#ast). |
| args_len   | `u32`                                 | Number of generic arguments (0 for a non-generic name).    |

### `TypePtr`

```mach
pub rec TypePtr {
    pointee: id.TypeId;
}
```

A pointer type `*T`.

| Field   | Type                          | Description                  |
|---------|-------------------------------|------------------------------|
| pointee | [`id.TypeId`](id.md#typeid)   | Element type being pointed to.|

### `TypeArray`

```mach
pub rec TypeArray {
    length:  id.ExprId;
    element: id.TypeId;
}
```

A fixed-length array type `[N]T`.

| Field   | Type                          | Description                                |
|---------|-------------------------------|--------------------------------------------|
| length  | [`id.ExprId`](id.md#exprid)   | Expression that evaluates to the array length.|
| element | [`id.TypeId`](id.md#typeid)   | Element type.                              |

### `TypeFun`

```mach
pub rec TypeFun {
    params_start: u32;
    params_len:   u32;
    ret:          id.TypeId;
}
```

A function type `fun(T, U) R`.

| Field        | Type                          | Description                                                |
|--------------|-------------------------------|------------------------------------------------------------|
| params_start | `u32`                         | Start index into [`Ast.type_ids`](../ast.md#ast). |
| params_len   | `u32`                         | Number of parameter types.                                 |
| ret          | [`id.TypeId`](id.md#typeid)   | Return type, or [`TYPE_NIL`](id.md#constants) for no explicit return. |

### `TypeRec`

```mach
pub rec TypeRec {
    fields_start: u32;
    fields_len:   u32;
}
```

An anonymous record type `rec { name: T; ... }`.

| Field        | Type    | Description                                                |
|--------------|---------|------------------------------------------------------------|
| fields_start | `u32`   | Start index into [`Ast.typed_names`](../ast.md#ast); each slot holds a [`TypedName`](decl.md#typedname). |
| fields_len   | `u32`   | Number of fields.                                          |

### `TypeUni`

```mach
pub rec TypeUni {
    fields_start: u32;
    fields_len:   u32;
}
```

An anonymous tagged-union type `uni { name: T; ... }`.

| Field        | Type    | Description                                                |
|--------------|---------|------------------------------------------------------------|
| fields_start | `u32`   | Start index into [`Ast.typed_names`](../ast.md#ast); each slot holds a [`TypedName`](decl.md#typedname). |
| fields_len   | `u32`   | Number of variants.                                        |

### `Type`

```mach
pub rec Type {
    span: token.Span;
    kind: TypeKind;
    data: uni {
        named: TypeNamed;
        ptr:   TypePtr;
        array: TypeArray;
        fun_:  TypeFun;
        rec_:  TypeRec;
        uni_:  TypeUni;
    };
}
```

A syntactic type reference as it appears in source. Payload is unused
for `ERROR` (discriminated by kind alone).

| Field | Type                                  | Description                                  |
|-------|---------------------------------------|----------------------------------------------|
| span  | [`token.Span`](../token.md#span)      | Byte range of the type expression.           |
| kind  | [`TypeKind`](#typekind)               | Which `TYPE_KIND_*` variant is active.       |
| data  | `uni { … }`                           | Kind-specific payload.                       |

## Constants

```mach
pub val TYPE_KIND_NAMED: TypeKind = 0;
pub val TYPE_KIND_PTR:   TypeKind = 1;
pub val TYPE_KIND_ARRAY: TypeKind = 2;
pub val TYPE_KIND_FUN:   TypeKind = 3;
pub val TYPE_KIND_REC:   TypeKind = 4;
pub val TYPE_KIND_UNI:   TypeKind = 5;
pub val TYPE_KIND_ERROR: TypeKind = 255;
```

[`TypeKind`](#typekind) values.

| Constant            | Value | Payload  | Notes                                          |
|---------------------|-------|----------|------------------------------------------------|
| `TYPE_KIND_NAMED`   | 0     | `named`  | A name reference (e.g. `i32`, `Map[K, V]`).    |
| `TYPE_KIND_PTR`     | 1     | `ptr`    | `*T`.                                          |
| `TYPE_KIND_ARRAY`   | 2     | `array`  | `[N]T`.                                        |
| `TYPE_KIND_FUN`     | 3     | `fun_`   | Function type `fun(T, U) R`.                   |
| `TYPE_KIND_REC`     | 4     | `rec_`   | Anonymous record `rec { ... }`.                |
| `TYPE_KIND_UNI`     | 5     | `uni_`   | Anonymous union `uni { ... }`.                 |
| `TYPE_KIND_ERROR`   | 255   | (none)   | Parser-produced poison.                        |

## Dependencies

`std.types.size`, [`mach.lang.fe.token`](../token.md),
[`mach.lang.fe.ast.id`](id.md).
