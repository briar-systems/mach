# mach.lang.fe.sema.check

Type-compatibility validation. Called from
[`infer`](infer.md) at composing sites (call arguments, assignment
targets, return values, field initializers, array elements). Failures
emit diagnostics into [`session.diags`](../../session.md#session); the
caller is expected to mark the failed position with
[`TYPE_ERROR`](../../type.md#typekind) and continue.

## Functions

### `check_assignable`

```mach
pub fun check_assignable(sc: *sema.SemaContext, expected: type.TypeId, actual: type.TypeId, span: token.Span) bool
```

Verifies that a value of type `actual` is assignable to a slot of type
`expected`. Tries [`coerce.try_coerce_literal`](coerce.md#try_coerce_literal)
first when one side is a literal default type; falls back to
identity equality.

Emits `"type mismatch: expected <expected>, got <actual>"` on failure.

| Param    | Type                                          | Description                                  |
|----------|-----------------------------------------------|----------------------------------------------|
| sc       | [`*sema.SemaContext`](../sema.md#semacontext) | Sema context.                                |
| expected | [`type.TypeId`](../../type.md#typeid)         | Target slot's type.                          |
| actual   | [`type.TypeId`](../../type.md#typeid)         | Value's inferred type.                       |
| span     | [`token.Span`](../../token.md#span)           | Span used for the diagnostic.                |

Returns `true` when compatible, `false` after emitting the diagnostic.

### `check_call`

```mach
pub fun check_call(sc: *sema.SemaContext, callee_sig: type.TypeId, args_start: u32, args_len: u32, span: token.Span) bool
```

Verifies a call against a function signature. Arity-checks
([`args_len`](#check_call) vs the callee's
[`TypeFun.params_len`](../../type.md#typefun)) and then walks each
argument expression, calling [`check_assignable`](#check_assignable)
against the corresponding parameter type.

Emits `"not callable: <callee_sig>"` when `callee_sig` is not
[`TYPE_FUN`](../../type.md#typefun), `"arity mismatch: expected N
arguments, got M"` on length mismatch, and one
`"type mismatch: ..."` per bad argument.

| Param      | Type                                          | Description                                              |
|------------|-----------------------------------------------|----------------------------------------------------------|
| sc         | [`*sema.SemaContext`](../sema.md#semacontext) | Sema context.                                            |
| callee_sig | [`type.TypeId`](../../type.md#typeid)         | The callee's signature.                                  |
| args_start | `u32`                                         | Start index into [`ast.expr_ids`](../../ast.md#ast).     |
| args_len   | `u32`                                         | Number of argument [`ExprId`](../../ast/id.md#exprid)s.  |
| span       | [`token.Span`](../../token.md#span)           | Span of the call expression for diagnostics.             |

Returns `true` when the call validates fully.

### `check_index`

```mach
pub fun check_index(sc: *sema.SemaContext, object: type.TypeId, index: type.TypeId, span: token.Span) Option[type.TypeId]
```

Verifies an index expression. Object must be
[`TYPE_POINTER`](../../type.md#typekind) or
[`TYPE_ARRAY`](../../type.md#typekind); index must be an integer type
(u8/u16/u32/u64/i8/i16/i32/i64).

| Param  | Type                                          | Description                                  |
|--------|-----------------------------------------------|----------------------------------------------|
| sc     | [`*sema.SemaContext`](../sema.md#semacontext) | Sema context.                                |
| object | [`type.TypeId`](../../type.md#typeid)         | Type of the indexed expression.              |
| index  | [`type.TypeId`](../../type.md#typeid)         | Type of the index expression.                |
| span   | [`token.Span`](../../token.md#span)           | Span used for diagnostics.                   |

Returns `some(elem_type)` on success, `none` after emitting
`"not indexable: ..."` or `"index must be an integer type"`.

### `check_member`

```mach
pub fun check_member(sc: *sema.SemaContext, object: type.TypeId, name: token.Span, span: token.Span) Option[type.TypeId]
```

Verifies a member access. Object must be
[`TYPE_REC`](../../type.md#typekind) or
[`TYPE_UNI`](../../type.md#typekind); the named field must exist in
the type's field table.

| Param  | Type                                          | Description                                  |
|--------|-----------------------------------------------|----------------------------------------------|
| sc     | [`*sema.SemaContext`](../sema.md#semacontext) | Sema context.                                |
| object | [`type.TypeId`](../../type.md#typeid)         | Type of the object expression.               |
| name   | [`token.Span`](../../token.md#span)           | Span of the field identifier.                |
| span   | [`token.Span`](../../token.md#span)           | Span of the full member expression.          |

Returns `some(field_type)` on success, `none` after emitting
`"no such field: <name>"` or `"member access on non-record type"`.

### `check_cast`

```mach
pub fun check_cast(sc: *sema.SemaContext, from: type.TypeId, to: type.TypeId, span: token.Span) Option[type.TypeId]
```

Verifies an `expr::Type` cast under the
[language-reference rules](../../../language/types.md#type-casting):

1. **Primitive numeric conversion**: both `from` and `to` are
   primitives → permitted; produces a converted value.
2. **Same-size bit reinterpret**: neither is a primitive numeric;
   `$size_of(from) == $size_of(to)` → permitted.
3. **Anything else**: emits `cast: size mismatch (M vs N bytes)` or
   `cast: invalid (from <from> to <to>)` and returns `none`.

| Param | Type                                          | Description                                  |
|-------|-----------------------------------------------|----------------------------------------------|
| sc    | [`*sema.SemaContext`](../sema.md#semacontext) | Sema context.                                |
| from  | [`type.TypeId`](../../type.md#typeid)         | Source value's type.                         |
| to    | [`type.TypeId`](../../type.md#typeid)         | Destination type.                            |
| span  | [`token.Span`](../../token.md#span)           | Span of the `::` operator.                   |

Returns `some(to)` on success, `none` after emitting a diagnostic.

### `check_return`

```mach
pub fun check_return(sc: *sema.SemaContext, fn_ret: type.TypeId, value: type.TypeId, span: token.Span) bool
```

Verifies a `ret expr` against the enclosing function's declared
return type.

- If `fn_ret` is [`TYPE_NIL`](../../type.md#type_nil) (function has no
  declared return type) and `value` is [`TYPE_NIL`](../../type.md#type_nil)
  (bare `ret;`), success.
- If `fn_ret` is [`TYPE_NIL`](../../type.md#type_nil) and `value` is
  not, emits `"return value in function with no return type"`.
- If `fn_ret` is set, delegates to [`check_assignable`](#check_assignable).

| Param  | Type                                          | Description                                  |
|--------|-----------------------------------------------|----------------------------------------------|
| sc     | [`*sema.SemaContext`](../sema.md#semacontext) | Sema context.                                |
| fn_ret | [`type.TypeId`](../../type.md#typeid)         | Declared return type of the enclosing fun.   |
| value  | [`type.TypeId`](../../type.md#typeid)         | Inferred type of the returned expression, or [`TYPE_NIL`](../../type.md#type_nil) for bare `ret;`. |
| span   | [`token.Span`](../../token.md#span)           | Span of the `ret` statement.                 |

Returns `true` when the return validates.

### `check_condition`

```mach
pub fun check_condition(sc: *sema.SemaContext, cond: type.TypeId, span: token.Span) bool
```

Verifies a control-flow condition (`if (cond)`, `for (cond)`,
`while`-style loops). Mach treats any integer-typed value as a
condition; `cond` must be one of the integer primitives.

Emits `"condition must be an integer type"` otherwise.

| Param | Type                                          | Description                          |
|-------|-----------------------------------------------|--------------------------------------|
| sc    | [`*sema.SemaContext`](../sema.md#semacontext) | Sema context.                        |
| cond  | [`type.TypeId`](../../type.md#typeid)         | Inferred type of the condition.      |
| span  | [`token.Span`](../../token.md#span)           | Span of the condition expression.    |

Returns `true` when the condition is valid.

## Internal helpers

File-private; listed for reference.

| Function              | Role                                                                       |
|-----------------------|----------------------------------------------------------------------------|
| `is_numeric`          | Predicate: is the kind one of `TYPE_U8`..`TYPE_F64`?                       |
| `is_integer`          | Predicate: is the kind one of `TYPE_U8`..`TYPE_I64`?                       |
| `is_primitive_numeric`| Predicate: is the kind one of the int/float primitives (excludes `TYPE_PTR`, `TYPE_STR`)? |
| `format_type`         | Produces a human-readable string for a [`TypeId`](../../type.md#typeid) used in diagnostic messages (e.g. `"*[3]i32"`, `"fun(i32, i32) i32"`). |
| `size_of`             | Returns the size in bytes of a [`TypeId`](../../type.md#typeid). Used by [`check_cast`](#check_cast). |

## Dependencies

`std.types.bool`, `std.types.option`, `std.types.size`,
`std.types.string`, `std.types.result`,
[`mach.lang.diagnostic`](../../diagnostic.md),
[`mach.lang.type`](../../type.md), [`mach.lang.fe.ast`](../../ast.md),
[`mach.lang.fe.ast.id`](../../ast/id.md),
[`mach.lang.fe.token`](../../token.md),
[`mach.lang.fe.sema`](../sema.md),
[`mach.lang.fe.sema.coerce`](coerce.md).
