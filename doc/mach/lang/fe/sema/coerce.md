# mach.lang.fe.sema.coerce

Implicit coercion rules. Mach is explicit-everything by design: there
is **no implicit numeric widening between variables**. The only
implicit conversions sema applies are literal-to-declared-type and a
small set of pointer mutability rules. Every other type difference
requires an explicit `::` cast.

## Functions

### `try_coerce_literal`

```mach
pub fun try_coerce_literal(sc: *sema.SemaContext, eid: id.ExprId, to: type.TypeId) Option[type.TypeId]
```

Attempts to coerce a literal expression `eid` from its default type to
the target type `to`. Per the [language reference](../../../../language/types.md#literal-coercion),
literal coercion applies to integer and float literals only and only
at the point of declaration / annotation / call / assignment.

| Expression form                                                   | Coercible to                                              |
|-------------------------------------------------------------------|-----------------------------------------------------------|
| [`EXPR_KIND_LIT_INT`](../ast/expr.md#exprkind) with value `v`  | Any integer primitive type that can represent `v`.        |
| [`EXPR_KIND_LIT_FLOAT`](../ast/expr.md#exprkind)               | [`TYPE_F32`](../../type.md#constants) or [`TYPE_F64`](../../type.md#constants). |
| [`EXPR_KIND_LIT_NIL`](../ast/expr.md#exprkind)                 | Any [`TYPE_POINTER`](../../type.md#typekind) or [`TYPE_PTR`](../../type.md#constants). |
| [`EXPR_KIND_LIT_CHAR`](../ast/expr.md#exprkind) with value `v` | Any integer primitive type that can represent `v`.        |
| anything else                                                     | Not coercible.                                            |

On a successful coercion, this function also rewrites
[`sc.expr_type[eid]`](../sema.md#semacontext) to `to` so downstream
consumers see the coerced type.

| Param | Type                                          | Description                          |
|-------|-----------------------------------------------|--------------------------------------|
| sc    | [`*sema.SemaContext`](../sema.md#semacontext) | Sema context.                        |
| eid   | [`id.ExprId`](../ast/id.md#exprid)         | Literal expression to coerce.        |
| to    | [`type.TypeId`](../../type.md#typeid)         | Target type.                         |

Returns `some(to)` when coercion succeeded, `none` otherwise (the
caller then falls back to identity equality, then emits a diagnostic).

### `is_assignable`

```mach
pub fun is_assignable(sc: *sema.SemaContext, from: type.TypeId, to: type.TypeId) bool
```

Identity-equality predicate: `from` is assignable to `to` iff
`from == to`. Every compound type — including
[`TYPE_FUN`](../../type.md#typekind) — is fully canonicalised by the
[`TypeInterner`](../../type.md#typeinterner)'s dedup tables (see
[`type.intern_function`](../../type.md#intern_function)), so
[`TypeId`](../../type.md#typeid) equality already means structural
equality. No per-kind special case is needed.

For every other kind, identity (`from == to`) is the rule.

| Param | Type                                          | Description                          |
|-------|-----------------------------------------------|--------------------------------------|
| sc    | [`*sema.SemaContext`](../sema.md#semacontext) | Sema context.                        |
| from  | [`type.TypeId`](../../type.md#typeid)         | Source type.                         |
| to    | [`type.TypeId`](../../type.md#typeid)         | Target type.                         |

Returns `true` when `from` is assignable to `to` without an explicit
cast (and without a literal coercion — that's [`try_coerce_literal`](#try_coerce_literal)'s job).

## Rules (full enumeration)

The rules sema applies, in order:

1. **Literal coercion** (handled by [`try_coerce_literal`](#try_coerce_literal)
   at composing sites only).
2. **Identity** (the default — `from == to` ⇒ assignable).
3. **Function structural equivalence** (the [`is_assignable`](#is_assignable)
   special case above).

That's it. There is no:

- Implicit integer widening (`u8` → `u32` requires `x::u32`).
- Implicit float widening or float ↔ int conversion.
- Implicit pointer-base widening (a `*u8` is not a `*ptr`).
- Implicit dereference or address-of.
- Mutable-to-readonly pointer coercion — Mach has no readonly pointer
  type (see [memory.md](../../../../language/memory.md)).

Any difference not covered above is a type error.

## Internal helpers

File-private; listed for reference.

| Function                | Role                                                                       |
|-------------------------|----------------------------------------------------------------------------|
| `literal_int_value`     | Extracts the numeric value of a [`LIT_INT`](../ast/expr.md#exprkind) expression for the range-check in [`try_coerce_literal`](#try_coerce_literal). |
| `fits_in`               | Predicate: does integer value `v` fit in the range of the target primitive integer type? |

## Dependencies

`std.types.bool`, `std.types.option`, `std.types.size`,
`std.types.string`, `std.types.result`,
[`mach.lang.type`](../../type.md), [`mach.lang.fe.ast`](../ast.md),
[`mach.lang.fe.ast.id`](../ast/id.md),
[`mach.lang.fe.ast.expr`](../ast/expr.md),
[`mach.lang.fe.sema`](../sema.md).
