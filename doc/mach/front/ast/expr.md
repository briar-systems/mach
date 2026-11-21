[mach](../../README.md) / [front](../README.md) / [ast](README.md) / **expr**

# mach.front.ast.expr

the expression nodes of the syntax tree and the operators they carry

Source: [`src/front/ast/expr.mach`](../../../../src/front/ast/expr.mach)

Uses types from [`mach.base.intern`](../../base/intern.md), [`mach.base.source`](../../base/source.md), [`mach.front.ast.id`](id.md), [`mach.type.kind`](../../type/kind.md).

| Type | | |
| :--- | :--- | :--- |
| [`ExprKind`](#exprkind) | `def` |  |
| [`BinOp`](#binop) | `def` |  |
| [`UnOp`](#unop) | `def` |  |
| [`ByteRange`](#byterange) | `rec` | a run of bytes in the tree's literal byte table |
| [`ExprBinary`](#exprbinary) | `rec` | a binary operation, assignment included |
| [`ExprUnary`](#exprunary) | `rec` | a unary operation |
| [`ExprCall`](#exprcall) | `rec` | a call, or a generic instance `f[T]` with no argument list the parser keeps the reading of the bracket as an index beside its reading as generic arguments, and name resolution picks one (interpretation.get) |
| [`ExprIndex`](#exprindex) | `rec` | `x[i]` reads one element; `x[i, n]` is a range of `n` elements from `i`, and `count` is EXPR_NIL for the single element |
| [`ExprMember`](#exprmember) | `rec` | `name_id` is `name` as the parse interned it, STR_NIL for a member with no name |
| [`ExprProject`](#exprproject) | `rec` | a comptime field projection `x.[f]` |
| [`ExprSpread`](#exprspread) | `rec` | a pack spread `va...` |
| [`ExprCast`](#exprcast) | `rec` | a cast `x::T` or `x:~T` |
| [`ExprStrip`](#exprstrip) | `rec` | a secret downgrade `x:>T` |
| [`ExprArrayLit`](#exprarraylit) | `rec` | an array literal `[N]T{a, b}` |
| [`FieldInit`](#fieldinit) | `rec` | `name_id` is `name` as the parse interned it, STR_NIL for a positional element |
| [`ExprStructLit`](#exprstructlit) | `rec` | a typed literal `T{...}` with named or bare fields, or no fields |
| [`ExprVectorLit`](#exprvectorlit) | `rec` | a typed literal with only positional elements, `T{a, b}`: a vector's lanes as written, or a tag case's payload when name resolution finds the head names a case (interpretation.CASE_LITERAL) |
| [`ExprTypeRef`](#exprtyperef) | `rec` | a type written where an expression stands |
| [`ExprSel`](#exprsel) | `rec` | a `sel place.case` test of whether a tag place holds the named case |
| [`Expr`](#expr) | `rec` | `span` is the node's visual extent, grouping parentheses included |

| On `ExprKind` | |
| :--- | :--- |
| [`EXPR_KIND_IDENT`](#exprkind), [`EXPR_KIND_LIT_INT`](#exprkind), [`EXPR_KIND_LIT_FLOAT`](#exprkind), [`EXPR_KIND_LIT_CHAR`](#exprkind), [`EXPR_KIND_LIT_STR`](#exprkind), [`EXPR_KIND_LIT_NIL`](#exprkind), [`EXPR_KIND_BINARY`](#exprkind), [`EXPR_KIND_UNARY`](#exprkind), [`EXPR_KIND_CALL`](#exprkind), [`EXPR_KIND_INDEX`](#exprkind), [`EXPR_KIND_MEMBER`](#exprkind), [`EXPR_KIND_CAST`](#exprkind), [`EXPR_KIND_ARRAY_LIT`](#exprkind), [`EXPR_KIND_STRUCT_LIT`](#exprkind), [`EXPR_KIND_COMPTIME_IDENT`](#exprkind), [`EXPR_KIND_PROJECT`](#exprkind), [`EXPR_KIND_SPREAD`](#exprkind), [`EXPR_KIND_STRIP`](#exprkind), [`EXPR_KIND_VECTOR_LIT`](#exprkind), [`EXPR_KIND_TYPE_REF`](#exprkind), [`EXPR_KIND_GENERIC_INSTANCE`](#exprkind), [`EXPR_KIND_SEL`](#exprkind), [`EXPR_KIND_ERROR`](#exprkind) | |

| On `BinOp` | |
| :--- | :--- |
| [`BIN_ADD`](#binop), [`BIN_SUB`](#binop), [`BIN_MUL`](#binop), [`BIN_DIV`](#binop), [`BIN_MOD`](#binop), [`BIN_EQ`](#binop), [`BIN_NEQ`](#binop), [`BIN_LT`](#binop), [`BIN_LEQ`](#binop), [`BIN_GT`](#binop), [`BIN_GEQ`](#binop), [`BIN_AND`](#binop), [`BIN_OR`](#binop), [`BIN_BIT_AND`](#binop), [`BIN_BIT_OR`](#binop), [`BIN_BIT_XOR`](#binop), [`BIN_SHL`](#binop), [`BIN_SHR`](#binop), [`BIN_ASSIGN`](#binop) | |

| On `UnOp` | |
| :--- | :--- |
| [`UN_NEG`](#unop), [`UN_NOT`](#unop), [`UN_BIT_NOT`](#unop), [`UN_ADDR`](#unop), [`UN_DEREF`](#unop) | |

| On `ExprIndex` | |
| :--- | :--- |
| [`is_range`](#is_range) | whether an index expression is a range `x[i, n]` |

| Constants | |
| :--- | :--- |
| [`LIT_SUFFIX_NONE`](#constants), [`INIT_NAMED`](#constants), [`INIT_BARE`](#constants), [`INIT_POSITIONAL`](#constants) | |

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

a run of bytes in the tree's literal byte table

## ExprBinary

```mach
pub rec ExprBinary {
    op:  BinOp;
    lhs: mach.front.ast.id.ExprId;
    rhs: mach.front.ast.id.ExprId;
}
```

a binary operation, assignment included

| Field | Type | |
| :--- | :--- | :--- |
| `op` | `BinOp` | the BIN_ operator |
| `lhs` | `mach.front.ast.id.ExprId` | the left operand |
| `rhs` | `mach.front.ast.id.ExprId` | the right operand |

## ExprUnary

```mach
pub rec ExprUnary {
    op:      UnOp;
    operand: mach.front.ast.id.ExprId;
}
```

a unary operation

| Field | Type | |
| :--- | :--- | :--- |
| `op` | `UnOp` | the UN_ operator |
| `operand` | `mach.front.ast.id.ExprId` | the operand |

## ExprCall

```mach
pub rec ExprCall {
    callee:          mach.front.ast.id.ExprId;
    args_start:      u32;
    args_len:        u32;
    type_args_start: u32;
    type_args_len:   u32;
    index_callee:    mach.front.ast.id.ExprId;
}
```

a call, or a generic instance `f[T]` with no argument list
the parser keeps the reading of the bracket as an index beside its reading as generic
arguments, and name resolution picks one (interpretation.get)

| Field | Type | |
| :--- | :--- | :--- |
| `callee` | `mach.front.ast.id.ExprId` | the called expression |
| `args_start` | `u32` | first argument in the expression id table |
| `args_len` | `u32` | how many arguments |
| `type_args_start` | `u32` | first generic argument in the type id table |
| `type_args_len` | `u32` | how many generic arguments |
| `index_callee` | `mach.front.ast.id.ExprId` | the bracket read as an index expression, EXPR_NIL when there is no bracket or it cannot be read as one |

## ExprIndex

```mach
pub rec ExprIndex {
    object: mach.front.ast.id.ExprId;
    index:  mach.front.ast.id.ExprId;
    count:  mach.front.ast.id.ExprId;
}
```

`x[i]` reads one element; `x[i, n]` is a range of `n` elements from `i`, and
`count` is EXPR_NIL for the single element

### is_range

```mach
pub fun is_range(ix: *ExprIndex) bool
```

whether an index expression is a range `x[i, n]`

| Parameter | |
| :--- | :--- |
| `ix` | the index expression |
| **returns** | true when it carries a count |

## ExprMember

```mach
pub rec ExprMember {
    object:  mach.front.ast.id.ExprId;
    name:    mach.base.source.Span;
    name_id: mach.base.intern.StrId;
}
```

`name_id` is `name` as the parse interned it, STR_NIL for a member with no name

## ExprProject

```mach
pub rec ExprProject {
    object: mach.front.ast.id.ExprId;
    field:  mach.front.ast.id.ExprId;
}
```

a comptime field projection `x.[f]`

| Field | Type | |
| :--- | :--- | :--- |
| `object` | `mach.front.ast.id.ExprId` | the projected value |
| `field` | `mach.front.ast.id.ExprId` | the comptime field descriptor |

## ExprSpread

```mach
pub rec ExprSpread {
    inner: mach.front.ast.id.ExprId;
}
```

a pack spread `va...`

| Field | Type | |
| :--- | :--- | :--- |
| `inner` | `mach.front.ast.id.ExprId` | the spread pack |

## ExprCast

```mach
pub rec ExprCast {
    value:       mach.front.ast.id.ExprId;
    ty:          mach.front.ast.id.TypeId;
    reinterpret: bool;
}
```

a cast `x::T` or `x:~T`

| Field | Type | |
| :--- | :--- | :--- |
| `value` | `mach.front.ast.id.ExprId` | the cast operand |
| `ty` | `mach.front.ast.id.TypeId` | the target type syntax |
| `reinterpret` | `bool` | true for a `:~` bit reinterpretation, false for a `::` value conversion |

## ExprStrip

```mach
pub rec ExprStrip {
    value: mach.front.ast.id.ExprId;
    ty:    mach.front.ast.id.TypeId;
}
```

a secret downgrade `x:>T`

| Field | Type | |
| :--- | :--- | :--- |
| `value` | `mach.front.ast.id.ExprId` | the operand |
| `ty` | `mach.front.ast.id.TypeId` | the public target type syntax |

## ExprArrayLit

```mach
pub rec ExprArrayLit {
    ty:          mach.front.ast.id.TypeId;
    elems_start: u32;
    elems_len:   u32;
}
```

an array literal `[N]T{a, b}`

| Field | Type | |
| :--- | :--- | :--- |
| `ty` | `mach.front.ast.id.TypeId` | the array type syntax |
| `elems_start` | `u32` | first element in the expression id table |
| `elems_len` | `u32` | how many elements |

## FieldInit

```mach
pub rec FieldInit {
    form:    u8;
    name:    mach.base.source.Span;
    name_id: mach.base.intern.StrId;
    value:   mach.front.ast.id.ExprId;
}
```

`name_id` is `name` as the parse interned it, STR_NIL for a positional element

## ExprStructLit

```mach
pub rec ExprStructLit {
    elems_start:  u32;
    elems_len:    u32;
    ty:           mach.front.ast.id.TypeId;
    fields_start: u32;
    fields_len:   u32;
}
```

a typed literal `T{...}` with named or bare fields, or no fields

| Field | Type | |
| :--- | :--- | :--- |
| `elems_start` | `u32` | first element value in the expression id table, one per field init in order |
| `elems_len` | `u32` | how many element values |
| `ty` | `mach.front.ast.id.TypeId` | the head type syntax |
| `fields_start` | `u32` | first field init in the field init table |
| `fields_len` | `u32` | how many field inits |

## ExprVectorLit

```mach
pub rec ExprVectorLit {
    ty:          mach.front.ast.id.TypeId;
    elems_start: u32;
    elems_len:   u32;
}
```

a typed literal with only positional elements, `T{a, b}`: a vector's lanes as written, or a tag
case's payload when name resolution finds the head names a case (interpretation.CASE_LITERAL)

## ExprTypeRef

```mach
pub rec ExprTypeRef {
    ty: mach.front.ast.id.TypeId;
}
```

a type written where an expression stands

| Field | Type | |
| :--- | :--- | :--- |
| `ty` | `mach.front.ast.id.TypeId` | the type syntax |

## ExprSel

```mach
pub rec ExprSel {
    place:     mach.front.ast.id.ExprId;
    case_name: mach.base.source.Span;
    case_id:   mach.base.intern.StrId;
    case_desc: mach.front.ast.id.ExprId;
}
```

a `sel place.case` test of whether a tag place holds the named case

| Field | Type | |
| :--- | :--- | :--- |
| `place` | `mach.front.ast.id.ExprId` | the tag place tested |
| `case_name` | `mach.base.source.Span` | the case as written, an empty span at the place for `sel place.[c]` |
| `case_id` | `mach.base.intern.StrId` | `case_name` as the parse interned it, STR_NIL for an unnamed case |
| `case_desc` | `mach.front.ast.id.ExprId` | `sel place.[c]`: the comptime case descriptor, EXPR_NIL when the case is named |

## Expr

```mach
pub rec Expr {
    span: mach.base.source.Span;
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
| `LIT_SUFFIX_NONE` | `mach.type.kind.TypeKind` | `0xFF` |  |
| `INIT_NAMED` | `u8` | `0` |  |
| `INIT_BARE` | `u8` | `1` |  |
| `INIT_POSITIONAL` | `u8` | `2` |  |

