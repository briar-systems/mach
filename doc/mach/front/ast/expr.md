[mach](../../README.md) / [front](../README.md) / [ast](README.md) / **expr**

# mach.front.ast.expr

the expression nodes of the syntax tree and the operators they carry

<sub>source: [`src/front/ast/expr.mach`](../../../../src/front/ast/expr.mach) · imported by 39 modules</sub>

<sub>`ast_id` is [`mach.front.ast.id`](id.md) · `base_source` is [`mach.base.source`](../../base/source.md) · `intern` is [`mach.base.intern`](../../base/intern.md) · `ExprIdent` is private · `ExprLitInt` is private · `ExprLitFloat` is private · `ExprLitChar` is private · `ExprLitStr` is private · `ExprLitNil` is private · `type_kind` is [`mach.type.kind`](../../type/kind.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`ExprKind`](#exprkind) | `def` |  |
| [`BinOp`](#binop) | `def` |  |
| [`UnOp`](#unop) | `def` |  |
| [`ByteRange`](#byterange) | `rec` | a string literal: `tok` covers the quotes a run of bytes in the tree's literal byte table |
| [`ExprBinary`](#exprbinary) | `rec` |  |
| [`ExprUnary`](#exprunary) | `rec` |  |
| [`ExprCall`](#exprcall) | `rec` |  |
| [`ExprIndex`](#exprindex) | `rec` | `x[i]` reads one element; `x[i, n]` is a range of `n` elements from `i`, and `count` is EXPR_NIL for the single element |
| [`ExprMember`](#exprmember) | `rec` | `name_id` is `name` as the parse interned it, STR_NIL for a member with no name |
| [`ExprProject`](#exprproject) | `rec` |  |
| [`ExprSpread`](#exprspread) | `rec` |  |
| [`ExprCast`](#exprcast) | `rec` |  |
| [`ExprStrip`](#exprstrip) | `rec` |  |
| [`ExprArrayLit`](#exprarraylit) | `rec` |  |
| [`FieldInit`](#fieldinit) | `rec` | `name_id` is `name` as the parse interned it, STR_NIL for a positional element |
| [`ExprStructLit`](#exprstructlit) | `rec` |  |
| [`ExprVectorLit`](#exprvectorlit) | `rec` | a typed literal with only positional elements, `T{a, b}`: a vector's lanes as written, or a tag case's payload when name resolution finds the head names a case (interpretation.CASE_LITERAL) |
| [`ExprTypeRef`](#exprtyperef) | `rec` |  |
| [`ExprSel`](#exprsel) | `rec` |  |
| [`Expr`](#expr) | `rec` | `span` is the node's visual extent, grouping parentheses included |

| On `ExprKind` | |
| :--- | :--- |
| [`EXPR_KIND_IDENT`](#exprkind) · [`EXPR_KIND_LIT_INT`](#exprkind) · [`EXPR_KIND_LIT_FLOAT`](#exprkind) · [`EXPR_KIND_LIT_CHAR`](#exprkind) · [`EXPR_KIND_LIT_STR`](#exprkind) · [`EXPR_KIND_LIT_NIL`](#exprkind) · [`EXPR_KIND_BINARY`](#exprkind) · [`EXPR_KIND_UNARY`](#exprkind) · [`EXPR_KIND_CALL`](#exprkind) · [`EXPR_KIND_INDEX`](#exprkind) · [`EXPR_KIND_MEMBER`](#exprkind) · [`EXPR_KIND_CAST`](#exprkind) · [`EXPR_KIND_ARRAY_LIT`](#exprkind) · [`EXPR_KIND_STRUCT_LIT`](#exprkind) · [`EXPR_KIND_COMPTIME_IDENT`](#exprkind) · [`EXPR_KIND_PROJECT`](#exprkind) · [`EXPR_KIND_SPREAD`](#exprkind) · [`EXPR_KIND_STRIP`](#exprkind) · [`EXPR_KIND_VECTOR_LIT`](#exprkind) · [`EXPR_KIND_TYPE_REF`](#exprkind) · [`EXPR_KIND_GENERIC_INSTANCE`](#exprkind) · [`EXPR_KIND_SEL`](#exprkind) · [`EXPR_KIND_ERROR`](#exprkind) | |

| On `BinOp` | |
| :--- | :--- |
| [`BIN_ADD`](#binop) · [`BIN_SUB`](#binop) · [`BIN_MUL`](#binop) · [`BIN_DIV`](#binop) · [`BIN_MOD`](#binop) · [`BIN_EQ`](#binop) · [`BIN_NEQ`](#binop) · [`BIN_LT`](#binop) · [`BIN_LEQ`](#binop) · [`BIN_GT`](#binop) · [`BIN_GEQ`](#binop) · [`BIN_AND`](#binop) · [`BIN_OR`](#binop) · [`BIN_BIT_AND`](#binop) · [`BIN_BIT_OR`](#binop) · [`BIN_BIT_XOR`](#binop) · [`BIN_SHL`](#binop) · [`BIN_SHR`](#binop) · [`BIN_ASSIGN`](#binop) | |

| On `UnOp` | |
| :--- | :--- |
| [`UN_NEG`](#unop) · [`UN_NOT`](#unop) · [`UN_BIT_NOT`](#unop) · [`UN_ADDR`](#unop) · [`UN_DEREF`](#unop) | |

| On `ExprIndex` | |
| :--- | :--- |
| [`is_range`](#is_range) | |

| Constants | |
| :--- | :--- |
| [`LIT_SUFFIX_NONE`](#constants) · [`INIT_NAMED`](#constants) · [`INIT_BARE`](#constants) · [`INIT_POSITIONAL`](#constants) | |

---

## ExprKind

```mach
pub def ExprKind: u8
```

| Name | Value | |
| :--- | :--- | :--- |
| `EXPR_KIND_IDENT` | `0` |  |
| `EXPR_KIND_LIT_INT` | `1` |  |
| `EXPR_KIND_LIT_FLOAT` | `2` |  |
| `EXPR_KIND_LIT_CHAR` | `3` |  |
| `EXPR_KIND_LIT_STR` | `4` |  |
| `EXPR_KIND_LIT_NIL` | `6` |  |
| `EXPR_KIND_BINARY` | `7` |  |
| `EXPR_KIND_UNARY` | `8` |  |
| `EXPR_KIND_CALL` | `9` |  |
| `EXPR_KIND_INDEX` | `10` |  |
| `EXPR_KIND_MEMBER` | `11` |  |
| `EXPR_KIND_CAST` | `12` |  |
| `EXPR_KIND_ARRAY_LIT` | `13` |  |
| `EXPR_KIND_STRUCT_LIT` | `14` |  |
| `EXPR_KIND_COMPTIME_IDENT` | `15` |  |
| `EXPR_KIND_PROJECT` | `16` |  |
| `EXPR_KIND_SPREAD` | `17` |  |
| `EXPR_KIND_STRIP` | `18` |  |
| `EXPR_KIND_VECTOR_LIT` | `19` |  |
| `EXPR_KIND_TYPE_REF` | `20` |  |
| `EXPR_KIND_GENERIC_INSTANCE` | `21` |  |
| `EXPR_KIND_SEL` | `23` |  |
| `EXPR_KIND_ERROR` | `255` |  |

## BinOp

```mach
pub def BinOp: u8
```

| Name | Value | |
| :--- | :--- | :--- |
| `BIN_ADD` | `0` |  |
| `BIN_SUB` | `1` |  |
| `BIN_MUL` | `2` |  |
| `BIN_DIV` | `3` |  |
| `BIN_MOD` | `4` |  |
| `BIN_EQ` | `5` |  |
| `BIN_NEQ` | `6` |  |
| `BIN_LT` | `7` |  |
| `BIN_LEQ` | `8` |  |
| `BIN_GT` | `9` |  |
| `BIN_GEQ` | `10` |  |
| `BIN_AND` | `11` |  |
| `BIN_OR` | `12` |  |
| `BIN_BIT_AND` | `13` |  |
| `BIN_BIT_OR` | `14` |  |
| `BIN_BIT_XOR` | `15` |  |
| `BIN_SHL` | `16` |  |
| `BIN_SHR` | `17` |  |
| `BIN_ASSIGN` | `18` |  |

## UnOp

```mach
pub def UnOp: u8
```

| Name | Value | |
| :--- | :--- | :--- |
| `UN_NEG` | `0` |  |
| `UN_NOT` | `1` |  |
| `UN_BIT_NOT` | `2` |  |
| `UN_ADDR` | `3` |  |
| `UN_DEREF` | `4` |  |

## ByteRange

```mach
pub rec ByteRange {
    start: u32;
    len:   u32;
}
```

a string literal: `tok` covers the quotes
a run of bytes in the tree's literal byte table

## ExprBinary

```mach
pub rec ExprBinary {
    op:  BinOp;
    lhs: ast_id.ExprId;
    rhs: ast_id.ExprId;
}
```

## ExprUnary

```mach
pub rec ExprUnary {
    op:      UnOp;
    operand: ast_id.ExprId;
}
```

## ExprCall

```mach
pub rec ExprCall {
    callee:          ast_id.ExprId;
    args_start:      u32;
    args_len:        u32;
    type_args_start: u32;
    type_args_len:   u32;
    index_callee:    ast_id.ExprId;
}
```

## ExprIndex

```mach
pub rec ExprIndex {
    object: ast_id.ExprId;
    index:  ast_id.ExprId;
    count:  ast_id.ExprId;
}
```

`x[i]` reads one element; `x[i, n]` is a range of `n` elements from `i`, and
`count` is EXPR_NIL for the single element

### is_range

```mach
pub fun is_range(ix: *ExprIndex) bool
```

## ExprMember

```mach
pub rec ExprMember {
    object:  ast_id.ExprId;
    name:    base_source.Span;
    name_id: intern.StrId;
}
```

`name_id` is `name` as the parse interned it, STR_NIL for a member with no name

## ExprProject

```mach
pub rec ExprProject {
    object: ast_id.ExprId;
    field:  ast_id.ExprId;
}
```

## ExprSpread

```mach
pub rec ExprSpread {
    inner: ast_id.ExprId;
}
```

## ExprCast

```mach
pub rec ExprCast {
    value:       ast_id.ExprId;
    ty:          ast_id.TypeId;
    reinterpret: bool;
}
```

## ExprStrip

```mach
pub rec ExprStrip {
    value: ast_id.ExprId;
    ty:    ast_id.TypeId;
}
```

## ExprArrayLit

```mach
pub rec ExprArrayLit {
    ty:          ast_id.TypeId;
    elems_start: u32;
    elems_len:   u32;
}
```

## FieldInit

```mach
pub rec FieldInit {
    form:    u8;
    name:    base_source.Span;
    name_id: intern.StrId;
    value:   ast_id.ExprId;
}
```

`name_id` is `name` as the parse interned it, STR_NIL for a positional element

## ExprStructLit

```mach
pub rec ExprStructLit {
    elems_start:  u32;
    elems_len:    u32;
    ty:           ast_id.TypeId;
    fields_start: u32;
    fields_len:   u32;
}
```

## ExprVectorLit

```mach
pub rec ExprVectorLit {
    ty:          ast_id.TypeId;
    elems_start: u32;
    elems_len:   u32;
}
```

a typed literal with only positional elements, `T{a, b}`: a vector's lanes as written, or a tag
case's payload when name resolution finds the head names a case (interpretation.CASE_LITERAL)

## ExprTypeRef

```mach
pub rec ExprTypeRef {
    ty: ast_id.TypeId;
}
```

## ExprSel

```mach
pub rec ExprSel {
    place:     ast_id.ExprId;
    case_name: base_source.Span;
    # `case_name` as the parse interned it, STR_NIL for an unnamed case
    case_id: intern.StrId;
    # `sel place.[c]`: the comptime case descriptor; EXPR_NIL when the case is named
    case_desc: ast_id.ExprId;
}
```

## Expr

```mach
pub rec Expr {
    span: base_source.Span;
    kind: ExprKind;
    data: uni {
        ident: ExprIdent;
        comptime_ident: ExprIdent;
        lit_int: ExprLitInt;
        lit_float: ExprLitFloat;
        lit_char: ExprLitChar;
        lit_str: ExprLitStr;
        lit_nil: ExprLitNil;
        binary: ExprBinary;
        unary: ExprUnary;
        call: ExprCall;
        index: ExprIndex;
        member: ExprMember;
        cast: ExprCast;
        strip: ExprStrip;
        array_lit: ExprArrayLit;
        struct_lit: ExprStructLit;
        vector_lit: ExprVectorLit;
        project: ExprProject;
        spread: ExprSpread;
        type_ref: ExprTypeRef;
        sel: ExprSel;
    };
}
```

`span` is the node's visual extent, grouping parentheses included. a token-backed node (a name
or a literal) keeps its token in its payload's `tok`, and its text is read through `tok`, never
through `span`

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `LIT_SUFFIX_NONE` | `type_kind.TypeKind` | `0xFF` |  |
| `INIT_NAMED` | `u8` | `0` |  |
| `INIT_BARE` | `u8` | `1` |  |
| `INIT_POSITIONAL` | `u8` | `2` |  |

