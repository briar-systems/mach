[mach](../../README.md) / [front](../README.md) / [ast](README.md) / **stmt**

# mach.front.ast.stmt

the statement nodes of the syntax tree

Source: [`src/front/ast/stmt.mach`](../../../../src/front/ast/stmt.mach)

Uses types from [`mach.base.source`](../../base/source.md), [`mach.front.ast.id`](id.md).

| Type | | |
| :--- | :--- | :--- |
| [`StmtKind`](#stmtkind) | `def` |  |
| [`StmtExpr`](#stmtexpr) | `rec` | an expression statement |
| [`StmtBlock`](#stmtblock) | `rec` | a braced block |
| [`StmtIf`](#stmtif) | `rec` | an `if` or an `or (cond)` arm |
| [`StmtFor`](#stmtfor) | `rec` | a `for` loop |
| [`StmtRet`](#stmtret) | `rec` | a `ret` statement |
| [`StmtFin`](#stmtfin) | `rec` | a `fin` cleanup |
| [`StmtDecl`](#stmtdecl) | `rec` | a local `val` or `var` |
| [`StmtAsm`](#stmtasm) | `rec` | head: the text between `asm` and the body's `{`, masc's selection handed over unchanged |
| [`StmtComptimeIf`](#stmtcomptimeif) | `rec` | a `$if` chain inside a function body |
| [`StmtComptimeEach`](#stmtcomptimeeach) | `rec` | a `$each` loop |
| [`StmtComptimeDirective`](#stmtcomptimedirective) | `rec` | a comptime directive statement such as `$error(...)` |
| [`Stmt`](#stmt) | `rec` | one statement node |

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

an expression statement

| Field | Type | |
| :--- | :--- | :--- |
| `expr` | `mach.front.ast.id.ExprId` | the expression |

## StmtBlock

```mach
pub rec StmtBlock {
    stmts_start: u32;
    stmts_len:   u32;
}
```

a braced block

| Field | Type | |
| :--- | :--- | :--- |
| `stmts_start` | `u32` | first statement in the statement id table |
| `stmts_len` | `u32` | how many statements |

## StmtIf

```mach
pub rec StmtIf {
    cond:       mach.front.ast.id.ExprId;
    then_block: mach.front.ast.id.StmtId;
    else_block: mach.front.ast.id.StmtId;
}
```

an `if` or an `or (cond)` arm

| Field | Type | |
| :--- | :--- | :--- |
| `cond` | `mach.front.ast.id.ExprId` | the condition |
| `then_block` | `mach.front.ast.id.StmtId` | the arm's block, STMT_NIL when the parse found none |
| `else_block` | `mach.front.ast.id.StmtId` | the next `or (cond)` arm as an if statement, a block for a bare `or`, STMT_NIL when the chain ends |

## StmtFor

```mach
pub rec StmtFor {
    cond: mach.front.ast.id.ExprId;
    body: mach.front.ast.id.StmtId;
}
```

a `for` loop

| Field | Type | |
| :--- | :--- | :--- |
| `cond` | `mach.front.ast.id.ExprId` | the condition, EXPR_NIL for a bare `for` |
| `body` | `mach.front.ast.id.StmtId` | the body block |

## StmtRet

```mach
pub rec StmtRet {
    value: mach.front.ast.id.ExprId;
}
```

a `ret` statement

| Field | Type | |
| :--- | :--- | :--- |
| `value` | `mach.front.ast.id.ExprId` | the returned value, EXPR_NIL for a bare `ret` |

## StmtFin

```mach
pub rec StmtFin {
    body: mach.front.ast.id.StmtId;
}
```

a `fin` cleanup

| Field | Type | |
| :--- | :--- | :--- |
| `body` | `mach.front.ast.id.StmtId` | the cleanup block |

## StmtDecl

```mach
pub rec StmtDecl {
    decl: mach.front.ast.id.DeclId;
}
```

a local `val` or `var`

| Field | Type | |
| :--- | :--- | :--- |
| `decl` | `mach.front.ast.id.DeclId` | the declaration node of the binding |

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

a `$if` chain inside a function body

| Field | Type | |
| :--- | :--- | :--- |
| `branches_start` | `u32` | first arm in the comptime branch table |
| `branches_len` | `u32` | how many arms |

## StmtComptimeEach

```mach
pub rec StmtComptimeEach {
    var_name:   mach.base.source.Span;
    seq:        mach.front.ast.id.ExprId;
    body_start: u32;
    body_len:   u32;
}
```

a `$each` loop

| Field | Type | |
| :--- | :--- | :--- |
| `var_name` | `mach.base.source.Span` | the span of the loop variable |
| `seq` | `mach.front.ast.id.ExprId` | the sequence expression it walks |
| `body_start` | `u32` | first body statement in the statement id table |
| `body_len` | `u32` | how many body statements |

## StmtComptimeDirective

```mach
pub rec StmtComptimeDirective {
    target: mach.front.ast.id.ExprId;
}
```

a comptime directive statement such as `$error(...)`

| Field | Type | |
| :--- | :--- | :--- |
| `target` | `mach.front.ast.id.ExprId` | the directive expression |

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

one statement node

| Field | Type | |
| :--- | :--- | :--- |
| `span` | `mach.base.source.Span` | the whole statement |
| `kind` | `StmtKind` | which STMT_KIND_ the statement is, which names the active `data` field |
| `data` | `uni { expr: StmtExpr; block: StmtBlock; if_: StmtIf; for_: StmtFor; ret_: StmtRet; fin_: StmtFin; decl: StmtDecl; asm_: StmtAsm; comptime_if: StmtComptimeIf; comptime_each: StmtComptimeEach; comptime_directive: StmtComptimeDirective; }` | the payload of the statement's kind |

