# mach.lang.fe.ast.id

Per-`Ast` node identifier types. Each is a `u32` handle into the
corresponding array on an [`Ast`](../ast.md#ast); the type alias exists
to make signatures self-describing.

## Types

### `ExprId`

```mach
pub def ExprId: u32;
```

Handle into [`Ast.exprs`](../ast.md#ast).

### `StmtId`

```mach
pub def StmtId: u32;
```

Handle into [`Ast.stmts`](../ast.md#ast).

### `DeclId`

```mach
pub def DeclId: u32;
```

Handle into [`Ast.decls`](../ast.md#ast).

### `TypeId`

```mach
pub def TypeId: u32;
```

Handle into [`Ast.types`](../ast.md#ast). Distinct from
[`type.TypeId`](../../type.md#typeid), which identifies a *semantic*
type in the [`TypeInterner`](../../type.md#typeinterner). This one
identifies the *syntactic* type written in source.

### `ModuleId`

```mach
pub def ModuleId: u32;
```

Handle into [`Ast.modules`](../ast.md#ast). Distinct from
[`sess.ModuleId`](../../session.md#moduleid), which is the project-wide
handle assigned by the driver.

## Constants

```mach
pub val EXPR_NIL:   ExprId   = 0xFFFFFFFF;
pub val STMT_NIL:   StmtId   = 0xFFFFFFFF;
pub val DECL_NIL:   DeclId   = 0xFFFFFFFF;
pub val TYPE_NIL:   TypeId   = 0xFFFFFFFF;
pub val MODULE_NIL: ModuleId = 0xFFFFFFFF;
```

Absent-node sentinels. Consumers receiving a sentinel from a query must
treat it as an error or as the explicit "no value" case, never as a
default.

## Dependencies

`std.types.size`.

