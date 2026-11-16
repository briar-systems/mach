[mach](../../README.md) / [front](../README.md) / [comptime](README.md) / **capability**

# mach.front.comptime.capability

what each phase lets a comptime evaluation ask of it: members, identifiers, types,
fields and layout, published as one record per phase

<sub>source: [`src/front/comptime/capability.mach`](../../../../src/front/comptime/capability.mach) · imported by 13 modules</sub>

<sub>`ast` is [`mach.front.ast`](../ast.md) · `ast_id` is [`mach.front.ast.id`](../ast/id.md) · `ast_expr` is [`mach.front.ast.expr`](../ast/expr.md) · `comptime_failure` is [`mach.front.comptime.failure`](failure.md) · `ast_type` is [`mach.front.ast.type`](../ast/type.md) · `PhaseCapabilityKind` is private · `comptime_value` is [`mach.front.comptime.value`](value.md) · `type` is [`mach.type`](../../type.md) · `comptime_deep` is [`mach.front.comptime.deep`](deep.md) · `fail` is [`mach.base.fail`](../../base/fail.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`NoCapabilityContext`](#nocapabilitycontext) | `rec` | the context of an evaluation that holds no phase capability |
| [`PhaseCapabilities`](#phasecapabilities) | `rec` |  |

| On `PhaseCapabilities` | |
| :--- | :--- |
| [`capabilities_make`](#capabilities_make) · [`read_head`](#read_head) · [`read_expression`](#read_expression) | |
| [`no_capabilities`](#no_capabilities) | no phase capability: an expression reads literals and the constants its context binds |
| [`loading_capabilities`](#loading_capabilities) · [`resolution_capabilities`](#resolution_capabilities) · [`semantic_name_capabilities`](#semantic_name_capabilities) · [`semantic_type_capabilities`](#semantic_type_capabilities) · [`lowering_capabilities`](#lowering_capabilities) · [`capabilities_have_member`](#capabilities_have_member) · [`capabilities_have_ident`](#capabilities_have_ident) · [`capabilities_have_constant`](#capabilities_have_constant) | |
| [`store_of`](#store_of) | the store holding the node a value names, which only a phase that reads interfaces has |
| [`capabilities_have_types`](#capabilities_have_types) | |
| [`capabilities_lowering`](#capabilities_lowering) | whether the phase is lowering, which materializes only values an earlier phase asked for |
| [`capabilities_cast`](#capabilities_cast) | |
| [`field_of`](#field_of) | what a field descriptor answers about field `index` of `owner`, `pick` naming the member asked; every answer but the offset reads the type store, and the offset is the phase's |

| Functions | |
| :--- | :--- |
| [`parsed_expression`](#parsed_expression) · [`parsed_head`](#parsed_head) | |

| Constants | |
| :--- | :--- |
| [`FIELD_SEL_NAME`](#constants) · [`FIELD_SEL_TYPE`](#constants) · [`FIELD_SEL_OFFSET`](#constants) · [`FIELD_SEL_HAS_PAYLOAD`](#constants) · [`FIELD_SEL_CODE`](#constants) · [`FIELD_SEL_TYPE_BY_NAME`](#constants) · [`COMPTIME_CASE_NO_PAYLOAD_MSG`](#constants) · [`TYPE_QUERY_IS_RECORD`](#constants) · [`TYPE_QUERY_IS_UNION`](#constants) · [`TYPE_QUERY_IS_POINTER`](#constants) · [`TYPE_QUERY_NAME`](#constants) · [`TYPE_QUERY_IS_SECRET`](#constants) · [`TYPE_QUERY_IS_TAG`](#constants) · [`TYPE_QUERY_IS_INTEGER`](#constants) · [`TYPE_QUERY_IS_FLOAT`](#constants) · [`TYPE_QUERY_HOLDS_SECRET`](#constants) · [`PHASE_CAP_NONE`](#constants) · [`PHASE_CAP_LOADING`](#constants) · [`PHASE_CAP_RESOLUTION`](#constants) | |

---

## NoCapabilityContext

```mach
pub rec NoCapabilityContext {}
```

the context of an evaluation that holds no phase capability

## PhaseCapabilities

```mach
pub rec PhaseCapabilities[T] {
    expression: fun(*T, *ast.Ast, ast_id.ExprId) res[ast_expr.Expr, comptime_failure.EvalFail];
    # a type node as the phase reads it, a literal head split into type and case once names are
    # resolved
    head:   fun(*T, *ast.Ast, ast_id.TypeId) res[ast_type.Type, comptime_failure.EvalFail];
    kind:   PhaseCapabilityKind;
    member: fun(*T, ast_id.ExprId) res[opt[comptime_value.CTValue], comptime_failure.EvalFail];
    type_:  fun(*T, ast_id.ExprId) res[opt[u32], comptime_failure.EvalFail];
    # the type store the field descriptors read, and the byte offset of a field, which only
    # the phase that knows layout answers
    types:  fun(*T) *type.TypeInterner;
    offset: fun(*T, u32, u32) res[opt[comptime_value.CTValue], comptime_failure.EvalFail];
    # the zero of a type, which a member an element literal leaves out holds: a scalar's value,
    # or an aggregate as a node `store` answers
    zero:   fun(*T, u32) res[comptime_value.CTValue, comptime_failure.EvalFail];
    query:  fun(*T, u32, u8) res[opt[comptime_value.CTValue], comptime_failure.EvalFail];
    layout: fun(*T, u32) res[opt[comptime_value.CTValue], comptime_failure.EvalFail];
    ident:  fun(*T, ast_id.ExprId) bool;
    # the value of the module constant a bare name names, which only sema binds on demand
    constant: fun(*T, ast_id.ExprId) res[opt[comptime_value.CTValue], comptime_failure.EvalFail];
    cast:     fun(*T, ast_id.ExprId, comptime_value.CTValue) res[comptime_value.CTValue, comptime_failure.EvalFail];
    scalar:   fun(*T, ast_id.ExprId, comptime_value.CTValue) res[comptime_value.CTValue, comptime_failure.EvalFail];
    # whether the place an expression reads is of a type that holds a secret, which only sema answers
    secret: fun(*T, ast_id.ExprId) bool;
    # the store of published constant values of the module with a stable id, which only the
    # phases that read interfaces hold
    store: fun(*T, u32) res[*comptime_deep.Store, comptime_failure.EvalFail];
}
```

### capabilities_make

```mach
pub fun capabilities_make[T](kind: PhaseCapabilityKind) PhaseCapabilities[T]
```

### read_head

```mach
pub fun read_head[T](ctx: *T, caps: PhaseCapabilities[T], a: *ast.Ast, tid: ast_id.TypeId) res[ast_type.Type, comptime_failure.EvalFail]
```

### read_expression

```mach
pub fun read_expression[T](ctx: *T, caps: PhaseCapabilities[T], a: *ast.Ast, eid: ast_id.ExprId) res[ast_expr.Expr, comptime_failure.EvalFail]
```

### no_capabilities

```mach
pub fun no_capabilities() PhaseCapabilities[NoCapabilityContext]
```

no phase capability: an expression reads literals and the constants its context binds

### loading_capabilities

```mach
pub fun loading_capabilities[T](member: fun(*T, ast_id.ExprId) res[opt[comptime_value.CTValue], comptime_failure.EvalFail],
    cast: fun(*T, ast_id.ExprId, comptime_value.CTValue) res[comptime_value.CTValue, comptime_failure.EvalFail],
    head: fun(*T, *ast.Ast, ast_id.TypeId) res[ast_type.Type, comptime_failure.EvalFail]) res[PhaseCapabilities[T], fail.Fail]
```

### resolution_capabilities

```mach
pub fun resolution_capabilities[T](ident: fun(*T, ast_id.ExprId) bool,
    expression: fun(*T, *ast.Ast, ast_id.ExprId) res[ast_expr.Expr, comptime_failure.EvalFail],
    head: fun(*T, *ast.Ast, ast_id.TypeId) res[ast_type.Type, comptime_failure.EvalFail]) res[PhaseCapabilities[T], fail.Fail]
```

### semantic_name_capabilities

```mach
pub fun semantic_name_capabilities[T](
    member: fun(*T, ast_id.ExprId) res[opt[comptime_value.CTValue], comptime_failure.EvalFail],
    constant: fun(*T, ast_id.ExprId) res[opt[comptime_value.CTValue], comptime_failure.EvalFail],
    store: fun(*T, u32) res[*comptime_deep.Store, comptime_failure.EvalFail],
    ident: fun(*T, ast_id.ExprId) bool,
    secret: fun(*T, ast_id.ExprId) bool,
    expression: fun(*T, *ast.Ast, ast_id.ExprId) res[ast_expr.Expr, comptime_failure.EvalFail],
    head: fun(*T, *ast.Ast, ast_id.TypeId) res[ast_type.Type, comptime_failure.EvalFail]) res[PhaseCapabilities[T], fail.Fail]
```

### semantic_type_capabilities

```mach
pub fun semantic_type_capabilities[T](
    member: fun(*T, ast_id.ExprId) res[opt[comptime_value.CTValue], comptime_failure.EvalFail],
    constant: fun(*T, ast_id.ExprId) res[opt[comptime_value.CTValue], comptime_failure.EvalFail],
    store: fun(*T, u32) res[*comptime_deep.Store, comptime_failure.EvalFail],
    type_: fun(*T, ast_id.ExprId) res[opt[u32], comptime_failure.EvalFail],
    types: fun(*T) *type.TypeInterner,
    offset: fun(*T, u32, u32) res[opt[comptime_value.CTValue], comptime_failure.EvalFail],
    zero: fun(*T, u32) res[comptime_value.CTValue, comptime_failure.EvalFail],
    query: fun(*T, u32, u8) res[opt[comptime_value.CTValue], comptime_failure.EvalFail],
    layout: fun(*T, u32) res[opt[comptime_value.CTValue], comptime_failure.EvalFail],
    cast: fun(*T, ast_id.ExprId, comptime_value.CTValue) res[comptime_value.CTValue, comptime_failure.EvalFail],
    scalar: fun(*T, ast_id.ExprId, comptime_value.CTValue) res[comptime_value.CTValue, comptime_failure.EvalFail],
    ident: fun(*T, ast_id.ExprId) bool,
    secret: fun(*T, ast_id.ExprId) bool,
    expression: fun(*T, *ast.Ast, ast_id.ExprId) res[ast_expr.Expr, comptime_failure.EvalFail],
    head: fun(*T, *ast.Ast, ast_id.TypeId) res[ast_type.Type, comptime_failure.EvalFail]) res[PhaseCapabilities[T], fail.Fail]
```

### lowering_capabilities

```mach
pub fun lowering_capabilities[T](
    member: fun(*T, ast_id.ExprId) res[opt[comptime_value.CTValue], comptime_failure.EvalFail],
    store: fun(*T, u32) res[*comptime_deep.Store, comptime_failure.EvalFail],
    type_: fun(*T, ast_id.ExprId) res[opt[u32], comptime_failure.EvalFail],
    types: fun(*T) *type.TypeInterner,
    offset: fun(*T, u32, u32) res[opt[comptime_value.CTValue], comptime_failure.EvalFail],
    zero: fun(*T, u32) res[comptime_value.CTValue, comptime_failure.EvalFail],
    query: fun(*T, u32, u8) res[opt[comptime_value.CTValue], comptime_failure.EvalFail],
    layout: fun(*T, u32) res[opt[comptime_value.CTValue], comptime_failure.EvalFail],
    cast: fun(*T, ast_id.ExprId, comptime_value.CTValue) res[comptime_value.CTValue, comptime_failure.EvalFail],
    scalar: fun(*T, ast_id.ExprId, comptime_value.CTValue) res[comptime_value.CTValue, comptime_failure.EvalFail],
    ident: fun(*T, ast_id.ExprId) bool,
    expression: fun(*T, *ast.Ast, ast_id.ExprId) res[ast_expr.Expr, comptime_failure.EvalFail],
    head: fun(*T, *ast.Ast, ast_id.TypeId) res[ast_type.Type, comptime_failure.EvalFail]) res[PhaseCapabilities[T], fail.Fail]
```

### capabilities_have_member

```mach
pub fun capabilities_have_member[T](ctx: *T, caps: PhaseCapabilities[T]) bool
```

### capabilities_have_ident

```mach
pub fun capabilities_have_ident[T](ctx: *T, caps: PhaseCapabilities[T]) bool
```

### capabilities_have_constant

```mach
pub fun capabilities_have_constant[T](ctx: *T, caps: PhaseCapabilities[T]) bool
```

### store_of

```mach
pub fun store_of[T](ctx: *T, caps: PhaseCapabilities[T], module: u32) res[*comptime_deep.Store, comptime_failure.EvalFail]
```

the store holding the node a value names, which only a phase that reads interfaces has

### capabilities_have_types

```mach
pub fun capabilities_have_types[T](ctx: *T, caps: PhaseCapabilities[T]) bool
```

### capabilities_lowering

```mach
pub fun capabilities_lowering[T](caps: PhaseCapabilities[T]) bool
```

whether the phase is lowering, which materializes only values an earlier phase asked for

### capabilities_cast

```mach
pub fun capabilities_cast[T](ctx: *T, caps: PhaseCapabilities[T]) bool
```

### field_of

```mach
pub fun field_of[T](ctx: *T, caps: PhaseCapabilities[T], owner: u32, index: u32, pick: u8) res[opt[comptime_value.CTValue], comptime_failure.EvalFail]
```

what a field descriptor answers about field `index` of `owner`, `pick` naming the member
asked; every answer but the offset reads the type store, and the offset is the phase's

## Functions

### parsed_expression

```mach
pub fun parsed_expression[T](ctx: *T, a: *ast.Ast, eid: ast_id.ExprId) res[ast_expr.Expr, comptime_failure.EvalFail]
```

### parsed_head

```mach
pub fun parsed_head[T](ctx: *T, a: *ast.Ast, tid: ast_id.TypeId) res[ast_type.Type, comptime_failure.EvalFail]
```

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `FIELD_SEL_NAME` | `u8` | `0` |  |
| `FIELD_SEL_TYPE` | `u8` | `1` |  |
| `FIELD_SEL_OFFSET` | `u8` | `2` |  |
| `FIELD_SEL_HAS_PAYLOAD` | `u8` | `4` |  |
| `FIELD_SEL_CODE` | `u8` | `5` |  |
| `FIELD_SEL_TYPE_BY_NAME` | `u8` | `6` |  |
| `COMPTIME_CASE_NO_PAYLOAD_MSG` | `str` | `` "this case has no payload, so it has no `.type` or `.offset`; gate on `.has_payload` first" `` |  |
| `TYPE_QUERY_IS_RECORD` | `u8` | `0` |  |
| `TYPE_QUERY_IS_UNION` | `u8` | `1` |  |
| `TYPE_QUERY_IS_POINTER` | `u8` | `2` |  |
| `TYPE_QUERY_NAME` | `u8` | `3` |  |
| `TYPE_QUERY_IS_SECRET` | `u8` | `4` |  |
| `TYPE_QUERY_IS_TAG` | `u8` | `5` |  |
| `TYPE_QUERY_IS_INTEGER` | `u8` | `6` |  |
| `TYPE_QUERY_IS_FLOAT` | `u8` | `7` |  |
| `TYPE_QUERY_HOLDS_SECRET` | `u8` | `8` |  |
| `PHASE_CAP_NONE` | `PhaseCapabilityKind` | `0` |  |
| `PHASE_CAP_LOADING` | `PhaseCapabilityKind` | `1` |  |
| `PHASE_CAP_RESOLUTION` | `PhaseCapabilityKind` | `2` |  |

