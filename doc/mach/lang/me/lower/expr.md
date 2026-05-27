# mach.lang.me.lower.expr

Per-expr lowering. Produces a [`Value`](../ir/value.md#value) for
every expression. Lvalues / rvalues are disambiguated by call site:
[`lower_rvalue`](#lower_rvalue) returns a loaded value;
[`lower_lvalue`](#lower_lvalue) returns a pointer.

Source is `new/lang/me/lower/expr.mach` (currently empty).

## Functions

### `lower_rvalue`

```mach
pub fun lower_rvalue(
    ctx: *lower.LowerContext,
    eid: id.ExprId,
) Result[value.Value, str]
```

Dispatches by [`ExprKind`](../../fe/ast/expr.md#exprkind):

| `EXPR_KIND_*`                | IR form                                                                  |
|------------------------------|--------------------------------------------------------------------------|
| `LIT_INT` / `LIT_FLOAT`      | [`const_int`](../ir/value.md#functions) / [`const_float`](../ir/value.md#functions) directly. |
| `LIT_STR`                    | `const_bytes` Value (the fat `str`) plus an entry in the module's read-only data globals. |
| `LIT_ZSTR`                   | `const_bytes` for the null-terminated bytes in a read-only data global; the Value is the `*u8` pointer to it. |
| `LIT_CHAR`                   | `const_int` of the literal's byte (`u8`).                                |
| `LIT_NIL`                    | `const_null` at the site's pointer type.                                 |
| `IDENT`                      | Look up in [`LowerContext.locals`](../lower.md#lowercontext): if alloca'd, emit `load`; if SSA, return the bound Value. `true` / `false` are ordinary identifiers resolving to userland constants and lower through this row. |
| `MEMBER`                     | `gep` for record fields; `extract` for register aggregates.             |
| `CALL`                       | Lower callee + args, emit `call`.                                        |
| `BINARY`                     | Dispatch by operator → arithmetic / comparison / bitwise instruction.    |
| `UNARY`                      | `neg` / `not` / address-of (returns a pointer) / dereference (`load`).   |
| `INDEX`                      | `gep` to compute element pointer + `load`.                               |
| `CAST` (`::`)                | One conversion instruction selected by the source / target type pair.    |
| `ARRAY_LIT`                  | Constant-foldable literals become a `const_bytes` read-only global; otherwise an `alloca` + per-element `store` through `gep`. |
| `STRUCT_LIT`                 | Per-field `store` through `gep` into an `alloca` (or an `OP_INSERT` chain for register aggregates); constant-foldable literals become a `const_bytes` global. |
| `COMPTIME_IDENT`             | Look up the folded [`CTValue`](../../fe/comptime.md#ctvalue) from [`LowerContext.ctx`](../lower.md#lowercontext); emit the corresponding constant `Value`. |

### `lower_lvalue`

```mach
pub fun lower_lvalue(
    ctx: *lower.LowerContext,
    eid: id.ExprId,
) Result[value.Value, str]
```

For the expression forms that name a memory location, returns a
pointer Value:

| `EXPR_KIND_*` | Result                                                                  |
|---------------|-------------------------------------------------------------------------|
| `IDENT`       | The alloca pointer bound in `LowerContext.locals` (or the global pointer for module-level bindings). |
| `MEMBER`      | `gep` against the base lvalue + field offset.                            |
| `INDEX`       | `gep` against the base lvalue + index.                                   |
| `UNARY` (deref) | The operand evaluated as an rvalue (which is already a pointer).      |

Other expression kinds are not valid lvalues and produce a
diagnostic during sema; lowering may panic if it sees one (invariant
relied on by the verifier).

## Dependencies

`std.types.bool`, `std.types.size`, `std.types.result`,
[`mach.lang.fe.ast`](../../fe/ast.md),
[`mach.lang.fe.ast.id`](../../fe/ast/id.md),
[`mach.lang.fe.ast.expr`](../../fe/ast/expr.md),
[`mach.lang.fe.comptime`](../../fe/comptime.md),
[`mach.lang.me.lower`](../lower.md),
[`mach.lang.me.ir`](../ir.md),
[`mach.lang.me.ir.builder`](../ir/builder.md),
[`mach.lang.me.ir.value`](../ir/value.md),
[`mach.lang.me.ir.type`](../ir/type.md).
