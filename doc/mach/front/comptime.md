[mach](../README.md) / [front](README.md) / **comptime**

# mach.front.comptime

the comptime evaluator: an expression's value under the capabilities of the phase that
asks, over the scope it runs in

Source: [`src/front/comptime.mach`](../../../src/front/comptime.mach)

Uses types from [`mach.base.float`](../base/float.md), [`mach.base.intern`](../base/intern.md), [`mach.front.ast`](ast.md), [`mach.front.ast.expr`](ast/expr.md), [`mach.front.ast.id`](ast/id.md), [`mach.front.ast.type`](ast/type.md), [`mach.front.comptime.capability`](comptime/capability.md), [`mach.front.comptime.failure`](comptime/failure.md), [`mach.front.comptime.scope`](comptime/scope.md), [`mach.front.comptime.value`](comptime/value.md).

| Functions | |
| :--- | :--- |
| [`evaluate`](#evaluate), [`evaluate_at_float_width`](#evaluate_at_float_width) | |
| [`evaluate_secret_at_float_width`](#evaluate_secret_at_float_width) | the value of the initializer of a constant whose declared type holds a secret, which may read other secret constants: the value it binds is secret in turn |
| [`declared_float_width`](#declared_float_width), [`error_message`](#error_message) | |
| [`names_case`](#names_case) | a literal head names a tag case, `T.c{...}` or `T.[c]{...}`, read as its phase reads it: the parser records the case only for a head with generic arguments, and name resolution splits the others, so the answer is complete once the literal's head has been bound |
| [`literal_head_type`](#literal_head_type) | the head type of a literal in either parsed spelling: the parser emits a positional `T.c{x}` as a lane list, which name resolution reads as a case construction once it learns the head names a case |
| [`literal_case`](#literal_case) | the case the head of the literal `e` selects, STR_NIL for a record literal or any other expression |

| Constants | |
| :--- | :--- |
| [`COMPTIME_TYPE_NEEDS_INSTANCE_MSG`](#constants), [`COMPTIME_FIELD_UNKNOWN_MEMBER_MSG`](#constants), [`COMPTIME_CASE_UNKNOWN_MEMBER_MSG`](#constants), [`COMPTIME_DESCRIPTOR_LAYOUT_MSG`](#constants), [`COMPTIME_SECRET_MSG`](#constants) | |

---

## Functions

### evaluate

```mach
pub fun evaluate[T](
    c: *mach.front.comptime.scope.ComptimeCtx,
    cap_ctx: *T,
    caps: mach.front.comptime.capability.PhaseCapabilities[T],
    a: *mach.front.ast.Ast,
    source: str,
    e: mach.front.ast.id.ExprId,
    interner: *mach.base.intern.Interner) res[mach.front.comptime.value.CTValue, mach.front.comptime.failure.EvalFail]
```

### evaluate_at_float_width

```mach
pub fun evaluate_at_float_width[T](
    c: *mach.front.comptime.scope.ComptimeCtx,
    cap_ctx: *T,
    caps: mach.front.comptime.capability.PhaseCapabilities[T],
    a: *mach.front.ast.Ast,
    source: str,
    e: mach.front.ast.id.ExprId,
    interner: *mach.base.intern.Interner,
    fw: mach.base.float.FloatWidth) res[mach.front.comptime.value.CTValue, mach.front.comptime.failure.EvalFail]
```

### evaluate_secret_at_float_width

```mach
pub fun evaluate_secret_at_float_width[T](
    c: *mach.front.comptime.scope.ComptimeCtx,
    cap_ctx: *T,
    caps: mach.front.comptime.capability.PhaseCapabilities[T],
    a: *mach.front.ast.Ast,
    source: str,
    e: mach.front.ast.id.ExprId,
    interner: *mach.base.intern.Interner,
    fw: mach.base.float.FloatWidth) res[mach.front.comptime.value.CTValue, mach.front.comptime.failure.EvalFail]
```

the value of the initializer of a constant whose declared type holds a secret, which may read
other secret constants: the value it binds is secret in turn

### declared_float_width

```mach
pub fun declared_float_width(a: *mach.front.ast.Ast, source: str, t: mach.front.ast.id.TypeId) mach.base.float.FloatWidth
```

### error_message

```mach
pub fun error_message[T](
    c: *mach.front.comptime.scope.ComptimeCtx,
    cap_ctx: *T,
    caps: mach.front.comptime.capability.PhaseCapabilities[T],
    a: *mach.front.ast.Ast,
    source: str,
    eid: mach.front.ast.id.ExprId,
    interner: *mach.base.intern.Interner) res[str, mach.front.comptime.failure.EvalFail]
```

### names_case

```mach
pub fun names_case(head: mach.front.ast.type.Type) bool
```

a literal head names a tag case, `T.c{...}` or `T.[c]{...}`, read as its phase reads it: the
parser records the case only for a head with generic arguments, and name resolution splits the
others, so the answer is complete once the literal's head has been bound

### literal_head_type

```mach
pub fun literal_head_type(e: *mach.front.ast.expr.Expr) mach.front.ast.id.TypeId
```

the head type of a literal in either parsed spelling: the parser emits a positional `T.c{x}` as a
lane list, which name resolution reads as a case construction once it learns the head names a case

### literal_case

```mach
pub fun literal_case[T](
    c: *mach.front.comptime.scope.ComptimeCtx,
    cap_ctx: *T,
    caps: mach.front.comptime.capability.PhaseCapabilities[T],
    a: *mach.front.ast.Ast,
    source: str,
    e: mach.front.ast.id.ExprId,
    interner: *mach.base.intern.Interner) res[mach.base.intern.StrId, mach.front.comptime.failure.EvalFail]
```

the case the head of the literal `e` selects, STR_NIL for a record literal or any other expression

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `COMPTIME_TYPE_NEEDS_INSTANCE_MSG` | `str` | `"a type comparison on an unsubstituted generic parameter is only decidable at an instantiation"` |  |
| `COMPTIME_FIELD_UNKNOWN_MEMBER_MSG` | `str` | `` "a field descriptor has only `.name`, `.type`, and `.offset`" `` |  |
| `COMPTIME_CASE_UNKNOWN_MEMBER_MSG` | `str` | `` "a case descriptor has only `.name`, `.has_payload`, `.type`, `.offset`, and `.code`" `` |  |
| `COMPTIME_DESCRIPTOR_LAYOUT_MSG` | `str` | `"this descriptor's offset has no answer: the layout of its owning type could not be determined"` |  |
| `COMPTIME_SECRET_MSG` | `str` | `` "a value whose type holds a secret has no compile-time value; declassify it where a compile-time value is meant, as in `K:>u64`" `` |  |

