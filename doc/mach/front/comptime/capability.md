[mach](../../README.md) / [front](../README.md) / [comptime](README.md) / **capability**

# mach.front.comptime.capability

what each phase lets a comptime evaluation ask of it: members, identifiers, types,
fields and layout, published as one record per phase

Source: [`src/front/comptime/capability.mach`](../../../../src/front/comptime/capability.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.front.ast`](../ast.md), [`mach.front.ast.expr`](../ast/expr.md), [`mach.front.ast.id`](../ast/id.md), [`mach.front.ast.type`](../ast/type.md), [`mach.front.comptime.deep`](deep.md), [`mach.front.comptime.failure`](failure.md), [`mach.front.comptime.value`](value.md), [`mach.type`](../../type.md).

| Type | | |
| :--- | :--- | :--- |
| [`NameKind`](#namekind) | `def` | what an identifier an evaluation reads resolved to: no symbol, a run-time binding, a comptime parameter or `$each` loop variable, which a frame binds, or a declaration |
| [`NoCapabilityContext`](#nocapabilitycontext) | `rec` | the context of an evaluation that holds no phase capability |
| [`PhaseCapabilities`](#phasecapabilities) | `rec` | the questions one phase answers for a comptime evaluation, each a function over the phase's context `T`, nil for a question the phase cannot answer |

| On `NameKind` | |
| :--- | :--- |
| [`NAME_NONE`](#namekind), [`NAME_RUNTIME`](#namekind), [`NAME_LOCAL`](#namekind), [`NAME_DECLARED`](#namekind) | |

| On `PhaseCapabilities` | |
| :--- | :--- |
| [`capabilities_make`](#capabilities_make) | a capability set of phase `kind` that answers only from the parsed tree, every other question nil |
| [`read_head`](#read_head) | a type node as the active phase reads it |
| [`read_expression`](#read_expression) | an expression node as the active phase reads it |
| [`no_capabilities`](#no_capabilities) | no phase capability: an expression reads literals and the constants its context binds |
| [`loading_capabilities`](#loading_capabilities) | the capabilities of the load: module members, casts to the integer a def chain names, and type heads |
| [`resolution_capabilities`](#resolution_capabilities) | the capabilities of name resolution: what each name an evaluation reads resolved to, and expressions and type heads as resolution reads them |
| [`semantic_name_capabilities`](#semantic_name_capabilities) | the capabilities of sema before types are known: members, module constants, published stores, what each name resolved to, secret places, and expressions and type heads |
| [`semantic_type_capabilities`](#semantic_type_capabilities) | the capabilities of sema once types are known: every question sema answers, layout included |
| [`lowering_capabilities`](#lowering_capabilities) | the capabilities of lowering: every question of typed sema except module constants and secret places |
| [`capabilities_have_member`](#capabilities_have_member) | whether the phase answers module members |
| [`capabilities_have_ident`](#capabilities_have_ident) | whether the phase answers which bare names bind run-time values |
| [`capabilities_have_constant`](#capabilities_have_constant) | whether the phase answers the values of module constants |
| [`store_of`](#store_of) | the store holding the node a value names, which only a phase that reads interfaces has |
| [`capabilities_have_types`](#capabilities_have_types) | whether the phase answers types and layout |
| [`capabilities_lowering`](#capabilities_lowering) | whether the phase is lowering, which materializes only values an earlier phase asked for |
| [`capabilities_cast`](#capabilities_cast) | whether the phase converts cast operands |
| [`field_of`](#field_of) | what a field descriptor answers about field `index` of `owner`, `pick` naming the member asked; every answer but the offset reads the type store, and the offset is the phase's |

| Functions | |
| :--- | :--- |
| [`parsed_expression`](#parsed_expression) | an expression node as the parse made it |
| [`parsed_head`](#parsed_head) | a type node as the parse made it |

| Constants | |
| :--- | :--- |
| [`FIELD_SEL_NAME`](#constants), [`FIELD_SEL_TYPE`](#constants), [`FIELD_SEL_OFFSET`](#constants), [`FIELD_SEL_HAS_PAYLOAD`](#constants), [`FIELD_SEL_CODE`](#constants), [`FIELD_SEL_TYPE_BY_NAME`](#constants), [`COMPTIME_CASE_NO_PAYLOAD_MSG`](#constants), [`TYPE_QUERY_IS_RECORD`](#constants), [`TYPE_QUERY_IS_UNION`](#constants), [`TYPE_QUERY_IS_POINTER`](#constants), [`TYPE_QUERY_NAME`](#constants), [`TYPE_QUERY_IS_SECRET`](#constants), [`TYPE_QUERY_IS_TAG`](#constants), [`TYPE_QUERY_IS_INTEGER`](#constants), [`TYPE_QUERY_IS_FLOAT`](#constants), [`TYPE_QUERY_HOLDS_SECRET`](#constants), [`PHASE_CAP_NONE`](#constants), [`PHASE_CAP_LOADING`](#constants), [`PHASE_CAP_RESOLUTION`](#constants) | |

---

## NameKind

```mach
pub def NameKind: u8
```

what an identifier an evaluation reads resolved to: no symbol, a run-time binding, a comptime
parameter or `$each` loop variable, which a frame binds, or a declaration

| Name | Value | |
| :--- | :--- | :--- |
| `NAME_NONE` | `0` |  |
| `NAME_RUNTIME` | `1` |  |
| `NAME_LOCAL` | `2` |  |
| `NAME_DECLARED` | `3` |  |

## NoCapabilityContext

```mach
pub rec NoCapabilityContext {}
```

the context of an evaluation that holds no phase capability

## PhaseCapabilities

```mach
pub rec PhaseCapabilities[T] {
    expression: fun(*T, *mach.front.ast.Ast, mach.front.ast.id.ExprId) res[mach.front.ast.expr.Expr, mach.front.comptime.failure.EvalFail];
    # a type node as the phase reads it, a literal head split into type and case once names are
    # resolved
    head:   fun(*T, *mach.front.ast.Ast, mach.front.ast.id.TypeId) res[mach.front.ast.type.Type, mach.front.comptime.failure.EvalFail];
    kind:   PhaseCapabilityKind;
    member: fun(*T, mach.front.ast.id.ExprId) res[opt[mach.front.comptime.value.CTValue], mach.front.comptime.failure.EvalFail];
    type_:  fun(*T, mach.front.ast.id.ExprId) res[opt[u32], mach.front.comptime.failure.EvalFail];
    # the type store the field descriptors read, and the byte offset of a field, which only
    # the phase that knows layout answers
    types:  fun(*T) *mach.type.TypeInterner;
    offset: fun(*T, u32, u32) res[opt[mach.front.comptime.value.CTValue], mach.front.comptime.failure.EvalFail];
    # the zero of a type, which a member an element literal leaves out holds: a scalar's value,
    # or an aggregate as a node `store` answers
    zero:   fun(*T, u32) res[mach.front.comptime.value.CTValue, mach.front.comptime.failure.EvalFail];
    query:  fun(*T, u32, u8) res[opt[mach.front.comptime.value.CTValue], mach.front.comptime.failure.EvalFail];
    layout: fun(*T, u32) res[opt[mach.front.comptime.value.CTValue], mach.front.comptime.failure.EvalFail];
    ident:  fun(*T, mach.front.ast.id.ExprId) NameKind;
    # the value of the module constant a bare name names, which only sema binds on demand
    constant: fun(*T, mach.front.ast.id.ExprId) res[opt[mach.front.comptime.value.CTValue], mach.front.comptime.failure.EvalFail];
    cast:     fun(*T, mach.front.ast.id.ExprId, mach.front.comptime.value.CTValue) res[mach.front.comptime.value.CTValue, mach.front.comptime.failure.EvalFail];
    scalar:   fun(*T, mach.front.ast.id.ExprId, mach.front.comptime.value.CTValue) res[mach.front.comptime.value.CTValue, mach.front.comptime.failure.EvalFail];
    # whether the place an expression reads is of a type that holds a secret, which only sema answers
    secret: fun(*T, mach.front.ast.id.ExprId) bool;
    # the store of published constant values of the module with a stable id, which only the
    # phases that read interfaces hold
    store: fun(*T, u32) res[*mach.front.comptime.deep.Store, mach.front.comptime.failure.EvalFail];
}
```

the questions one phase answers for a comptime evaluation, each a function over the phase's
context `T`, nil for a question the phase cannot answer. `kind` says which phase it is, and
only the set each kind publishes is complete

| Field | Type | |
| :--- | :--- | :--- |
| `expression` | `fun(*T, *mach.front.ast.Ast, mach.front.ast.id.ExprId) res[mach.front.ast.expr.Expr, mach.front.comptime.failure.EvalFail]` | an expression node as the phase reads it |
| `kind` | `PhaseCapabilityKind` | the phase, one of PHASE_CAP_ |
| `member` | `fun(*T, mach.front.ast.id.ExprId) res[opt[mach.front.comptime.value.CTValue], mach.front.comptime.failure.EvalFail]` | the value of a module member `m.name`, none when the member names no constant |
| `type_` | `fun(*T, mach.front.ast.id.ExprId) res[opt[u32], mach.front.comptime.failure.EvalFail]` | the type id a type operand names |
| `query` | `fun(*T, u32, u8) res[opt[mach.front.comptime.value.CTValue], mach.front.comptime.failure.EvalFail]` | the answer to a TYPE_QUERY_ about a type id |
| `layout` | `fun(*T, u32) res[opt[mach.front.comptime.value.CTValue], mach.front.comptime.failure.EvalFail]` | the value of a layout intrinsic at an expression |
| `ident` | `fun(*T, mach.front.ast.id.ExprId) NameKind` | whether a bare name binds a run-time value, which makes it no constant |
| `cast` | `fun(*T, mach.front.ast.id.ExprId, mach.front.comptime.value.CTValue) res[mach.front.comptime.value.CTValue, mach.front.comptime.failure.EvalFail]` | a value converted by the cast expression it is the operand of |
| `scalar` | `fun(*T, mach.front.ast.id.ExprId, mach.front.comptime.value.CTValue) res[mach.front.comptime.value.CTValue, mach.front.comptime.failure.EvalFail]` | a value fit to the scalar type the phase gave an expression |

### capabilities_make

```mach
pub fun capabilities_make[T](kind: PhaseCapabilityKind) PhaseCapabilities[T]
```

a capability set of phase `kind` that answers only from the parsed tree, every other question nil

| Parameter | |
| :--- | :--- |
| `kind` | the phase |
| **returns** | the set, which a publisher fills in and checks |

### read_head

```mach
pub fun read_head[T](ctx: *T, caps: PhaseCapabilities[T], a: *mach.front.ast.Ast, tid: mach.front.ast.id.TypeId) res[mach.front.ast.type.Type, mach.front.comptime.failure.EvalFail]
```

a type node as the active phase reads it

| Parameter | |
| :--- | :--- |
| `ctx` | the phase context, nil only for a set of no phase |
| `caps` | the phase's capabilities |
| `a` | the tree |
| `tid` | the type node |
| **returns** | the node, or an internal failure when the set is incomplete or its context missing |

### read_expression

```mach
pub fun read_expression[T](ctx: *T, caps: PhaseCapabilities[T], a: *mach.front.ast.Ast, eid: mach.front.ast.id.ExprId) res[mach.front.ast.expr.Expr, mach.front.comptime.failure.EvalFail]
```

an expression node as the active phase reads it

| Parameter | |
| :--- | :--- |
| `ctx` | the phase context, nil only for a set of no phase |
| `caps` | the phase's capabilities |
| `a` | the tree |
| `eid` | the expression |
| **returns** | the node, or an internal failure when the set is incomplete or its context missing |

### no_capabilities

```mach
pub fun no_capabilities() PhaseCapabilities[NoCapabilityContext]
```

no phase capability: an expression reads literals and the constants its context binds

### loading_capabilities

```mach
pub fun loading_capabilities[T](member: fun(*T, mach.front.ast.id.ExprId) res[opt[mach.front.comptime.value.CTValue], mach.front.comptime.failure.EvalFail],
    cast: fun(*T, mach.front.ast.id.ExprId, mach.front.comptime.value.CTValue) res[mach.front.comptime.value.CTValue, mach.front.comptime.failure.EvalFail],
    head: fun(*T, *mach.front.ast.Ast, mach.front.ast.id.TypeId) res[mach.front.ast.type.Type, mach.front.comptime.failure.EvalFail]) res[PhaseCapabilities[T], mach.base.fail.Fail]
```

the capabilities of the load: module members, casts to the integer a def chain names, and type
heads

| Parameter | |
| :--- | :--- |
| **returns** | the checked set, or a failure when it is incomplete or contradictory |

### resolution_capabilities

```mach
pub fun resolution_capabilities[T](ident: fun(*T, mach.front.ast.id.ExprId) NameKind,
    expression: fun(*T, *mach.front.ast.Ast, mach.front.ast.id.ExprId) res[mach.front.ast.expr.Expr, mach.front.comptime.failure.EvalFail],
    head: fun(*T, *mach.front.ast.Ast, mach.front.ast.id.TypeId) res[mach.front.ast.type.Type, mach.front.comptime.failure.EvalFail]) res[PhaseCapabilities[T], mach.base.fail.Fail]
```

the capabilities of name resolution: what each name an evaluation reads resolved to, and
expressions and type heads as resolution reads them

| Parameter | |
| :--- | :--- |
| **returns** | the checked set, or a failure when it is incomplete or contradictory |

### semantic_name_capabilities

```mach
pub fun semantic_name_capabilities[T](
    member: fun(*T, mach.front.ast.id.ExprId) res[opt[mach.front.comptime.value.CTValue], mach.front.comptime.failure.EvalFail],
    constant: fun(*T, mach.front.ast.id.ExprId) res[opt[mach.front.comptime.value.CTValue], mach.front.comptime.failure.EvalFail],
    store: fun(*T, u32) res[*mach.front.comptime.deep.Store, mach.front.comptime.failure.EvalFail],
    ident: fun(*T, mach.front.ast.id.ExprId) NameKind,
    secret: fun(*T, mach.front.ast.id.ExprId) bool,
    expression: fun(*T, *mach.front.ast.Ast, mach.front.ast.id.ExprId) res[mach.front.ast.expr.Expr, mach.front.comptime.failure.EvalFail],
    head: fun(*T, *mach.front.ast.Ast, mach.front.ast.id.TypeId) res[mach.front.ast.type.Type, mach.front.comptime.failure.EvalFail]) res[PhaseCapabilities[T], mach.base.fail.Fail]
```

the capabilities of sema before types are known: members, module constants, published stores,
what each name resolved to, secret places, and expressions and type heads

| Parameter | |
| :--- | :--- |
| **returns** | the checked set, or a failure when it is incomplete or contradictory |

### semantic_type_capabilities

```mach
pub fun semantic_type_capabilities[T](
    member: fun(*T, mach.front.ast.id.ExprId) res[opt[mach.front.comptime.value.CTValue], mach.front.comptime.failure.EvalFail],
    constant: fun(*T, mach.front.ast.id.ExprId) res[opt[mach.front.comptime.value.CTValue], mach.front.comptime.failure.EvalFail],
    store: fun(*T, u32) res[*mach.front.comptime.deep.Store, mach.front.comptime.failure.EvalFail],
    type_: fun(*T, mach.front.ast.id.ExprId) res[opt[u32], mach.front.comptime.failure.EvalFail],
    types: fun(*T) *mach.type.TypeInterner,
    offset: fun(*T, u32, u32) res[opt[mach.front.comptime.value.CTValue], mach.front.comptime.failure.EvalFail],
    zero: fun(*T, u32) res[mach.front.comptime.value.CTValue, mach.front.comptime.failure.EvalFail],
    query: fun(*T, u32, u8) res[opt[mach.front.comptime.value.CTValue], mach.front.comptime.failure.EvalFail],
    layout: fun(*T, u32) res[opt[mach.front.comptime.value.CTValue], mach.front.comptime.failure.EvalFail],
    cast: fun(*T, mach.front.ast.id.ExprId, mach.front.comptime.value.CTValue) res[mach.front.comptime.value.CTValue, mach.front.comptime.failure.EvalFail],
    scalar: fun(*T, mach.front.ast.id.ExprId, mach.front.comptime.value.CTValue) res[mach.front.comptime.value.CTValue, mach.front.comptime.failure.EvalFail],
    ident: fun(*T, mach.front.ast.id.ExprId) NameKind,
    secret: fun(*T, mach.front.ast.id.ExprId) bool,
    expression: fun(*T, *mach.front.ast.Ast, mach.front.ast.id.ExprId) res[mach.front.ast.expr.Expr, mach.front.comptime.failure.EvalFail],
    head: fun(*T, *mach.front.ast.Ast, mach.front.ast.id.TypeId) res[mach.front.ast.type.Type, mach.front.comptime.failure.EvalFail]) res[PhaseCapabilities[T], mach.base.fail.Fail]
```

the capabilities of sema once types are known: every question sema answers, layout included

| Parameter | |
| :--- | :--- |
| **returns** | the checked set, or a failure when it is incomplete or contradictory |

### lowering_capabilities

```mach
pub fun lowering_capabilities[T](
    member: fun(*T, mach.front.ast.id.ExprId) res[opt[mach.front.comptime.value.CTValue], mach.front.comptime.failure.EvalFail],
    store: fun(*T, u32) res[*mach.front.comptime.deep.Store, mach.front.comptime.failure.EvalFail],
    type_: fun(*T, mach.front.ast.id.ExprId) res[opt[u32], mach.front.comptime.failure.EvalFail],
    types: fun(*T) *mach.type.TypeInterner,
    offset: fun(*T, u32, u32) res[opt[mach.front.comptime.value.CTValue], mach.front.comptime.failure.EvalFail],
    zero: fun(*T, u32) res[mach.front.comptime.value.CTValue, mach.front.comptime.failure.EvalFail],
    query: fun(*T, u32, u8) res[opt[mach.front.comptime.value.CTValue], mach.front.comptime.failure.EvalFail],
    layout: fun(*T, u32) res[opt[mach.front.comptime.value.CTValue], mach.front.comptime.failure.EvalFail],
    cast: fun(*T, mach.front.ast.id.ExprId, mach.front.comptime.value.CTValue) res[mach.front.comptime.value.CTValue, mach.front.comptime.failure.EvalFail],
    scalar: fun(*T, mach.front.ast.id.ExprId, mach.front.comptime.value.CTValue) res[mach.front.comptime.value.CTValue, mach.front.comptime.failure.EvalFail],
    ident: fun(*T, mach.front.ast.id.ExprId) NameKind,
    expression: fun(*T, *mach.front.ast.Ast, mach.front.ast.id.ExprId) res[mach.front.ast.expr.Expr, mach.front.comptime.failure.EvalFail],
    head: fun(*T, *mach.front.ast.Ast, mach.front.ast.id.TypeId) res[mach.front.ast.type.Type, mach.front.comptime.failure.EvalFail]) res[PhaseCapabilities[T], mach.base.fail.Fail]
```

the capabilities of lowering: every question of typed sema except module constants and secret places

| Parameter | |
| :--- | :--- |
| **returns** | the checked set, or a failure when it is incomplete or contradictory |

### capabilities_have_member

```mach
pub fun capabilities_have_member[T](ctx: *T, caps: PhaseCapabilities[T]) bool
```

whether the phase answers module members

| Parameter | |
| :--- | :--- |
| `ctx` | the phase context |
| `caps` | the phase's capabilities |
| **returns** | true for the load, sema and lowering, false for a nil context or an incomplete set |

### capabilities_have_ident

```mach
pub fun capabilities_have_ident[T](ctx: *T, caps: PhaseCapabilities[T]) bool
```

whether the phase answers which bare names bind run-time values

| Parameter | |
| :--- | :--- |
| `ctx` | the phase context |
| `caps` | the phase's capabilities |
| **returns** | true for resolution, sema and lowering, false for a nil context or an incomplete set |

### capabilities_have_constant

```mach
pub fun capabilities_have_constant[T](ctx: *T, caps: PhaseCapabilities[T]) bool
```

whether the phase answers the values of module constants

| Parameter | |
| :--- | :--- |
| `ctx` | the phase context |
| `caps` | the phase's capabilities |
| **returns** | true for sema, false for a nil context or an incomplete set |

### store_of

```mach
pub fun store_of[T](ctx: *T, caps: PhaseCapabilities[T], module: u32) res[*mach.front.comptime.deep.Store, mach.front.comptime.failure.EvalFail]
```

the store holding the node a value names, which only a phase that reads interfaces has

### capabilities_have_types

```mach
pub fun capabilities_have_types[T](ctx: *T, caps: PhaseCapabilities[T]) bool
```

whether the phase answers types and layout

| Parameter | |
| :--- | :--- |
| `ctx` | the phase context |
| `caps` | the phase's capabilities |
| **returns** | true for typed sema and lowering, false for a nil context or an incomplete set |

### capabilities_lowering

```mach
pub fun capabilities_lowering[T](caps: PhaseCapabilities[T]) bool
```

whether the phase is lowering, which materializes only values an earlier phase asked for

### capabilities_cast

```mach
pub fun capabilities_cast[T](ctx: *T, caps: PhaseCapabilities[T]) bool
```

whether the phase converts cast operands

| Parameter | |
| :--- | :--- |
| `ctx` | the phase context |
| `caps` | the phase's capabilities |
| **returns** | true for the load, typed sema and lowering, false for a nil context or an incomplete set |

### field_of

```mach
pub fun field_of[T](ctx: *T, caps: PhaseCapabilities[T], owner: u32, index: u32, pick: u8) res[opt[mach.front.comptime.value.CTValue], mach.front.comptime.failure.EvalFail]
```

what a field descriptor answers about field `index` of `owner`, `pick` naming the member
asked; every answer but the offset reads the type store, and the offset is the phase's

## Functions

### parsed_expression

```mach
pub fun parsed_expression[T](ctx: *T, a: *mach.front.ast.Ast, eid: mach.front.ast.id.ExprId) res[mach.front.ast.expr.Expr, mach.front.comptime.failure.EvalFail]
```

an expression node as the parse made it

| Parameter | |
| :--- | :--- |
| `ctx` | the phase context, unused |
| `a` | the tree |
| `eid` | the expression |
| **returns** | a copy of the node, or an internal failure when the tree is nil or the id out of range |

### parsed_head

```mach
pub fun parsed_head[T](ctx: *T, a: *mach.front.ast.Ast, tid: mach.front.ast.id.TypeId) res[mach.front.ast.type.Type, mach.front.comptime.failure.EvalFail]
```

a type node as the parse made it

| Parameter | |
| :--- | :--- |
| `ctx` | the phase context, unused |
| `a` | the tree |
| `tid` | the type node |
| **returns** | a copy of the node, or an internal failure when the tree is nil or the id out of range |

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `FIELD_SEL_NAME` | `u8` | `0` |  |
| `FIELD_SEL_TYPE` | `u8` | `1` |  |
| `FIELD_SEL_OFFSET` | `u8` | `2` |  |
| `FIELD_SEL_HAS_PAYLOAD` | `u8` | `4` |  |
| `FIELD_SEL_CODE` | `u8` | `5` |  |
| `FIELD_SEL_TYPE_BY_NAME` | `u8` | `6` |  |
| `COMPTIME_CASE_NO_PAYLOAD_MSG` | `str` | `` "this case has no payload, so it has no `.type` or `.offset`. gate on `.has_payload` first" `` |  |
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

