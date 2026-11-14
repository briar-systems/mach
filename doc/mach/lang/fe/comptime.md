# mach.lang.fe.comptime

the comptime evaluator: an expression's value under the capabilities of the phase that
asks, over the scope it runs in

## val COMPTIME_TYPE_NEEDS_INSTANCE_MSG

```mach
pub val COMPTIME_TYPE_NEEDS_INSTANCE_MSG: str =
"a type comparison on an unsubstituted generic parameter is only decidable at an instantiation"
```

## val COMPTIME_FIELD_UNKNOWN_MEMBER_MSG

```mach
pub val COMPTIME_FIELD_UNKNOWN_MEMBER_MSG: str =
"a field descriptor has only `.name`, `.type`, and `.offset`"
```

## val COMPTIME_CASE_UNKNOWN_MEMBER_MSG

```mach
pub val COMPTIME_CASE_UNKNOWN_MEMBER_MSG: str =
"a case descriptor has only `.name`, `.has_payload`, `.type`, `.offset`, and `.code`"
```

## val COMPTIME_DESCRIPTOR_LAYOUT_MSG

```mach
pub val COMPTIME_DESCRIPTOR_LAYOUT_MSG: str =
"this descriptor's offset has no answer: the layout of its owning type could not be determined"
```

## val COMPTIME_SECRET_MSG

```mach
pub val COMPTIME_SECRET_MSG: str =
"a value whose type holds a secret has no compile-time value
```

## fun evaluate

```mach
pub fun evaluate[T](
c: *comptime_scope.ComptimeCtx,
cap_ctx: *T,
caps: comptime_capability.PhaseCapabilities[T],
a: *ast.Ast,
source: str,
e: ast_id.ExprId,
interner: *intern.Interner) res[comptime_value.CTValue, comptime_failure.EvalFail];
```

## fun evaluate_at_float_width

```mach
pub fun evaluate_at_float_width[T](
c: *comptime_scope.ComptimeCtx,
cap_ctx: *T,
caps: comptime_capability.PhaseCapabilities[T],
a: *ast.Ast,
source: str,
e: ast_id.ExprId,
interner: *intern.Interner,
fw: float.FloatWidth) res[comptime_value.CTValue, comptime_failure.EvalFail];
```

## fun evaluate_secret_at_float_width

```mach
pub fun evaluate_secret_at_float_width[T](
c: *comptime_scope.ComptimeCtx,
cap_ctx: *T,
caps: comptime_capability.PhaseCapabilities[T],
a: *ast.Ast,
source: str,
e: ast_id.ExprId,
interner: *intern.Interner,
fw: float.FloatWidth) res[comptime_value.CTValue, comptime_failure.EvalFail];
```

the value of the initializer of a constant whose declared type holds a secret, which may read
other secret constants: the value it binds is secret in turn

## fun declared_float_width

```mach
pub fun declared_float_width(a: *ast.Ast, source: str, t: ast_id.TypeId) float.FloatWidth;
```

## fun error_message

```mach
pub fun error_message[T](
c: *comptime_scope.ComptimeCtx,
cap_ctx: *T,
caps: comptime_capability.PhaseCapabilities[T],
a: *ast.Ast,
source: str,
eid: ast_id.ExprId,
interner: *intern.Interner) res[str, comptime_failure.EvalFail];
```

## fun names_case

```mach
pub fun names_case(head: ast_type.Type) bool;
```

a literal head names a tag case, `T.c{...}` or `T.[c]{...}`, read as its phase reads it: the
parser records the case only for a head with generic arguments, and name resolution splits the
others, so the answer is complete once the literal's head has been bound

## fun literal_head_type

```mach
pub fun literal_head_type(e: *ast_expr.Expr) ast_id.TypeId;
```

the head type of a literal in either parsed spelling: the parser emits a positional `T.c{x}` as a
lane list, which name resolution reads as a case construction once it learns the head names a case

## fun literal_case

```mach
pub fun literal_case[T](
c: *comptime_scope.ComptimeCtx,
cap_ctx: *T,
caps: comptime_capability.PhaseCapabilities[T],
a: *ast.Ast,
source: str,
e: ast_id.ExprId,
interner: *intern.Interner) res[intern.StrId, comptime_failure.EvalFail];
```

the case the head of the literal `e` selects, STR_NIL for a record literal or any other expression

