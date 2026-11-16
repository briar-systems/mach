[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **constant**

# mach.front.sema.constant

the module constants sema binds: every module `val` whose initializer evaluates at compile
time, bound once in sema's own bindings, which its result publishes to lowering and to every
importer. a constant is bound where it is first read while declarations are typed, and every
other one once they all are, before any body or instance is. the deep value of a constant,
its array, record or case literal evaluated member by member, is built in the module's store
of constant values, which its interface publishes

<sub>source: [`src/front/sema/constant.mach`](../../../../src/front/sema/constant.mach) · imported by 4 modules</sub>

<sub>`sema_context` is [`mach.front.sema.context`](context.md) · `ast_id` is [`mach.front.ast.id`](../ast/id.md) · `comptime_value` is [`mach.front.comptime.value`](../comptime/value.md) · `comptime_failure` is [`mach.front.comptime.failure`](../comptime/failure.md) · `fail` is [`mach.base.fail`](../../base/fail.md) · `resolve` is [`mach.front.resolve`](../resolve.md) · `comptime_deep` is [`mach.front.comptime.deep`](../comptime/deep.md) · `type_kind` is [`mach.type.kind`](../../type/kind.md)</sub>

| Functions | |
| :--- | :--- |
| [`named`](#named) | the value of the module constant the bare name `eid` names: one of this module's, bound on first read, or one an imported module published |
| [`member`](#member) | the value of the module constant a path names, which for a path through a module is the imported constant itself |
| [`bind`](#bind) | binds the module `val` `did` declares when its initializer is a compile-time constant |
| [`rejected`](#rejected) | whether the evaluator rejected the initializer of the module `val` `did` when it was bound |
| [`unevaluable`](#unevaluable) | whether the initializer of the module `val` `did` was no constant when it was bound |
| [`settle`](#settle) | every constant still unbound once all are tried is none, so no later read, inside a body or an instance, evaluates one again |
| [`own_value`](#own_value) | the node of the value the module `val` `sym` declares, built in the module's store on first ask: the constant sema bound, or its initializer evaluated member by member; NONE for a declaration with no initializer of its own |
| [`array_value`](#array_value) | the value of the array `sym` names that a `$each` reads: a module constant's, this module's built once and another module's as it published it, or the value of a `val` local to a body, built in the walk that reads it since each instance of the body may give it another; none for a binding with no array, record or case literal value |
| [`store_of`](#store_of) | the store of constant values of the module with stable id `module`: this module's, or the one another module's interface publishes |
| [`is_aggregate`](#is_aggregate) | an array literal, or a record or case literal spelled with its head type |
| [`type_at`](#type_at) | the type the walk gave an expression |
| [`node_build`](#node_build) | the node of the expression `eid`: an aggregate literal member by member, the value of the constant it names, an address, or anything else as the evaluator answers it at the type the walk gave it |
| [`zero_of`](#zero_of) | the zero of the type `ty` as a value, for an evaluation that holds a type and no expression |

| Constants | |
| :--- | :--- |
| [`CONSTANT_PENDING`](#constants) | pending: not bound yet, and tried again on the next read; none: no constant, for good; refused: no constant, for good, since the evaluator rejected its initializer; unevaluable: no constant, for good, since its initializer needs a value no constant gives |

---

## Functions

### named

```mach
pub fun named(sc: *sema_context.SemaContext, eid: ast_id.ExprId) res[opt[comptime_value.CTValue], comptime_failure.EvalFail]
```

the value of the module constant the bare name `eid` names: one of this module's, bound on
first read, or one an imported module published

### member

```mach
pub fun member(sc: *sema_context.SemaContext, eid: ast_id.ExprId) res[opt[comptime_value.CTValue], comptime_failure.EvalFail]
```

the value of the module constant a path names, which for a path through a module is the
imported constant itself

### bind

```mach
pub fun bind(sc: *sema_context.SemaContext, did: ast_id.DeclId) err[fail.Fail]
```

binds the module `val` `did` declares when its initializer is a compile-time constant. this
is the last try, so an initializer the evaluator rejects is marked refused, which its
published value reports, and one that is no constant is marked unevaluable

### rejected

```mach
pub fun rejected(sc: *sema_context.SemaContext, did: ast_id.DeclId) bool
```

whether the evaluator rejected the initializer of the module `val` `did` when it was bound

### unevaluable

```mach
pub fun unevaluable(sc: *sema_context.SemaContext, did: ast_id.DeclId) bool
```

whether the initializer of the module `val` `did` was no constant when it was bound

### settle

```mach
pub fun settle(sc: *sema_context.SemaContext)
```

every constant still unbound once all are tried is none, so no later read, inside a body or
an instance, evaluates one again

### own_value

```mach
pub fun own_value(sc: *sema_context.SemaContext, sym: *resolve.Symbol) res[u32, fail.Fail]
```

the node of the value the module `val` `sym` declares, built in the module's store on first
ask: the constant sema bound, or its initializer evaluated member by member; NONE for a
declaration with no initializer of its own

### array_value

```mach
pub fun array_value(sc: *sema_context.SemaContext, sym: *resolve.Symbol) res[opt[comptime_value.CTValue], fail.Fail]
```

the value of the array `sym` names that a `$each` reads: a module constant's, this module's
built once and another module's as it published it, or the value of a `val` local to a body,
built in the walk that reads it since each instance of the body may give it another; none for
a binding with no array, record or case literal value

### store_of

```mach
pub fun store_of(sc: *sema_context.SemaContext, module_index: u32) res[*comptime_deep.Store, comptime_failure.EvalFail]
```

the store of constant values of the module with stable id `module`: this module's, or the one
another module's interface publishes

### is_aggregate

```mach
pub fun is_aggregate(sc: *sema_context.SemaContext, eid: ast_id.ExprId) bool
```

an array literal, or a record or case literal spelled with its head type

### type_at

```mach
pub fun type_at(sc: *sema_context.SemaContext, eid: ast_id.ExprId) type_kind.TypeId
```

the type the walk gave an expression

### node_build

```mach
pub fun node_build(sc: *sema_context.SemaContext, eid: ast_id.ExprId) res[u32, fail.Fail]
```

the node of the expression `eid`: an aggregate literal member by member, the value of the
constant it names, an address, or anything else as the evaluator answers it at the type the
walk gave it

### zero_of

```mach
pub fun zero_of(sc: *sema_context.SemaContext, ty: u32) res[comptime_value.CTValue, comptime_failure.EvalFail]
```

the zero of the type `ty` as a value, for an evaluation that holds a type and no expression

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `CONSTANT_PENDING` | `u8` | `0` | pending: not bound yet, and tried again on the next read; none: no constant, for good; refused: no constant, for good, since the evaluator rejected its initializer; unevaluable: no constant, for good, since its initializer needs a value no constant gives |

