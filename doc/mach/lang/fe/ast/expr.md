# mach.lang.fe.ast.expr

Expression AST nodes. Each [`Expr`](#expr) is stored by value in
[`Ast.exprs`](../ast.md#ast); subnodes are referenced by
[`ExprId`](id.md#exprid) handles, never by pointer.

## Types

### `ExprKind`

```mach
pub def ExprKind: u8;
```

Discriminator for [`Expr.data`](#expr). See [Constants](#constants) for
the enumerated values.

### `BinOp`

```mach
pub def BinOp: u8;
```

Binary operator code carried in [`ExprBinary.op`](#exprbinary). See
[Constants](#constants) for the enumerated values.

### `UnOp`

```mach
pub def UnOp: u8;
```

Unary operator code carried in [`ExprUnary.op`](#exprunary). See
[Constants](#constants) for the enumerated values.

### `ExprBinary`

```mach
pub rec ExprBinary {
    op:  BinOp;
    lhs: id.ExprId;
    rhs: id.ExprId;
}
```

A binary operation applied to two subexpressions.

| Field | Type                          | Description           |
|-------|-------------------------------|-----------------------|
| op    | [`BinOp`](#binop)             | Operator identity.    |
| lhs   | [`id.ExprId`](id.md#exprid)   | Left-hand operand.    |
| rhs   | [`id.ExprId`](id.md#exprid)   | Right-hand operand.   |

### `ExprUnary`

```mach
pub rec ExprUnary {
    op:      UnOp;
    operand: id.ExprId;
}
```

A unary operation applied to one subexpression.

| Field   | Type                          | Description           |
|---------|-------------------------------|-----------------------|
| op      | [`UnOp`](#unop)               | Operator identity.    |
| operand | [`id.ExprId`](id.md#exprid)   | The subexpression.    |

### `ExprCall`

```mach
pub rec ExprCall {
    callee:     id.ExprId;
    args_start: u32;
    args_len:   u32;
}
```

A function or method call expression.

| Field      | Type                          | Description                                                |
|------------|-------------------------------|------------------------------------------------------------|
| callee     | [`id.ExprId`](id.md#exprid)   | The expression being invoked.                              |
| args_start | `u32`                         | Start index into [`Ast.expr_ids`](../ast.md#ast). |
| args_len   | `u32`                         | Number of argument expressions.                            |

### `ExprIndex`

```mach
pub rec ExprIndex {
    object: id.ExprId;
    index:  id.ExprId;
}
```

An index expression `obj[idx]`.

| Field  | Type                          | Description                  |
|--------|-------------------------------|------------------------------|
| object | [`id.ExprId`](id.md#exprid)   | The collection being indexed.|
| index  | [`id.ExprId`](id.md#exprid)   | The index expression.        |

### `ExprMember`

```mach
pub rec ExprMember {
    object: id.ExprId;
    name:   token.Span;
}
```

A member access expression `obj.field`.

| Field  | Type                                  | Description                                  |
|--------|---------------------------------------|----------------------------------------------|
| object | [`id.ExprId`](id.md#exprid)           | The expression on the left of the dot.       |
| name   | [`token.Span`](../token.md#span)      | Span of the field identifier.                |

### `ExprCast`

```mach
pub rec ExprCast {
    value: id.ExprId;
    ty:    id.TypeId;
}
```

A type cast expression `value::Type`.

| Field | Type                          | Description           |
|-------|-------------------------------|-----------------------|
| value | [`id.ExprId`](id.md#exprid)   | Expression being cast.|
| ty    | [`id.TypeId`](id.md#typeid)   | Target type.          |

### `ExprArrayLit`

```mach
pub rec ExprArrayLit {
    ty:          id.TypeId;
    elems_start: u32;
    elems_len:   u32;
}
```

An array literal `[N]T{a, b, c}`.

| Field        | Type                          | Description                                                |
|--------------|-------------------------------|------------------------------------------------------------|
| ty           | [`id.TypeId`](id.md#typeid)   | Declared array type including length and element type.     |
| elems_start  | `u32`                         | Start index into [`Ast.expr_ids`](../ast.md#ast). |
| elems_len    | `u32`                         | Number of element expressions.                             |

### `FieldInit`

```mach
pub rec FieldInit {
    name:  token.Span;
    value: id.ExprId;
}
```

A field name paired with its initializer expression.

| Field | Type                                  | Description                          |
|-------|---------------------------------------|--------------------------------------|
| name  | [`token.Span`](../token.md#span)      | Span of the field identifier.        |
| value | [`id.ExprId`](id.md#exprid)           | Expression assigned to the field.    |

### `ExprStructLit`

```mach
pub rec ExprStructLit {
    ty:           id.TypeId;
    fields_start: u32;
    fields_len:   u32;
}
```

A struct literal `Name{field: value, ...}`.

| Field        | Type                          | Description                                                |
|--------------|-------------------------------|------------------------------------------------------------|
| ty           | [`id.TypeId`](id.md#typeid)   | Named type of the struct being constructed.                |
| fields_start | `u32`                         | Start index into [`Ast.field_inits`](../ast.md#ast).       |
| fields_len   | `u32`                         | Number of field initializers.                              |

### `Expr`

```mach
pub rec Expr {
    span: token.Span;
    kind: ExprKind;
    data: uni {
        lit_int:    u64;
        lit_float:  f64;
        lit_char:   u8;
        binary:     ExprBinary;
        unary:      ExprUnary;
        call:       ExprCall;
        index:      ExprIndex;
        member:     ExprMember;
        cast:       ExprCast;
        array_lit:  ExprArrayLit;
        struct_lit: ExprStructLit;
    };
}
```

A syntactic expression node. Payload is unused for `IDENT`,
`COMPTIME_IDENT`, `LIT_STR`, `LIT_ZSTR`, `LIT_NIL`, and `ERROR`
(discriminated by kind alone).

| Field | Type                                  | Description                                  |
|-------|---------------------------------------|----------------------------------------------|
| span  | [`token.Span`](../token.md#span)      | Byte range of the expression in source.      |
| kind  | [`ExprKind`](#exprkind)               | Which `EXPR_KIND_*` variant is active.       |
| data  | `uni { ... }`                           | Kind-specific payload.                       |

## Constants

```mach
pub val EXPR_KIND_IDENT:          ExprKind = 0;
pub val EXPR_KIND_LIT_INT:        ExprKind = 1;
pub val EXPR_KIND_LIT_FLOAT:      ExprKind = 2;
pub val EXPR_KIND_LIT_CHAR:       ExprKind = 3;
pub val EXPR_KIND_LIT_STR:        ExprKind = 4;
pub val EXPR_KIND_LIT_ZSTR:       ExprKind = 5;
pub val EXPR_KIND_LIT_NIL:        ExprKind = 6;
pub val EXPR_KIND_BINARY:         ExprKind = 7;
pub val EXPR_KIND_UNARY:          ExprKind = 8;
pub val EXPR_KIND_CALL:           ExprKind = 9;
pub val EXPR_KIND_INDEX:          ExprKind = 10;
pub val EXPR_KIND_MEMBER:         ExprKind = 11;
pub val EXPR_KIND_CAST:           ExprKind = 12;
pub val EXPR_KIND_ARRAY_LIT:      ExprKind = 13;
pub val EXPR_KIND_STRUCT_LIT:     ExprKind = 14;
pub val EXPR_KIND_COMPTIME_IDENT: ExprKind = 15;
pub val EXPR_KIND_ERROR:          ExprKind = 255;
```

[`ExprKind`](#exprkind) values.

| Constant                  | Value | Notes                                         |
|---------------------------|-------|-----------------------------------------------|
| `EXPR_KIND_IDENT`         | 0     | Identifier reference. No payload.             |
| `EXPR_KIND_LIT_INT`       | 1     | Integer literal. Payload: `lit_int`.          |
| `EXPR_KIND_LIT_FLOAT`     | 2     | Float literal. Payload: `lit_float`.          |
| `EXPR_KIND_LIT_CHAR`      | 3     | Character literal. Payload: `lit_char`.       |
| `EXPR_KIND_LIT_STR`       | 4     | Double-quoted string literal. No payload.     |
| `EXPR_KIND_LIT_ZSTR`      | 5     | Backtick literal. No payload.                 |
| `EXPR_KIND_LIT_NIL`       | 6     | `nil` keyword. No payload.                    |
| `EXPR_KIND_BINARY`        | 7     | Binary op. Payload: `binary`.                 |
| `EXPR_KIND_UNARY`         | 8     | Unary op. Payload: `unary`.                   |
| `EXPR_KIND_CALL`          | 9     | Call expression. Payload: `call`.             |
| `EXPR_KIND_INDEX`         | 10    | Index expression. Payload: `index`.           |
| `EXPR_KIND_MEMBER`        | 11    | Member access. Payload: `member`.             |
| `EXPR_KIND_CAST`          | 12    | Cast expression. Payload: `cast`.             |
| `EXPR_KIND_ARRAY_LIT`     | 13    | Array literal. Payload: `array_lit`.          |
| `EXPR_KIND_STRUCT_LIT`    | 14    | Record literal. Payload: `struct_lit`.        |
| `EXPR_KIND_COMPTIME_IDENT`| 15    | `$`-prefixed comptime identifier. No payload. |
| `EXPR_KIND_ERROR`         | 255   | Parser-produced poison. No payload.           |

```mach
pub val BIN_ADD:     BinOp = 0;
pub val BIN_SUB:     BinOp = 1;
pub val BIN_MUL:     BinOp = 2;
pub val BIN_DIV:     BinOp = 3;
pub val BIN_MOD:     BinOp = 4;
pub val BIN_EQ:      BinOp = 5;
pub val BIN_NEQ:     BinOp = 6;
pub val BIN_LT:      BinOp = 7;
pub val BIN_LEQ:     BinOp = 8;
pub val BIN_GT:      BinOp = 9;
pub val BIN_GEQ:     BinOp = 10;
pub val BIN_AND:     BinOp = 11;
pub val BIN_OR:      BinOp = 12;
pub val BIN_BIT_AND: BinOp = 13;
pub val BIN_BIT_OR:  BinOp = 14;
pub val BIN_BIT_XOR: BinOp = 15;
pub val BIN_SHL:     BinOp = 16;
pub val BIN_SHR:     BinOp = 17;
pub val BIN_ASSIGN:  BinOp = 18;
```

[`BinOp`](#binop) values.

| Constant        | Value | Surface                  |
|-----------------|-------|--------------------------|
| `BIN_ADD`       | 0     | `+`                      |
| `BIN_SUB`       | 1     | `-`                      |
| `BIN_MUL`       | 2     | `*`                      |
| `BIN_DIV`       | 3     | `/`                      |
| `BIN_MOD`       | 4     | `%`                      |
| `BIN_EQ`        | 5     | `==`                     |
| `BIN_NEQ`       | 6     | `!=`                     |
| `BIN_LT`        | 7     | `<`                      |
| `BIN_LEQ`       | 8     | `<=`                     |
| `BIN_GT`        | 9     | `>`                      |
| `BIN_GEQ`       | 10    | `>=`                     |
| `BIN_AND`       | 11    | `&&`                     |
| `BIN_OR`        | 12    | `\|\|`                   |
| `BIN_BIT_AND`   | 13    | `&`                      |
| `BIN_BIT_OR`    | 14    | `\|`                     |
| `BIN_BIT_XOR`   | 15    | `^`                      |
| `BIN_SHL`       | 16    | `<<`                     |
| `BIN_SHR`       | 17    | `>>`                     |
| `BIN_ASSIGN`    | 18    | `=`                      |

```mach
pub val UN_NEG:     UnOp = 0;
pub val UN_NOT:     UnOp = 1;
pub val UN_BIT_NOT: UnOp = 2;
pub val UN_ADDR:    UnOp = 3;
pub val UN_DEREF:   UnOp = 4;
```

[`UnOp`](#unop) values.

| Constant      | Value | Surface              |
|---------------|-------|----------------------|
| `UN_NEG`      | 0     | `-` (numeric negation)|
| `UN_NOT`      | 1     | `!` (logical NOT)    |
| `UN_BIT_NOT`  | 2     | `~` (bitwise NOT)    |
| `UN_ADDR`     | 3     | `?` (address-of)     |
| `UN_DEREF`    | 4     | `@` (dereference)    |

## Dependencies

`std.types.size`, [`mach.lang.fe.token`](../token.md),
[`mach.lang.fe.ast.id`](id.md).
