# mach.lang.fe.ast.stmt

Statement AST nodes. Each [`Stmt`](#stmt) is stored by value in
[`Ast.stmts`](../ast.md#ast); subnodes are referenced by
[`StmtId`](id.md#stmtid), [`ExprId`](id.md#exprid), or
[`DeclId`](id.md#declid) handles.

## Types

### `StmtKind`

```mach
pub def StmtKind: u8;
```

Discriminator for [`Stmt.data`](#stmt). See [Constants](#constants) for
the enumerated values.

### `StmtExpr`

```mach
pub rec StmtExpr {
    expr: id.ExprId;
}
```

A bare expression used as a statement.

| Field | Type                          | Description       |
|-------|-------------------------------|-------------------|
| expr  | [`id.ExprId`](id.md#exprid)   | The expression.   |

### `StmtBlock`

```mach
pub rec StmtBlock {
    stmts_start: u32;
    stmts_len:   u32;
}
```

A brace-delimited sequence of statements.

| Field        | Type    | Description                                                  |
|--------------|---------|--------------------------------------------------------------|
| stmts_start  | `u32`   | Start index into [`Ast.stmt_ids`](../ast.md#ast). |
| stmts_len    | `u32`   | Number of statements in the block.                           |

### `StmtIf`

```mach
pub rec StmtIf {
    cond:       id.ExprId;
    then_block: id.StmtId;
    else_block: id.StmtId;
}
```

An `if`/`or`-chain statement. `else_block` is
[`STMT_NIL`](id.md#constants) when there is no trailing `or` branch.

| Field      | Type                          | Description                                          |
|------------|-------------------------------|------------------------------------------------------|
| cond       | [`id.ExprId`](id.md#exprid)   | Condition expression.                                |
| then_block | [`id.StmtId`](id.md#stmtid)   | Body executed when `cond` is true.                   |
| else_block | [`id.StmtId`](id.md#stmtid)   | Body executed when `cond` is false, or [`STMT_NIL`](id.md#constants).|

### `StmtFor`

```mach
pub rec StmtFor {
    cond: id.ExprId;
    body: id.StmtId;
}
```

A `for` loop. An absent condition runs forever until a `brk` or `ret`.

| Field | Type                          | Description                                          |
|-------|-------------------------------|------------------------------------------------------|
| cond  | [`id.ExprId`](id.md#exprid)   | Loop condition, or [`EXPR_NIL`](id.md#constants) for infinite loop.|
| body  | [`id.StmtId`](id.md#stmtid)   | Loop body statement (typically `STMT_KIND_BLOCK`).   |

### `StmtRet`

```mach
pub rec StmtRet {
    value: id.ExprId;
}
```

A `ret` statement.

| Field | Type                          | Description                                          |
|-------|-------------------------------|------------------------------------------------------|
| value | [`id.ExprId`](id.md#exprid)   | Returned expression, or [`EXPR_NIL`](id.md#constants) for bare `ret`.|

### `StmtFin`

```mach
pub rec StmtFin {
    body: id.StmtId;
}
```

A `fin { ... }` deferred-execution statement.

| Field | Type                          | Description                                          |
|-------|-------------------------------|------------------------------------------------------|
| body  | [`id.StmtId`](id.md#stmtid)   | Statement to execute at scope exit (typically `STMT_KIND_BLOCK`). |

### `StmtDecl`

```mach
pub rec StmtDecl {
    decl: id.DeclId;
}
```

A local declaration used as a statement (`val` or `var`).

| Field | Type                          | Description              |
|-------|-------------------------------|--------------------------|
| decl  | [`id.DeclId`](id.md#declid)   | The wrapped declaration. |

### `AsmOperandRole`

```mach
pub def AsmOperandRole: u8;
```

Role of an asm operand, encoded in [`AsmOperand.role`](#asmoperand).
See [Constants](#constants) for the enumerated values.

### `AsmOperand`

```mach
pub rec AsmOperand {
    role:   AsmOperandRole;
    name:   token.Span;
    source: token.Span;
}
```

One entry in an asm operand list.

| Field  | Type                                          | Description                                              |
|--------|-----------------------------------------------|----------------------------------------------------------|
| role   | [`AsmOperandRole`](#asmoperandrole)           | Which role the operand plays.                            |
| name   | [`token.Span`](../token.md#span)              | Body-scope identifier for `IN` and `OUT`, or clobbered register name for `CLOBBER`. |
| source | [`token.Span`](../token.md#span)              | mach-level identifier bound to `name` when renamed (e.g. `m: in = msg`); empty span when no rename, always empty for `CLOBBER`. |

### `StmtAsm`

```mach
pub rec StmtAsm {
    isa:            token.Span;
    operands_start: u32;
    operands_len:   u32;
    body:           token.Span;
}
```

An `asm ISA? (operands?) { body }` inline-assembly statement.

| Field          | Type                                  | Description                                                                |
|----------------|---------------------------------------|----------------------------------------------------------------------------|
| isa            | [`token.Span`](../token.md#span)      | Span of the optional ISA identifier (e.g. `x86_64`); empty for portable MASM.|
| operands_start | `u32`                                 | Start index into [`Ast.asm_operands`](../ast.md#ast).                      |
| operands_len   | `u32`                                 | Number of operands; 0 when the operand list is absent or empty.            |
| body           | [`token.Span`](../token.md#span)      | Span covering the raw text between the outer `{` and its matching `}`.     |

### `StmtComptimeIf`

```mach
pub rec StmtComptimeIf {
    branches_start: u32;
    branches_len:   u32;
}
```

A `$if (cond) { stmts } $or (cond) { stmts } $or { stmts }` chain at
statement scope.

| Field          | Type    | Description                                                |
|----------------|---------|------------------------------------------------------------|
| branches_start | `u32`   | Start index into [`Ast.comptime_branches`](../ast.md#ast). |
| branches_len   | `u32`   | Number of arms (at least 1).                               |

### `Stmt`

```mach
pub rec Stmt {
    span: token.Span;
    kind: StmtKind;
    data: uni {
        expr:        StmtExpr;
        block:       StmtBlock;
        if_:         StmtIf;
        for_:        StmtFor;
        ret_:        StmtRet;
        fin:         StmtFin;
        decl:        StmtDecl;
        asm_:        StmtAsm;
        comptime_if: StmtComptimeIf;
    };
}
```

A syntactic statement node. Payload is unused for `BRK`, `CNT`, and
`ERROR` (discriminated by kind alone). The asm body span contains raw
assembly text rather than mach tokens.

| Field | Type                                  | Description                                  |
|-------|---------------------------------------|----------------------------------------------|
| span  | [`token.Span`](../token.md#span)      | Byte range of the statement in source.       |
| kind  | [`StmtKind`](#stmtkind)               | Which `STMT_KIND_*` variant is active.       |
| data  | `uni { … }`                           | Kind-specific payload.                       |

## Constants

```mach
pub val STMT_KIND_EXPR:        StmtKind = 0;
pub val STMT_KIND_BLOCK:       StmtKind = 1;
pub val STMT_KIND_IF:          StmtKind = 2;
pub val STMT_KIND_FOR:         StmtKind = 3;
pub val STMT_KIND_RET:         StmtKind = 4;
pub val STMT_KIND_BRK:         StmtKind = 5;
pub val STMT_KIND_CNT:         StmtKind = 6;
pub val STMT_KIND_FIN:         StmtKind = 7;
pub val STMT_KIND_DECL:        StmtKind = 8;
pub val STMT_KIND_ASM:         StmtKind = 9;
pub val STMT_KIND_COMPTIME_IF: StmtKind = 10;
pub val STMT_KIND_ERROR:       StmtKind = 255;
```

[`StmtKind`](#stmtkind) values.

| Constant                | Value | Notes                                          |
|-------------------------|-------|------------------------------------------------|
| `STMT_KIND_EXPR`        | 0     | Bare-expression statement. Payload: `expr`.    |
| `STMT_KIND_BLOCK`       | 1     | `{ ... }` block. Payload: `block`.             |
| `STMT_KIND_IF`          | 2     | `if`/`or` chain. Payload: `if_`.               |
| `STMT_KIND_FOR`         | 3     | `for` loop. Payload: `for_`.                   |
| `STMT_KIND_RET`         | 4     | `ret` statement. Payload: `ret_`.              |
| `STMT_KIND_BRK`         | 5     | `brk`. No payload.                             |
| `STMT_KIND_CNT`         | 6     | `cnt`. No payload.                             |
| `STMT_KIND_FIN`         | 7     | `fin` deferred-execution. Payload: `fin`.      |
| `STMT_KIND_DECL`        | 8     | Local `val`/`var` decl wrapped as a stmt. Payload: `decl`. |
| `STMT_KIND_ASM`         | 9     | `asm` block. Payload: `asm_`.                  |
| `STMT_KIND_COMPTIME_IF` | 10    | `$if`/`$or` chain at stmt scope. Payload: `comptime_if`. |
| `STMT_KIND_ERROR`       | 255   | Parser-produced poison. No payload.            |

```mach
pub val ASM_OP_IN:      AsmOperandRole = 0;
pub val ASM_OP_OUT:     AsmOperandRole = 1;
pub val ASM_OP_CLOBBER: AsmOperandRole = 2;
```

[`AsmOperandRole`](#asmoperandrole) values.

| Constant         | Value | Meaning                                   |
|------------------|-------|-------------------------------------------|
| `ASM_OP_IN`      | 0     | Input operand.                            |
| `ASM_OP_OUT`     | 1     | Output operand.                           |
| `ASM_OP_CLOBBER` | 2     | Clobbered register.                       |

## Dependencies

`std.types.size`, [`mach.lang.fe.token`](../token.md),
[`mach.lang.fe.ast.id`](id.md).
