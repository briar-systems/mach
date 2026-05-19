# mach.lang.fe.ast.decl

Declaration AST nodes. Each [`Decl`](#decl) is stored by value in
[`Ast.decls`](../ast.md#ast); subnodes are referenced by
[`DeclId`](id.md#declid), [`StmtId`](id.md#stmtid),
[`ExprId`](id.md#exprid), and [`TypeId`](id.md#typeid) handles.

## Types

### `DeclKind`

```mach
pub def DeclKind: u8;
```

Discriminator for [`Decl.data`](#decl). See [Constants](#constants) for
the enumerated values.

### `TypedName`

```mach
pub rec TypedName {
    name: token.Span;
    ty:   id.TypeId;
}
```

A name paired with an explicit type; used for function parameters and
record fields.

| Field | Type                                  | Description                          |
|-------|---------------------------------------|--------------------------------------|
| name  | [`token.Span`](../token.md#span)      | Span of the identifier.              |
| ty    | [`id.TypeId`](id.md#typeid)           | Declared type.                       |

### `DeclUse`

```mach
pub rec DeclUse {
    alias: token.Span;
    path:  token.Span;
}
```

A `use` declaration importing a module, optionally under an alias.

| Field | Type                                  | Description                                                |
|-------|---------------------------------------|------------------------------------------------------------|
| alias | [`token.Span`](../token.md#span)      | Span of the alias identifier, or an empty span (len 0) if absent. |
| path  | [`token.Span`](../token.md#span)      | Span covering the dotted module path.                      |

### `DeclFun`

```mach
pub rec DeclFun {
    name:           token.Span;
    generics_start: u32;
    generics_len:   u32;
    params_start:   u32;
    params_len:     u32;
    return_type:    id.TypeId;
    body:           id.StmtId;
}
```

A function declaration.

| Field          | Type                                  | Description                                                |
|----------------|---------------------------------------|------------------------------------------------------------|
| name           | [`token.Span`](../token.md#span)      | Span of the function identifier.                           |
| generics_start | `u32`                                 | Start index into [`Ast.generic_names`](../ast.md#ast); each slot holds a [`Span`](../token.md#span). |
| generics_len   | `u32`                                 | Number of generic parameters.                              |
| params_start   | `u32`                                 | Start index into [`Ast.typed_names`](../ast.md#ast); each slot holds a [`TypedName`](#typedname). |
| params_len     | `u32`                                 | Number of parameters.                                      |
| return_type    | [`id.TypeId`](id.md#typeid)           | Declared return type, or [`TYPE_NIL`](id.md#constants) for no explicit return. |
| body           | [`id.StmtId`](id.md#stmtid)           | Body statement (typically `STMT_KIND_BLOCK`), or [`STMT_NIL`](id.md#constants) for forward-only declarations. |

### `DeclRec`

```mach
pub rec DeclRec {
    name:           token.Span;
    generics_start: u32;
    generics_len:   u32;
    fields_start:   u32;
    fields_len:     u32;
}
```

A record declaration.

| Field          | Type                                  | Description                                                |
|----------------|---------------------------------------|------------------------------------------------------------|
| name           | [`token.Span`](../token.md#span)      | Span of the record identifier.                             |
| generics_start | `u32`                                 | Start index into [`Ast.generic_names`](../ast.md#ast).     |
| generics_len   | `u32`                                 | Number of generic parameters.                              |
| fields_start   | `u32`                                 | Start index into [`Ast.typed_names`](../ast.md#ast); each slot holds a [`TypedName`](#typedname). |
| fields_len     | `u32`                                 | Number of fields.                                          |

### `DeclBind`

```mach
pub rec DeclBind {
    name: token.Span;
    ty:   id.TypeId;
    init: id.ExprId;
}
```

A binding declaration; used for both `DECL_KIND_VAL` and `DECL_KIND_VAR`
(kind discriminates mutability).

| Field | Type                                  | Description                                                |
|-------|---------------------------------------|------------------------------------------------------------|
| name  | [`token.Span`](../token.md#span)      | Span of the bound identifier.                              |
| ty    | [`id.TypeId`](id.md#typeid)           | Declared type, or [`TYPE_NIL`](id.md#constants) if inferred. |
| init  | [`id.ExprId`](id.md#exprid)           | Initializer expression, or [`EXPR_NIL`](id.md#constants) if absent. |

### `DeclDef`

```mach
pub rec DeclDef {
    name: token.Span;
    ty:   id.TypeId;
}
```

A type alias declaration `def Name: Type`.

| Field | Type                                  | Description                          |
|-------|---------------------------------------|--------------------------------------|
| name  | [`token.Span`](../token.md#span)      | Span of the alias identifier.        |
| ty    | [`id.TypeId`](id.md#typeid)           | Underlying type.                     |

### `DeclTest`

```mach
pub rec DeclTest {
    label: token.Span;
    body:  id.StmtId;
}
```

A `test "label" { body }` declaration.

| Field | Type                                  | Description                                          |
|-------|---------------------------------------|------------------------------------------------------|
| label | [`token.Span`](../token.md#span)      | Span of the string literal naming the test.          |
| body  | [`id.StmtId`](id.md#stmtid)           | Body statement (typically `STMT_KIND_BLOCK`).        |

### `ComptimeBranch`

```mach
pub rec ComptimeBranch {
    cond:       id.ExprId;
    body_start: u32;
    body_len:   u32;
}
```

One arm of a comptime `$if`/`$or` chain; shared between decl-level and
stmt-level comptime_if.

| Field      | Type                          | Description                                                |
|------------|-------------------------------|------------------------------------------------------------|
| cond       | [`id.ExprId`](id.md#exprid)   | Arm condition, or [`EXPR_NIL`](id.md#constants) for the final unconditional `$or { ... }`. |
| body_start | `u32`                         | Start index into [`Ast.decl_ids`](../ast.md#ast) (decl-level) or [`Ast.stmt_ids`](../ast.md#ast) (stmt-level), determined by the parent node's kind. |
| body_len   | `u32`                         | Number of entries in the arm body.                         |

### `DeclComptimeIf`

```mach
pub rec DeclComptimeIf {
    branches_start: u32;
    branches_len:   u32;
}
```

A `$if (cond) { decls } $or (cond) { decls } $or { decls }` chain at
declaration scope.

| Field          | Type    | Description                                                |
|----------------|---------|------------------------------------------------------------|
| branches_start | `u32`   | Start index into [`Ast.comptime_branches`](../ast.md#ast). |
| branches_len   | `u32`   | Number of arms (at least 1).                               |

### `DeclComptimeAttr`

```mach
pub rec DeclComptimeAttr {
    target: id.ExprId;
    value:  id.ExprId;
}
```

A compile-time attribute directive `$target = value`.

| Field  | Type                          | Description                                                |
|--------|-------------------------------|------------------------------------------------------------|
| target | [`id.ExprId`](id.md#exprid)   | Expression naming the attribute being set (typically a comptime-ident member chain such as `$main.symbol`). |
| value  | [`id.ExprId`](id.md#exprid)   | Expression assigned to the attribute.                      |

### `Decl`

```mach
pub rec Decl {
    span:  token.Span;
    kind:  DeclKind;
    flags: u8;
    data: uni {
        use_:          DeclUse;
        fun_:          DeclFun;
        rec_:          DeclRec;
        bind:          DeclBind;
        def_:          DeclDef;
        test_:         DeclTest;
        comptime_if:   DeclComptimeIf;
        comptime_attr: DeclComptimeAttr;
    };
}
```

A syntactic declaration node. Payload is unused for `ERROR`
(discriminated by kind alone).

| Field | Type                                  | Description                                  |
|-------|---------------------------------------|----------------------------------------------|
| span  | [`token.Span`](../token.md#span)      | Byte range of the declaration in source.     |
| kind  | [`DeclKind`](#declkind)               | Which `DECL_KIND_*` variant is active.       |
| flags | `u8`                                  | Bitfield of `DECL_FLAG_*` values.            |
| data  | `uni { … }`                           | Kind-specific payload.                       |

## Constants

```mach
pub val DECL_KIND_USE:            DeclKind = 0;
pub val DECL_KIND_FUN:            DeclKind = 1;
pub val DECL_KIND_REC:            DeclKind = 2;
pub val DECL_KIND_VAL:            DeclKind = 3;
pub val DECL_KIND_VAR:            DeclKind = 4;
pub val DECL_KIND_DEF:            DeclKind = 5;
pub val DECL_KIND_TEST:           DeclKind = 6;
pub val DECL_KIND_COMPTIME_IF:    DeclKind = 7;
pub val DECL_KIND_COMPTIME_ATTR:  DeclKind = 8;
pub val DECL_KIND_ERROR:          DeclKind = 255;
```

[`DeclKind`](#declkind) values.

| Constant                  | Value | Payload         |
|---------------------------|-------|-----------------|
| `DECL_KIND_USE`           | 0     | `use_`          |
| `DECL_KIND_FUN`           | 1     | `fun_`          |
| `DECL_KIND_REC`           | 2     | `rec_`          |
| `DECL_KIND_VAL`           | 3     | `bind`          |
| `DECL_KIND_VAR`           | 4     | `bind`          |
| `DECL_KIND_DEF`           | 5     | `def_`          |
| `DECL_KIND_TEST`          | 6     | `test_`         |
| `DECL_KIND_COMPTIME_IF`   | 7     | `comptime_if`   |
| `DECL_KIND_COMPTIME_ATTR` | 8     | `comptime_attr` |
| `DECL_KIND_ERROR`         | 255   | (none)          |

```mach
pub val DECL_FLAG_PUB: u8 = 1;
pub val DECL_FLAG_EXT: u8 = 2;
pub val DECL_FLAG_FWD: u8 = 4;
```

Bit flags stored on [`Decl.flags`](#decl).

| Constant         | Bit | Meaning                                   |
|------------------|-----|-------------------------------------------|
| `DECL_FLAG_PUB`  | 0   | `pub` visibility.                         |
| `DECL_FLAG_EXT`  | 1   | `ext` external linkage.                   |
| `DECL_FLAG_FWD`  | 2   | Forward declaration (no body).            |

## Dependencies

`std.types.size`, [`mach.lang.fe.token`](../token.md),
[`mach.lang.fe.ast.id`](id.md).
