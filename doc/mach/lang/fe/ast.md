# mach.lang.fe.ast

The per-module AST container. Owns every node array (modules, exprs,
stmts, decls, types) plus typed side pools for variable-length payloads
(`decl_ids`, `stmt_ids`, `expr_ids`, `type_ids`, `typed_names`,
`field_inits`, `generic_names`, `comptime_branches`, `asm_operands`).

Sub-modules:

- [`ast.id`](ast/id.md) — `ExprId`/`StmtId`/`DeclId`/`TypeId`/`ModuleId` aliases.
- [`ast.module`](ast/module.md) — top-level `Module` record.
- [`ast.expr`](ast/expr.md) — `Expr` and its variants.
- [`ast.stmt`](ast/stmt.md) — `Stmt` and its variants.
- [`ast.decl`](ast/decl.md) — `Decl` and its variants.
- [`ast.type`](ast/type.md) — syntactic `Type` and its variants.

## Types

### `Ast`

```mach
pub rec Ast {
    alloc:       *Allocator;
    file_id:     src.FileId;
    root_module: id.ModuleId;

    modules:    *module.Module;
    module_len: usize;
    module_cap: usize;

    exprs:    *expr.Expr;
    expr_len: usize;
    expr_cap: usize;

    stmts:    *stmt.Stmt;
    stmt_len: usize;
    stmt_cap: usize;

    decls:    *decl.Decl;
    decl_len: usize;
    decl_cap: usize;

    types:    *m_type.Type;
    type_len: usize;
    type_cap: usize;

    typed_names:    *decl.TypedName;
    typed_name_len: usize;
    typed_name_cap: usize;

    field_inits:    *expr.FieldInit;
    field_init_len: usize;
    field_init_cap: usize;

    generic_names:    *token.Span;
    generic_name_len: usize;
    generic_name_cap: usize;

    comptime_branches:    *decl.ComptimeBranch;
    comptime_branch_len:  usize;
    comptime_branch_cap:  usize;

    asm_operands:    *stmt.AsmOperand;
    asm_operand_len: usize;
    asm_operand_cap: usize;

    decl_ids:    *id.DeclId;
    decl_id_len: usize;
    decl_id_cap: usize;

    stmt_ids:    *id.StmtId;
    stmt_id_len: usize;
    stmt_id_cap: usize;

    expr_ids:    *id.ExprId;
    expr_id_len: usize;
    expr_id_cap: usize;

    type_ids:    *id.TypeId;
    type_id_len: usize;
    type_id_cap: usize;
}
```

| Field                 | Type                                                 | Description                                                |
|-----------------------|------------------------------------------------------|------------------------------------------------------------|
| alloc                 | `*Allocator`                                         | Allocator backing every owned array on this `Ast`.         |
| file_id               | [`src.FileId`](../source.md#fileid)                  | Identifier of the source file this AST was parsed from.    |
| root_module           | [`id.ModuleId`](ast/id.md#moduleid)                  | Top-level [`Module`](ast/module.md#module), or [`MODULE_NIL`](ast/id.md#constants) before parsing completes. |
| modules               | [`*module.Module`](ast/module.md#module)             | Module array.                                              |
| module_len/cap        | `usize`                                              | Length / capacity of `modules`.                            |
| exprs                 | [`*expr.Expr`](ast/expr.md#expr)                     | Expression array.                                          |
| expr_len/cap          | `usize`                                              | Length / capacity of `exprs`.                              |
| stmts                 | [`*stmt.Stmt`](ast/stmt.md#stmt)                     | Statement array.                                           |
| stmt_len/cap          | `usize`                                              | Length / capacity of `stmts`.                              |
| decls                 | [`*decl.Decl`](ast/decl.md#decl)                     | Declaration array.                                         |
| decl_len/cap          | `usize`                                              | Length / capacity of `decls`.                              |
| types                 | [`*m_type.Type`](ast/type.md#type)                   | Syntactic type array.                                      |
| type_len/cap          | `usize`                                              | Length / capacity of `types`.                              |
| typed_names           | [`*decl.TypedName`](ast/decl.md#typedname)           | Shared pool used by [`DeclFun`](ast/decl.md#declfun) parameters and [`DeclRec`](ast/decl.md#declrec) fields. |
| typed_name_len/cap    | `usize`                                              | Length / capacity of `typed_names`.                        |
| field_inits           | [`*expr.FieldInit`](ast/expr.md#fieldinit)           | Shared pool used by [`ExprStructLit`](ast/expr.md#exprstructlit). |
| field_init_len/cap    | `usize`                                              | Length / capacity of `field_inits`.                        |
| generic_names         | [`*token.Span`](token.md#span)                       | Pool of generic-parameter name spans used by [`DeclFun`](ast/decl.md#declfun) and [`DeclRec`](ast/decl.md#declrec). |
| generic_name_len/cap  | `usize`                                              | Length / capacity of `generic_names`.                      |
| comptime_branches     | [`*decl.ComptimeBranch`](ast/decl.md#comptimebranch) | Pool of [`ComptimeBranch`](ast/decl.md#comptimebranch) records referenced by [`DeclComptimeIf`](ast/decl.md#declcomptimeif) and [`StmtComptimeIf`](ast/stmt.md#stmtcomptimeif). |
| comptime_branch_len/cap | `usize`                                            | Length / capacity of `comptime_branches`.                  |
| asm_operands          | [`*stmt.AsmOperand`](ast/stmt.md#asmoperand)         | Pool of [`AsmOperand`](ast/stmt.md#asmoperand) records referenced by [`StmtAsm`](ast/stmt.md#stmtasm). |
| asm_operand_len/cap   | `usize`                                              | Length / capacity of `asm_operands`.                       |
| decl_ids              | [`*id.DeclId`](ast/id.md#declid)                     | Typed pool of [`DeclId`](ast/id.md#declid)s for node-owned decl lists. |
| decl_id_len/cap       | `usize`                                              | Length / capacity of `decl_ids`.                           |
| stmt_ids              | [`*id.StmtId`](ast/id.md#stmtid)                     | Typed pool of [`StmtId`](ast/id.md#stmtid)s for node-owned stmt lists. |
| stmt_id_len/cap       | `usize`                                              | Length / capacity of `stmt_ids`.                           |
| expr_ids              | [`*id.ExprId`](ast/id.md#exprid)                     | Typed pool of [`ExprId`](ast/id.md#exprid)s for node-owned expr lists. |
| expr_id_len/cap       | `usize`                                              | Length / capacity of `expr_ids`.                           |
| type_ids              | [`*id.TypeId`](ast/id.md#typeid)                     | Typed pool of [`TypeId`](ast/id.md#typeid)s for node-owned type lists. |
| type_id_len/cap       | `usize`                                              | Length / capacity of `type_ids`.                           |

## Constants

```mach
val INITIAL_NODE_CAP:    usize = 32;
val INITIAL_AUX_CAP:     usize = 16;
val INITIAL_ID_POOL_CAP: usize = 64;
```

| Constant              | Description                                                |
|-----------------------|------------------------------------------------------------|
| `INITIAL_NODE_CAP`    | Starting capacity for the primary node arrays (`modules`, `exprs`, `stmts`, `decls`, `types`). Each doubles on overflow. |
| `INITIAL_AUX_CAP`     | Starting capacity for the auxiliary pools (`typed_names`, `field_inits`, `generic_names`, `comptime_branches`, `asm_operands`). Each doubles on overflow. |
| `INITIAL_ID_POOL_CAP` | Starting capacity for the typed id pools (`decl_ids`, `stmt_ids`, `expr_ids`, `type_ids`). Each doubles on overflow.|

## Functions

### `init`

```mach
pub fun init(alloc: *Allocator, file_id: src.FileId) Ast
```

Constructs an empty [`Ast`](#ast) backed by the given allocator.
Infallible — all arrays start nil and grow on first append.

| Param   | Type                                  | Description                                  |
|---------|---------------------------------------|----------------------------------------------|
| alloc   | `*Allocator`                          | Allocator used for every array on the `Ast`. |
| file_id | [`src.FileId`](../source.md#fileid)   | Identifier of the source file this `Ast` belongs to.|

Returns a fresh [`Ast`](#ast) with [`root_module`](#ast) set to
[`MODULE_NIL`](ast/id.md#constants).

### `dnit`

```mach
pub fun dnit(a: *Ast)
```

Releases every array held by the [`Ast`](#ast) and clears its fields.
`nil` is a no-op.

| Param | Type      | Description                          |
|-------|-----------|--------------------------------------|
| a     | `*Ast`    | `Ast` to tear down. `nil` is a no-op.|

### `add_module`

```mach
pub fun add_module(a: *Ast, m: module.Module) Result[id.ModuleId, str]
```

Appends `m` to [`modules`](#ast); grows the array on demand. Returns
the new [`ModuleId`](ast/id.md#moduleid) or an allocation error.

### `add_expr`

```mach
pub fun add_expr(a: *Ast, e: expr.Expr) Result[id.ExprId, str]
```

Appends `e` to [`exprs`](#ast); grows the array on demand. Returns
the new [`ExprId`](ast/id.md#exprid) or an allocation error.

### `add_stmt`

```mach
pub fun add_stmt(a: *Ast, s: stmt.Stmt) Result[id.StmtId, str]
```

Appends `s` to [`stmts`](#ast); grows the array on demand. Returns
the new [`StmtId`](ast/id.md#stmtid) or an allocation error.

### `add_decl`

```mach
pub fun add_decl(a: *Ast, d: decl.Decl) Result[id.DeclId, str]
```

Appends `d` to [`decls`](#ast); grows the array on demand. Returns
the new [`DeclId`](ast/id.md#declid) or an allocation error.

### `add_type`

```mach
pub fun add_type(a: *Ast, t: m_type.Type) Result[id.TypeId, str]
```

Appends `t` to [`types`](#ast); grows the array on demand. Returns
the new [`TypeId`](ast/id.md#typeid) or an allocation error.

### `get_module`

```mach
pub fun get_module(a: *Ast, id_: id.ModuleId) Option[*module.Module]
```

Returns `some(*Module)` into [`modules`](#ast) when `id_` is in range,
`none` otherwise. Pointer is invalidated by a later
[`add_module`](#add_module) that grows the array.

### `get_expr`

```mach
pub fun get_expr(a: *Ast, id_: id.ExprId) Option[*expr.Expr]
```

Returns `some(*Expr)` into [`exprs`](#ast) when `id_` is in range,
`none` otherwise. Pointer is invalidated by a later
[`add_expr`](#add_expr) that grows the array.

### `get_stmt`

```mach
pub fun get_stmt(a: *Ast, id_: id.StmtId) Option[*stmt.Stmt]
```

Returns `some(*Stmt)` into [`stmts`](#ast) when `id_` is in range,
`none` otherwise. Pointer is invalidated by a later
[`add_stmt`](#add_stmt) that grows the array.

### `get_decl`

```mach
pub fun get_decl(a: *Ast, id_: id.DeclId) Option[*decl.Decl]
```

Returns `some(*Decl)` into [`decls`](#ast) when `id_` is in range,
`none` otherwise. Pointer is invalidated by a later
[`add_decl`](#add_decl) that grows the array.

### `get_type`

```mach
pub fun get_type(a: *Ast, id_: id.TypeId) Option[*m_type.Type]
```

Returns `some(*Type)` into [`types`](#ast) when `id_` is in range,
`none` otherwise. Pointer is invalidated by a later
[`add_type`](#add_type) that grows the array.

### `add_typed_name`

```mach
pub fun add_typed_name(a: *Ast, tn: decl.TypedName) Result[u32, str]
```

Appends a [`TypedName`](ast/decl.md#typedname) to the shared pool
referenced by `DeclFun.params_start` / `DeclRec.fields_start`. Errors
on allocation failure.

Returns the pool index for the new entry.

### `add_field_init`

```mach
pub fun add_field_init(a: *Ast, fi: expr.FieldInit) Result[u32, str]
```

Appends a [`FieldInit`](ast/expr.md#fieldinit) to the shared pool
referenced by `ExprStructLit.fields_start`. Errors on allocation
failure.

Returns the pool index for the new entry.

### `add_generic_name`

```mach
pub fun add_generic_name(a: *Ast, name: token.Span) Result[u32, str]
```

Appends a generic-parameter name span to the shared pool. Errors on
allocation failure.

| Param | Type                                  | Description                                  |
|-------|---------------------------------------|----------------------------------------------|
| a     | `*Ast`                                | Target `Ast`.                                |
| name  | [`token.Span`](token.md#span)         | Span of the generic parameter identifier.    |

Returns the pool index for the new entry.

### `add_comptime_branch`

```mach
pub fun add_comptime_branch(a: *Ast, br: decl.ComptimeBranch) Result[u32, str]
```

Appends a [`ComptimeBranch`](ast/decl.md#comptimebranch) to the shared
pool. Errors on allocation failure.

Returns the pool index for the new entry.

### `add_asm_operand`

```mach
pub fun add_asm_operand(a: *Ast, op: stmt.AsmOperand) Result[u32, str]
```

Appends an [`AsmOperand`](ast/stmt.md#asmoperand) to the shared pool.
Errors on allocation failure.

Returns the pool index for the new entry.

### `add_decl_id`

```mach
pub fun add_decl_id(a: *Ast, id_: id.DeclId) Result[u32, str]
```

Appends a [`DeclId`](ast/id.md#declid) to the
[`decl_ids`](#ast) pool. Used by node-owned [`DeclId`](ast/id.md#declid)
lists ([`Module.decls`](ast/module.md#module), decl-scope
[`ComptimeBranch`](ast/decl.md#comptimebranch) bodies). Errors on
allocation failure.

| Param | Type                          | Description                          |
|-------|-------------------------------|--------------------------------------|
| a     | `*Ast`                        | Target `Ast`.                        |
| id_   | [`id.DeclId`](ast/id.md#declid) | DeclId to append.                  |

Returns the [`decl_ids`](#ast) pool index for the new entry.

### `add_stmt_id`

```mach
pub fun add_stmt_id(a: *Ast, id_: id.StmtId) Result[u32, str]
```

Appends a [`StmtId`](ast/id.md#stmtid) to the
[`stmt_ids`](#ast) pool. Used by node-owned [`StmtId`](ast/id.md#stmtid)
lists ([`StmtBlock.stmts`](ast/stmt.md#stmtblock), stmt-scope
[`ComptimeBranch`](ast/decl.md#comptimebranch) bodies). Errors on
allocation failure.

| Param | Type                          | Description                          |
|-------|-------------------------------|--------------------------------------|
| a     | `*Ast`                        | Target `Ast`.                        |
| id_   | [`id.StmtId`](ast/id.md#stmtid) | StmtId to append.                  |

Returns the [`stmt_ids`](#ast) pool index for the new entry.

### `add_expr_id`

```mach
pub fun add_expr_id(a: *Ast, id_: id.ExprId) Result[u32, str]
```

Appends an [`ExprId`](ast/id.md#exprid) to the
[`expr_ids`](#ast) pool. Used by node-owned [`ExprId`](ast/id.md#exprid)
lists ([`ExprCall.args`](ast/expr.md#exprcall),
[`ExprArrayLit.elems`](ast/expr.md#exprarraylit)). Errors on allocation
failure.

| Param | Type                          | Description                          |
|-------|-------------------------------|--------------------------------------|
| a     | `*Ast`                        | Target `Ast`.                        |
| id_   | [`id.ExprId`](ast/id.md#exprid) | ExprId to append.                  |

Returns the [`expr_ids`](#ast) pool index for the new entry.

### `add_type_id`

```mach
pub fun add_type_id(a: *Ast, id_: id.TypeId) Result[u32, str]
```

Appends a [`TypeId`](ast/id.md#typeid) to the
[`type_ids`](#ast) pool. Used by node-owned [`TypeId`](ast/id.md#typeid)
lists ([`TypeNamed.args`](ast/type.md#typenamed),
[`TypeFun.params`](ast/type.md#typefun)). Errors on allocation failure.

| Param | Type                          | Description                          |
|-------|-------------------------------|--------------------------------------|
| a     | `*Ast`                        | Target `Ast`.                        |
| id_   | [`id.TypeId`](ast/id.md#typeid) | TypeId to append.                  |

Returns the [`type_ids`](#ast) pool index for the new entry.

## Dependencies

`std.types.bool`, `std.types.option`, `std.types.size`,
`std.types.string`, `std.types.result`, `std.allocator`,
[`mach.lang.fe.token`](token.md), [`mach.lang.source`](../source.md),
[`mach.lang.fe.ast.id`](ast/id.md),
[`mach.lang.fe.ast.module`](ast/module.md),
[`mach.lang.fe.ast.expr`](ast/expr.md),
[`mach.lang.fe.ast.stmt`](ast/stmt.md),
[`mach.lang.fe.ast.decl`](ast/decl.md),
[`mach.lang.fe.ast.type`](ast/type.md).
