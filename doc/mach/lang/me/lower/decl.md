# mach.lang.me.lower.decl

Per-decl lowering. Dispatched from
[`lower.lower_module`](../lower.md#lower_module) over the module's
top-level decl list.

Source is `new/lang/me/lower/decl.mach` (currently empty).

## Functions

### `lower_decl`

```mach
pub fun lower_decl(
    ctx: *lower.LowerContext,
    did: id.DeclId,
) Result[bool, str]
```

Dispatches by [`DeclKind`](../../fe/ast/decl.md#declkind):

| `DECL_KIND_*`         | Action                                                                |
|-----------------------|-----------------------------------------------------------------------|
| `FUN`                 | [`lower_fun`](#lower_fun) — emit a new IR function.                   |
| `REC`                 | No-op (records are erased into [`IrType`](../ir/type.md#irtype) entries during sema). |
| `VAL` / `VAR` (module-level) | [`lower_global`](#lower_global) — emit an [`ir.Global`](../ir.md#global). |
| `DEF`                 | No-op (aliases are erased).                                           |
| `USE`                 | No-op (resolution already happened).                                  |
| `TEST`                | Emit as a function and tag it with `FN_FLAG_TEST` so the test runner can find it. |
| `COMPTIME_IF`         | Walk the active arm only; recurse into each contained decl.           |
| `COMPTIME_ATTR`       | No-op at IR level; attributes are consumed earlier.                   |

### `lower_fun`

```mach
pub fun lower_fun(ctx: *lower.LowerContext, did: id.DeclId, d: *decl.Decl) Result[bool, str]
```

1. Resolve the function's signature [`IrTypeId`](../ir/type.md#irtypeid)
   through [`ir_type.intern_fn`](../ir/type.md#functions).
2. Append a new [`Function`](../ir.md#function) to the module.
3. Materialise [`VAL_PARAM`](../ir/value.md#valuekind) values for each
   parameter and seed
   [`LowerContext.locals`](../lower.md#lowercontext).
4. Allocate the entry block; set the builder cursor there.
5. Call [`lower.stmt.lower_stmt`](stmt.md#lower_stmt) on the body.
6. If the entry block falls through, emit a default `ret` (or
   `unreachable` for `FN_FLAG_NORETURN`).

### `lower_global`

```mach
pub fun lower_global(ctx: *lower.LowerContext, did: id.DeclId, d: *decl.Decl) Result[bool, str]
```

1. Look up the initialiser's
   [`comptime.CTValue`](../../fe/comptime.md#ctvalue) from the
   resolve / sema results.
2. Synthesise a [`Value`](../ir/value.md#value) constant and append
   a [`Global`](../ir.md#global) entry.
3. If the initialiser is non-comptime, append its initialisation
   logic to the module's init function — the single synthesized
   function tracked by [`Module.init_fn`](../ir.md#module), created
   lazily on the first non-comptime global. The driver emits a call
   to every module's `init_fn` before `main`.

## Dependencies

`std.types.bool`, `std.types.size`, `std.types.result`,
[`mach.lang.fe.ast`](../../fe/ast.md),
[`mach.lang.fe.ast.id`](../../fe/ast/id.md),
[`mach.lang.fe.ast.decl`](../../fe/ast/decl.md),
[`mach.lang.fe.comptime`](../../fe/comptime.md),
[`mach.lang.me.lower`](../lower.md),
[`mach.lang.me.lower.stmt`](stmt.md),
[`mach.lang.me.ir`](../ir.md),
[`mach.lang.me.ir.builder`](../ir/builder.md),
[`mach.lang.me.ir.type`](../ir/type.md),
[`mach.lang.me.ir.value`](../ir/value.md).
