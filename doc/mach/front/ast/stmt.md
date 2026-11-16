[mach](../../README.md) / [front](../README.md) / [ast](README.md) / **stmt**

# mach.front.ast.stmt

the statement nodes of the syntax tree

Source: [`src/front/ast/stmt.mach`](../../../../src/front/ast/stmt.mach)

Uses types from [`mach.base.source`](../../base/source.md), [`mach.front.ast.id`](id.md).

| Type | | |
| :--- | :--- | :--- |
| [`StmtKind`](#stmtkind) | `def` |  |
| [`StmtExpr`](#stmtexpr) | `rec` |  |
| [`StmtBlock`](#stmtblock) | `rec` |  |
| [`StmtIf`](#stmtif) | `rec` |  |
| [`StmtFor`](#stmtfor) | `rec` |  |
| [`StmtRet`](#stmtret) | `rec` |  |
| [`StmtFin`](#stmtfin) | `rec` |  |
| [`StmtDecl`](#stmtdecl) | `rec` |  |
| [`StmtAsm`](#stmtasm) | `rec` | head: the text between `asm` and the body's `{`, masc's selection handed over unchanged |
| [`StmtComptimeIf`](#stmtcomptimeif) | `rec` |  |
| [`StmtComptimeEach`](#stmtcomptimeeach) | `rec` |  |
| [`StmtComptimeDirective`](#stmtcomptimedirective) | `rec` |  |
| [`Stmt`](#stmt) | `rec` |  |

| On `StmtKind` | |
| :--- | :--- |
| [`STMT_KIND_EXPR`](#stmtkind), [`STMT_KIND_BLOCK`](#stmtkind), [`STMT_KIND_IF`](#stmtkind), [`STMT_KIND_FOR`](#stmtkind), [`STMT_KIND_RET`](#stmtkind), [`STMT_KIND_BRK`](#stmtkind), [`STMT_KIND_CNT`](#stmtkind), [`STMT_KIND_FIN`](#stmtkind), [`STMT_KIND_DECL`](#stmtkind), [`STMT_KIND_ASM`](#stmtkind), [`STMT_KIND_COMPTIME_IF`](#stmtkind), [`STMT_KIND_COMPTIME_EACH`](#stmtkind), [`STMT_KIND_COMPTIME_DIRECTIVE`](#stmtkind), [`STMT_KIND_ERROR`](#stmtkind) | |

---

## StmtKind

```mach
pub def StmtKind: u8
```

| Name | Value | |
| :--- | :--- | :--- |
| `STMT_KIND_EXPR` | `0` |  |
| `STMT_KIND_BLOCK` | `1` |  |
| `STMT_KIND_IF` | `2` |  |
| `STMT_KIND_FOR` | `3` |  |
| `STMT_KIND_RET` | `4` |  |
| `STMT_KIND_BRK` | `5` |  |
| `STMT_KIND_CNT` | `6` |  |
| `STMT_KIND_FIN` | `7` |  |
| `STMT_KIND_DECL` | `8` |  |
| `STMT_KIND_ASM` | `9` |  |
| `STMT_KIND_COMPTIME_IF` | `10` |  |
| `STMT_KIND_COMPTIME_EACH` | `11` |  |
| `STMT_KIND_COMPTIME_DIRECTIVE` | `12` |  |
| `STMT_KIND_ERROR` | `255` |  |

## StmtExpr

```mach
pub rec StmtExpr {
    expr: mach.front.ast.id.ExprId;
}
```

## StmtBlock

```mach
pub rec StmtBlock {
    stmts_start: u32;
    stmts_len:   u32;
}
```

## StmtIf

```mach
pub rec StmtIf {
    cond:       mach.front.ast.id.ExprId;
    then_block: mach.front.ast.id.StmtId;
    else_block: mach.front.ast.id.StmtId;
}
```

## StmtFor

```mach
pub rec StmtFor {
    cond: mach.front.ast.id.ExprId;
    body: mach.front.ast.id.StmtId;
}
```

## StmtRet

```mach
pub rec StmtRet {
    value: mach.front.ast.id.ExprId;
}
```

## StmtFin

```mach
pub rec StmtFin {
    body: mach.front.ast.id.StmtId;
}
```

## StmtDecl

```mach
pub rec StmtDecl {
    decl: mach.front.ast.id.DeclId;
}
```

## StmtAsm

```mach
pub rec StmtAsm {
    head: mach.base.source.Span;
    body: mach.base.source.Span;
}
```

head: the text between `asm` and the body's `{`, masc's selection handed over unchanged

## StmtComptimeIf

```mach
pub rec StmtComptimeIf {
    branches_start: u32;
    branches_len:   u32;
}
```

## StmtComptimeEach

```mach
pub rec StmtComptimeEach {
    var_name:   mach.base.source.Span;
    seq:        mach.front.ast.id.ExprId;
    body_start: u32;
    body_len:   u32;
}
```

## StmtComptimeDirective

```mach
pub rec StmtComptimeDirective {
    target: mach.front.ast.id.ExprId;
}
```

## Stmt

```mach
pub rec Stmt {
    span: mach.base.source.Span;
    kind: StmtKind;
    data: uni {
        expr: StmtExpr;
        block: StmtBlock;
        if_: StmtIf;
        for_: StmtFor;
        ret_: StmtRet;
        fin_: StmtFin;
        decl: StmtDecl;
        asm_: StmtAsm;
        comptime_if: StmtComptimeIf;
        comptime_each: StmtComptimeEach;
        comptime_directive: StmtComptimeDirective;
    };
}
```

