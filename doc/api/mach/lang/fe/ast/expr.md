# mach.lang.fe.ast.expr

## def ExprKind

```mach
pub def ExprKind: u8
```

## val EXPR_KIND_IDENT

```mach
pub val EXPR_KIND_IDENT:            ExprKind = 0
```

## val EXPR_KIND_LIT_INT

```mach
pub val EXPR_KIND_LIT_INT:          ExprKind = 1
```

## val EXPR_KIND_LIT_FLOAT

```mach
pub val EXPR_KIND_LIT_FLOAT:        ExprKind = 2
```

## val EXPR_KIND_LIT_CHAR

```mach
pub val EXPR_KIND_LIT_CHAR:         ExprKind = 3
```

## val EXPR_KIND_LIT_STR

```mach
pub val EXPR_KIND_LIT_STR:          ExprKind = 4
```

## val EXPR_KIND_LIT_NIL

```mach
pub val EXPR_KIND_LIT_NIL:          ExprKind = 6
```

## val EXPR_KIND_BINARY

```mach
pub val EXPR_KIND_BINARY:           ExprKind = 7
```

## val EXPR_KIND_UNARY

```mach
pub val EXPR_KIND_UNARY:            ExprKind = 8
```

## val EXPR_KIND_CALL

```mach
pub val EXPR_KIND_CALL:             ExprKind = 9
```

## val EXPR_KIND_INDEX

```mach
pub val EXPR_KIND_INDEX:            ExprKind = 10
```

## val EXPR_KIND_MEMBER

```mach
pub val EXPR_KIND_MEMBER:           ExprKind = 11
```

## val EXPR_KIND_CAST

```mach
pub val EXPR_KIND_CAST:             ExprKind = 12
```

## val EXPR_KIND_ARRAY_LIT

```mach
pub val EXPR_KIND_ARRAY_LIT:        ExprKind = 13
```

## val EXPR_KIND_STRUCT_LIT

```mach
pub val EXPR_KIND_STRUCT_LIT:       ExprKind = 14
```

## val EXPR_KIND_COMPTIME_IDENT

```mach
pub val EXPR_KIND_COMPTIME_IDENT:   ExprKind = 15
```

## val EXPR_KIND_PROJECT

```mach
pub val EXPR_KIND_PROJECT:          ExprKind = 16
```

## val EXPR_KIND_SPREAD

```mach
pub val EXPR_KIND_SPREAD:           ExprKind = 17
```

## val EXPR_KIND_STRIP

```mach
pub val EXPR_KIND_STRIP:            ExprKind = 18
```

## val EXPR_KIND_VECTOR_LIT

```mach
pub val EXPR_KIND_VECTOR_LIT:       ExprKind = 19
```

## val EXPR_KIND_TYPE_REF

```mach
pub val EXPR_KIND_TYPE_REF:         ExprKind = 20
```

## val EXPR_KIND_GENERIC_INSTANCE

```mach
pub val EXPR_KIND_GENERIC_INSTANCE: ExprKind = 21
```

## val EXPR_KIND_SEL

```mach
pub val EXPR_KIND_SEL:              ExprKind = 23
```

## val EXPR_KIND_ERROR

```mach
pub val EXPR_KIND_ERROR:            ExprKind = 255
```

## def BinOp

```mach
pub def BinOp: u8
```

## val BIN_ADD

```mach
pub val BIN_ADD:     BinOp = 0
```

## val BIN_SUB

```mach
pub val BIN_SUB:     BinOp = 1
```

## val BIN_MUL

```mach
pub val BIN_MUL:     BinOp = 2
```

## val BIN_DIV

```mach
pub val BIN_DIV:     BinOp = 3
```

## val BIN_MOD

```mach
pub val BIN_MOD:     BinOp = 4
```

## val BIN_EQ

```mach
pub val BIN_EQ:      BinOp = 5
```

## val BIN_NEQ

```mach
pub val BIN_NEQ:     BinOp = 6
```

## val BIN_LT

```mach
pub val BIN_LT:      BinOp = 7
```

## val BIN_LEQ

```mach
pub val BIN_LEQ:     BinOp = 8
```

## val BIN_GT

```mach
pub val BIN_GT:      BinOp = 9
```

## val BIN_GEQ

```mach
pub val BIN_GEQ:     BinOp = 10
```

## val BIN_AND

```mach
pub val BIN_AND:     BinOp = 11
```

## val BIN_OR

```mach
pub val BIN_OR:      BinOp = 12
```

## val BIN_BIT_AND

```mach
pub val BIN_BIT_AND: BinOp = 13
```

## val BIN_BIT_OR

```mach
pub val BIN_BIT_OR:  BinOp = 14
```

## val BIN_BIT_XOR

```mach
pub val BIN_BIT_XOR: BinOp = 15
```

## val BIN_SHL

```mach
pub val BIN_SHL:     BinOp = 16
```

## val BIN_SHR

```mach
pub val BIN_SHR:     BinOp = 17
```

## val BIN_ASSIGN

```mach
pub val BIN_ASSIGN:  BinOp = 18
```

## def UnOp

```mach
pub def UnOp: u8
```

## val UN_NEG

```mach
pub val UN_NEG:     UnOp = 0
```

## val UN_NOT

```mach
pub val UN_NOT:     UnOp = 1
```

## val UN_BIT_NOT

```mach
pub val UN_BIT_NOT: UnOp = 2
```

## val UN_ADDR

```mach
pub val UN_ADDR:    UnOp = 3
```

## val UN_DEREF

```mach
pub val UN_DEREF:   UnOp = 4
```

## val LIT_SUFFIX_NONE

```mach
pub val LIT_SUFFIX_NONE: type.TypeKind = 0xFF
```

## rec ExprLitInt

```mach
pub rec ExprLitInt;
```

## rec ExprLitFloat

```mach
pub rec ExprLitFloat;
```

## rec ExprBinary

```mach
pub rec ExprBinary;
```

## rec ExprUnary

```mach
pub rec ExprUnary;
```

## rec ExprCall

```mach
pub rec ExprCall;
```

## rec ExprIndex

```mach
pub rec ExprIndex;
```

## rec ExprMember

```mach
pub rec ExprMember;
```

## rec ExprProject

```mach
pub rec ExprProject;
```

## rec ExprSpread

```mach
pub rec ExprSpread;
```

## rec ExprCast

```mach
pub rec ExprCast;
```

## rec ExprStrip

```mach
pub rec ExprStrip;
```

## rec ExprArrayLit

```mach
pub rec ExprArrayLit;
```

## val INIT_NAMED

```mach
pub val INIT_NAMED:      u8 = 0
```

## val INIT_BARE

```mach
pub val INIT_BARE:       u8 = 1
```

## val INIT_POSITIONAL

```mach
pub val INIT_POSITIONAL: u8 = 2
```

## rec FieldInit

```mach
pub rec FieldInit;
```

## rec ExprStructLit

```mach
pub rec ExprStructLit;
```

## rec ExprVectorLit

```mach
pub rec ExprVectorLit;
```

## rec ExprTypeRef

```mach
pub rec ExprTypeRef;
```

## rec ExprSel

```mach
pub rec ExprSel;
```

## rec Expr

```mach
pub rec Expr;
```

