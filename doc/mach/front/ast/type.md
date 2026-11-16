[mach](../../README.md) / [front](../README.md) / [ast](README.md) / **type**

# mach.front.ast.type

the type-syntax nodes of the syntax tree: types as they are written, before they resolve

<sub>source: [`src/front/ast/type.mach`](../../../../src/front/ast/type.mach) · imported by 24 modules</sub>

<sub>`base_source` is [`mach.base.source`](../../base/source.md) · `ast_id` is [`mach.front.ast.id`](id.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`TypeKind`](#typekind) | `def` |  |
| [`TypeNamed`](#typenamed) | `rec` |  |
| [`TypePtr`](#typeptr) | `rec` |  |
| [`TypeSecret`](#typesecret) | `rec` |  |
| [`TypeArray`](#typearray) | `rec` |  |
| [`TypeFun`](#typefun) | `rec` |  |
| [`TypeRec`](#typerec) | `rec` |  |
| [`TypeUni`](#typeuni) | `rec` |  |
| [`TypeFieldType`](#typefieldtype) | `rec` |  |
| [`TypePointeeOf`](#typepointeeof) | `rec` |  |
| [`TypeDiscriminantOf`](#typediscriminantof) | `rec` |  |
| [`Type`](#type) | `rec` |  |

| On `TypeKind` | |
| :--- | :--- |
| [`TYPE_KIND_NAMED`](#typekind) · [`TYPE_KIND_PTR`](#typekind) · [`TYPE_KIND_ARRAY`](#typekind) · [`TYPE_KIND_FUN`](#typekind) · [`TYPE_KIND_REC`](#typekind) · [`TYPE_KIND_UNI`](#typekind) · [`TYPE_KIND_PACK`](#typekind) · [`TYPE_KIND_SECRET`](#typekind) · [`TYPE_KIND_FIELD_TYPE`](#typekind) · [`TYPE_KIND_POINTEE_OF`](#typekind) · [`TYPE_KIND_DISCRIMINANT_OF`](#typekind) · [`TYPE_KIND_ERROR`](#typekind) | |

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
    name:       base_source.Span;
    args_start: u32;
    args_len:   u32;
    operand:    bool;
    # in a literal head after generic arguments, `T[A].c{...}`, the tag case written; length zero
    # otherwise. a bare `T.c` keeps its whole path in `name`, and name resolution's head
    # interpretation reads it as type and case
    case_name: base_source.Span;
    # in a literal head `T.[c]{...}`, the comptime case descriptor; EXPR_NIL otherwise
    case_desc: ast_id.ExprId;
    # where the tree's path names hold the id of each segment of `name`, in order
    segments: u32;
}
```

## TypePtr

```mach
pub rec TypePtr {
    pointee: ast_id.TypeId;
}
```

## TypeSecret

```mach
pub rec TypeSecret {
    inner: ast_id.TypeId;
}
```

## TypeArray

```mach
pub rec TypeArray {
    length:  ast_id.ExprId;
    element: ast_id.TypeId;
}
```

## TypeFun

```mach
pub rec TypeFun {
    params_start: u32;
    params_len:   u32;
    ret_type:     ast_id.TypeId;
    c_variadic:   bool;
}
```

## TypeRec

```mach
pub rec TypeRec {
    fields_start: u32;
    fields_len:   u32;
}
```

## TypeUni

```mach
pub rec TypeUni {
    fields_start: u32;
    fields_len:   u32;
}
```

## TypeFieldType

```mach
pub rec TypeFieldType {
    var_name: base_source.Span;
}
```

## TypePointeeOf

```mach
pub rec TypePointeeOf {
    inner: ast_id.TypeId;
}
```

## TypeDiscriminantOf

```mach
pub rec TypeDiscriminantOf {
    inner: ast_id.TypeId;
}
```

## Type

```mach
pub rec Type {
    span: base_source.Span;
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

