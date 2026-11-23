[mach](../../README.md) / [front](../README.md) / [ast](README.md) / **type**

# mach.front.ast.type

the type-syntax nodes of the syntax tree: types as they are written, before they resolve

Source: [`src/front/ast/type.mach`](../../../../src/front/ast/type.mach)

Uses types from [`mach.base.source`](../../base/source.md), [`mach.front.ast.id`](id.md).

| Type | | |
| :--- | :--- | :--- |
| [`TypeKind`](#typekind) | `def` |  |
| [`TypeNamed`](#typenamed) | `rec` | a named type, with its generic arguments |
| [`TypePtr`](#typeptr) | `rec` | a pointer type `*T` |
| [`TypeSecret`](#typesecret) | `rec` | a secret type `^T` |
| [`TypeArray`](#typearray) | `rec` | an array type `[N]T` |
| [`TypeFun`](#typefun) | `rec` | a function type |
| [`TypeRec`](#typerec) | `rec` | an anonymous `rec { ... }` type |
| [`TypeUni`](#typeuni) | `rec` | an anonymous `uni { ... }` type |
| [`TypeFieldType`](#typefieldtype) | `rec` | the type of a comptime field descriptor, written `f.type` |
| [`TypePointeeOf`](#typepointeeof) | `rec` | `$pointee_of(T)`, the type a typed reference refers to |
| [`TypeDiscriminantOf`](#typediscriminantof) | `rec` | `$discriminant_of(T)`, a tag's discriminator type |
| [`Type`](#type) | `rec` | one type-syntax node |

| On `TypeKind` | |
| :--- | :--- |
| [`TYPE_KIND_NAMED`](#typekind), [`TYPE_KIND_PTR`](#typekind), [`TYPE_KIND_ARRAY`](#typekind), [`TYPE_KIND_FUN`](#typekind), [`TYPE_KIND_REC`](#typekind), [`TYPE_KIND_UNI`](#typekind), [`TYPE_KIND_PACK`](#typekind), [`TYPE_KIND_SECRET`](#typekind), [`TYPE_KIND_FIELD_TYPE`](#typekind), [`TYPE_KIND_POINTEE_OF`](#typekind), [`TYPE_KIND_DISCRIMINANT_OF`](#typekind), [`TYPE_KIND_ERROR`](#typekind) | |

---

## TypeKind

```mach
pub def TypeKind: u8
```

| Name | Value | |
| :--- | :--- | :--- |
| `TYPE_KIND_NAMED` | `0` |  |
| `TYPE_KIND_PTR` | `1` |  |
| `TYPE_KIND_ARRAY` | `2` |  |
| `TYPE_KIND_FUN` | `3` |  |
| `TYPE_KIND_REC` | `4` |  |
| `TYPE_KIND_UNI` | `5` |  |
| `TYPE_KIND_PACK` | `6` |  |
| `TYPE_KIND_SECRET` | `7` |  |
| `TYPE_KIND_FIELD_TYPE` | `8` |  |
| `TYPE_KIND_POINTEE_OF` | `9` |  |
| `TYPE_KIND_DISCRIMINANT_OF` | `10` |  |
| `TYPE_KIND_ERROR` | `255` |  |

## TypeNamed

```mach
pub rec TypeNamed {
    name:       mach.base.source.Span;
    args_start: u32;
    args_len:   u32;
    operand:    bool;
    # in a literal head after generic arguments, `T[A].c{...}`, the tag case written; length zero
    # otherwise. a bare `T.c` keeps its whole path in `name`, and name resolution's head
    # interpretation reads it as type and case
    case_name: mach.base.source.Span;
    # in a literal head `T.[c]{...}`, the comptime case descriptor; EXPR_NIL otherwise
    case_desc: mach.front.ast.id.ExprId;
    # where the tree's path names hold the id of each segment of `name`, in order
    segments: u32;
}
```

a named type, with its generic arguments

| Field | Type | |
| :--- | :--- | :--- |
| `name` | `mach.base.source.Span` | the span of the dotted path |
| `args_start` | `u32` | first generic argument in the type id table |
| `args_len` | `u32` | how many generic arguments |
| `operand` | `bool` | whether the type stands as a type operand of a call, such as the argument of `$size_of` |

## TypePtr

```mach
pub rec TypePtr {
    pointee: mach.front.ast.id.TypeId;
}
```

a pointer type `*T`

| Field | Type | |
| :--- | :--- | :--- |
| `pointee` | `mach.front.ast.id.TypeId` | the pointed-to type syntax |

## TypeSecret

```mach
pub rec TypeSecret {
    inner: mach.front.ast.id.TypeId;
}
```

a secret type `^T`

| Field | Type | |
| :--- | :--- | :--- |
| `inner` | `mach.front.ast.id.TypeId` | the qualified type syntax |

## TypeArray

```mach
pub rec TypeArray {
    length:  mach.front.ast.id.ExprId;
    element: mach.front.ast.id.TypeId;
}
```

an array type `[N]T`

| Field | Type | |
| :--- | :--- | :--- |
| `length` | `mach.front.ast.id.ExprId` | the length expression, EXPR_NIL for `[_]` |
| `element` | `mach.front.ast.id.TypeId` | the element type syntax |

## TypeFun

```mach
pub rec TypeFun {
    params_start: u32;
    params_len:   u32;
    ret_type:     mach.front.ast.id.TypeId;
    c_variadic:   bool;
}
```

a function type

| Field | Type | |
| :--- | :--- | :--- |
| `params_start` | `u32` | first parameter in the typed name table |
| `params_len` | `u32` | how many parameters |
| `ret_type` | `mach.front.ast.id.TypeId` | the return type syntax, TYPE_NIL when none is written |
| `c_variadic` | `bool` | whether the parameter list ends in a C `...` |

## TypeRec

```mach
pub rec TypeRec {
    fields_start: u32;
    fields_len:   u32;
}
```

an anonymous `rec { ... }` type

| Field | Type | |
| :--- | :--- | :--- |
| `fields_start` | `u32` | first field in the typed name table |
| `fields_len` | `u32` | how many fields |

## TypeUni

```mach
pub rec TypeUni {
    fields_start: u32;
    fields_len:   u32;
}
```

an anonymous `uni { ... }` type

| Field | Type | |
| :--- | :--- | :--- |
| `fields_start` | `u32` | first field in the typed name table |
| `fields_len` | `u32` | how many fields |

## TypeFieldType

```mach
pub rec TypeFieldType {
    var_name: mach.base.source.Span;
}
```

the type of a comptime field descriptor, written `f.type`

| Field | Type | |
| :--- | :--- | :--- |
| `var_name` | `mach.base.source.Span` | the span of the descriptor variable |

## TypePointeeOf

```mach
pub rec TypePointeeOf {
    inner: mach.front.ast.id.TypeId;
}
```

`$pointee_of(T)`, the type a typed reference refers to

| Field | Type | |
| :--- | :--- | :--- |
| `inner` | `mach.front.ast.id.TypeId` | the reference type syntax |

## TypeDiscriminantOf

```mach
pub rec TypeDiscriminantOf {
    inner: mach.front.ast.id.TypeId;
}
```

`$discriminant_of(T)`, a tag's discriminator type

| Field | Type | |
| :--- | :--- | :--- |
| `inner` | `mach.front.ast.id.TypeId` | the tag type syntax |

## Type

```mach
pub rec Type {
    span: mach.base.source.Span;
    kind: TypeKind;
    data: uni {
        named: TypeNamed;
        ptr: TypePtr;
        secret: TypeSecret;
        array: TypeArray;
        fun_: TypeFun;
        rec_: TypeRec;
        uni_: TypeUni;
        field_type: TypeFieldType;
        pointee_of: TypePointeeOf;
        discriminant_of: TypeDiscriminantOf;
    };
}
```

one type-syntax node

| Field | Type | |
| :--- | :--- | :--- |
| `span` | `mach.base.source.Span` | the whole type as written |
| `kind` | `TypeKind` | which TYPE_KIND_ the node is, which names the active `data` field |
| `data` | `uni { named: TypeNamed; ptr: TypePtr; secret: TypeSecret; array: TypeArray; fun_: TypeFun; rec_: TypeRec; uni_: TypeUni; field_type: TypeFieldType; pointee_of: TypePointeeOf; discriminant_of: TypeDiscriminantOf; }` | the payload of the node's kind |

