# mach.lang.fe.comptime.capability

what each phase lets a comptime evaluation ask of it: members, identifiers, types,
fields and layout, published as one record per phase

## val FIELD_SEL_NAME

```mach
pub val FIELD_SEL_NAME:         u8 = 0
```

## val FIELD_SEL_TYPE

```mach
pub val FIELD_SEL_TYPE:         u8 = 1
```

## val FIELD_SEL_OFFSET

```mach
pub val FIELD_SEL_OFFSET:       u8 = 2
```

## val FIELD_SEL_ZERO_BY_NAME

```mach
pub val FIELD_SEL_ZERO_BY_NAME: u8 = 3
```

## val FIELD_SEL_HAS_PAYLOAD

```mach
pub val FIELD_SEL_HAS_PAYLOAD:  u8 = 4
```

## val FIELD_SEL_CODE

```mach
pub val FIELD_SEL_CODE:         u8 = 5
```

## val FIELD_SEL_TYPE_BY_NAME

```mach
pub val FIELD_SEL_TYPE_BY_NAME: u8 = 6
```

## val COMPTIME_CASE_NO_PAYLOAD_MSG

```mach
pub val COMPTIME_CASE_NO_PAYLOAD_MSG: str =
"this case has no payload, so it has no `.type` or `.offset`
```

## val TYPE_QUERY_IS_RECORD

```mach
pub val TYPE_QUERY_IS_RECORD:    u8 = 0
```

## val TYPE_QUERY_IS_UNION

```mach
pub val TYPE_QUERY_IS_UNION:     u8 = 1
```

## val TYPE_QUERY_IS_POINTER

```mach
pub val TYPE_QUERY_IS_POINTER:   u8 = 2
```

## val TYPE_QUERY_NAME

```mach
pub val TYPE_QUERY_NAME:         u8 = 3
```

## val TYPE_QUERY_IS_SECRET

```mach
pub val TYPE_QUERY_IS_SECRET:    u8 = 4
```

## val TYPE_QUERY_IS_TAG

```mach
pub val TYPE_QUERY_IS_TAG:       u8 = 5
```

## val TYPE_QUERY_IS_INTEGER

```mach
pub val TYPE_QUERY_IS_INTEGER:   u8 = 6
```

## val TYPE_QUERY_IS_FLOAT

```mach
pub val TYPE_QUERY_IS_FLOAT:     u8 = 7
```

## val TYPE_QUERY_HOLDS_SECRET

```mach
pub val TYPE_QUERY_HOLDS_SECRET: u8 = 8
```

## val PHASE_CAP_NONE

```mach
pub val PHASE_CAP_NONE:           PhaseCapabilityKind = 0
```

## val PHASE_CAP_LOADING

```mach
pub val PHASE_CAP_LOADING:        PhaseCapabilityKind = 1
```

## val PHASE_CAP_RESOLUTION

```mach
pub val PHASE_CAP_RESOLUTION:     PhaseCapabilityKind = 2
```

## rec NoCapabilityContext

```mach
pub rec NoCapabilityContext;
```

the context of an evaluation that holds no phase capability

## rec PhaseCapabilities

```mach
pub rec PhaseCapabilities[T];
```

## fun capabilities_empty

```mach
pub fun capabilities_empty[T](kind: PhaseCapabilityKind) PhaseCapabilities[T];
```

## fun parsed_expression

```mach
pub fun parsed_expression[T](ctx: *T, a: *ast.Ast, eid: ast_id.ExprId) res[ast_expr.Expr, comptime_failure.EvalFail];
```

## fun read_expression

```mach
pub fun read_expression[T](ctx: *T, caps: PhaseCapabilities[T], a: *ast.Ast, eid: ast_id.ExprId) res[ast_expr.Expr, comptime_failure.EvalFail];
```

## fun no_capabilities

```mach
pub fun no_capabilities() PhaseCapabilities[NoCapabilityContext];
```

no phase capability: an expression reads literals and the constants its context binds

## fun loading_capabilities

```mach
pub fun loading_capabilities[T](member: fun(*T, ast_id.ExprId) res[opt[comptime_value.CTValue], comptime_failure.EvalFail],
cast: fun(*T, ast_id.ExprId, comptime_value.CTValue) res[comptime_value.CTValue, comptime_failure.EvalFail]) res[PhaseCapabilities[T], fail.Fail];
```

## fun resolution_capabilities

```mach
pub fun resolution_capabilities[T](ident: fun(*T, ast_id.ExprId) bool,
expression: fun(*T, *ast.Ast, ast_id.ExprId) res[ast_expr.Expr, comptime_failure.EvalFail]) res[PhaseCapabilities[T], fail.Fail];
```

## fun semantic_name_capabilities

```mach
pub fun semantic_name_capabilities[T](
member: fun(*T, ast_id.ExprId) res[opt[comptime_value.CTValue], comptime_failure.EvalFail],
constant: fun(*T, ast_id.ExprId) res[opt[comptime_value.CTValue], comptime_failure.EvalFail],
store: fun(*T, u32) res[*comptime_deep.Store, comptime_failure.EvalFail],
ident: fun(*T, ast_id.ExprId) bool,
expression: fun(*T, *ast.Ast, ast_id.ExprId) res[ast_expr.Expr, comptime_failure.EvalFail]) res[PhaseCapabilities[T], fail.Fail];
```

## fun semantic_type_capabilities

```mach
pub fun semantic_type_capabilities[T](
member: fun(*T, ast_id.ExprId) res[opt[comptime_value.CTValue], comptime_failure.EvalFail],
constant: fun(*T, ast_id.ExprId) res[opt[comptime_value.CTValue], comptime_failure.EvalFail],
store: fun(*T, u32) res[*comptime_deep.Store, comptime_failure.EvalFail],
type_: fun(*T, ast_id.ExprId) res[opt[u32], comptime_failure.EvalFail],
types: fun(*T) *type.TypeInterner,
offset: fun(*T, u32, u32) res[opt[comptime_value.CTValue], comptime_failure.EvalFail],
query: fun(*T, u32, u8) res[opt[comptime_value.CTValue], comptime_failure.EvalFail],
layout: fun(*T, u32) res[opt[comptime_value.CTValue], comptime_failure.EvalFail],
cast: fun(*T, ast_id.ExprId, comptime_value.CTValue) res[comptime_value.CTValue, comptime_failure.EvalFail],
scalar: fun(*T, ast_id.ExprId, comptime_value.CTValue) res[comptime_value.CTValue, comptime_failure.EvalFail],
ident: fun(*T, ast_id.ExprId) bool,
expression: fun(*T, *ast.Ast, ast_id.ExprId) res[ast_expr.Expr, comptime_failure.EvalFail]) res[PhaseCapabilities[T], fail.Fail];
```

## fun lowering_capabilities

```mach
pub fun lowering_capabilities[T](
member: fun(*T, ast_id.ExprId) res[opt[comptime_value.CTValue], comptime_failure.EvalFail],
store: fun(*T, u32) res[*comptime_deep.Store, comptime_failure.EvalFail],
type_: fun(*T, ast_id.ExprId) res[opt[u32], comptime_failure.EvalFail],
types: fun(*T) *type.TypeInterner,
offset: fun(*T, u32, u32) res[opt[comptime_value.CTValue], comptime_failure.EvalFail],
query: fun(*T, u32, u8) res[opt[comptime_value.CTValue], comptime_failure.EvalFail],
layout: fun(*T, u32) res[opt[comptime_value.CTValue], comptime_failure.EvalFail],
cast: fun(*T, ast_id.ExprId, comptime_value.CTValue) res[comptime_value.CTValue, comptime_failure.EvalFail],
scalar: fun(*T, ast_id.ExprId, comptime_value.CTValue) res[comptime_value.CTValue, comptime_failure.EvalFail],
ident: fun(*T, ast_id.ExprId) bool,
expression: fun(*T, *ast.Ast, ast_id.ExprId) res[ast_expr.Expr, comptime_failure.EvalFail]) res[PhaseCapabilities[T], fail.Fail];
```

## fun capabilities_have_member

```mach
pub fun capabilities_have_member[T](ctx: *T, caps: PhaseCapabilities[T]) bool;
```

## fun capabilities_have_ident

```mach
pub fun capabilities_have_ident[T](ctx: *T, caps: PhaseCapabilities[T]) bool;
```

## fun capabilities_have_constant

```mach
pub fun capabilities_have_constant[T](ctx: *T, caps: PhaseCapabilities[T]) bool;
```

## fun store_of

```mach
pub fun store_of[T](ctx: *T, caps: PhaseCapabilities[T], module: u32) res[*comptime_deep.Store, comptime_failure.EvalFail];
```

the store holding the node a value names, which only a phase that reads interfaces has

## fun capabilities_have_types

```mach
pub fun capabilities_have_types[T](ctx: *T, caps: PhaseCapabilities[T]) bool;
```

## fun capabilities_cast

```mach
pub fun capabilities_cast[T](ctx: *T, caps: PhaseCapabilities[T]) bool;
```

## fun field_of

```mach
pub fun field_of[T](ctx: *T, caps: PhaseCapabilities[T], owner: u32, index: u32, pick: u8) res[opt[comptime_value.CTValue], comptime_failure.EvalFail];
```

what a field descriptor answers about field `index` of `owner`, `pick` naming the member
asked; every answer but the offset reads the type store, and the offset is the phase's

