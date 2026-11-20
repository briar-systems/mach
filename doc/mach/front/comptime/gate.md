[mach](../../README.md) / [front](../README.md) / [comptime](README.md) / **gate**

# mach.front.comptime.gate

comptime gates: which later phase a gate's condition depends on, and the verdict that
selects or discards its arm

Source: [`src/front/comptime/gate.mach`](../../../../src/front/comptime/gate.mach)

Uses types from [`mach.base.diagnostic.kind`](../../base/diagnostic/kind.md), [`mach.base.fail`](../../base/fail.md), [`mach.base.intern`](../../base/intern.md), [`mach.base.source`](../../base/source.md), [`mach.front.ast`](../ast.md), [`mach.front.ast.expr`](../ast/expr.md), [`mach.front.ast.id`](../ast/id.md), [`mach.front.comptime.capability`](capability.md), [`mach.front.comptime.failure`](failure.md), [`mach.front.comptime.scope`](scope.md), [`mach.front.comptime.value`](value.md).

| Type | | |
| :--- | :--- | :--- |
| [`GateOutcome`](#gateoutcome) | `def` |  |
| [`GateProbe`](#gateprobe) | `def` |  |
| [`GateNodeVerdict`](#gatenodeverdict) | `def` |  |
| [`GateProbes`](#gateprobes) | `rec` | the observers a dependency probe asks of the phase walking a gate, each over the observer `T` |
| [`GateScope`](#gatescope) | `def` |  |
| [`GateVerdict`](#gateverdict) | `rec` | what deciding one gate gave |

| On `GateOutcome` | |
| :--- | :--- |
| [`GATE_INACTIVE`](#gateoutcome), [`GATE_ACTIVE`](#gateoutcome), [`GATE_REJECTED`](#gateoutcome), [`GATE_FAILED`](#gateoutcome), [`GATE_AWAITING_PHASE`](#gateoutcome), [`GATE_AWAITING_INSTANCE`](#gateoutcome) | |

| On `GateProbe` | |
| :--- | :--- |
| [`GATE_PROBE_COMPTIME_PARAM`](#gateprobe), [`GATE_PROBE_TYPE_COMPARISON`](#gateprobe) | |

| On `GateNodeVerdict` | |
| :--- | :--- |
| [`GATE_NODE_NO`](#gatenodeverdict), [`GATE_NODE_PRUNE`](#gatenodeverdict) | |

| On `GateProbes` | |
| :--- | :--- |
| [`gate_probes`](#gate_probes) | the observers of the dependency probes, with no node action |
| [`gate_chain_depends_on`](#gate_chain_depends_on) | whether any arm condition of a `$if` chain depends on what `probe` looks for |
| [`gate_chain_defers`](#gate_chain_defers) | whether any arm condition of a `$if` chain depends on something only a later phase knows: a `$` parameter, a layout intrinsic, a type comparison, a field descriptor or a `$each` variable |
| [`gate_depends_on`](#gate_depends_on) | whether the expression `eid` or any expression under it depends on what `probe` looks for |

| On `GateScope` | |
| :--- | :--- |
| [`GATE_SCOPE_DECL`](#gatescope), [`GATE_SCOPE_INSTANCE`](#gatescope) | |
| [`evaluate_gate`](#evaluate_gate) | decide a `$if` gate in the phase `caps` describes a nil condition, a bare `$or`, is active |

| Functions | |
| :--- | :--- |
| [`comptime_if_declares_nothing`](#comptime_if_declares_nothing) | whether every arm of a chain, at any depth, declares nothing but directives and chains; a chain nested deeper than the stack lets the walk descend is refused (`nesting`) |
| [`nesting`](#nesting) | the refusal of a walk over nesting deeper than the stack lets it descend, which the phase that asked reports against the node it asked about |
| [`gate_walk`](#gate_walk) | walk the condition `eid` with `action` deciding each node: a node it says yes to ends the walk, one it prunes is not descended |
| [`never_decided_message`](#never_decided_message) | what a failed gate says when the name it waits on is a constant nowhere the module can see, which the caller frees |
| [`type_question`](#type_question) | the first call in an expression that asks a type question, EXPR_NIL when none does |
| [`type_question_message`](#type_question_message) | a declaring gate may not ask a type question; `through` names the constants it asks it through, as ` through `T` -> `S``, or is empty when the gate asks it itself |

---

## GateOutcome

```mach
pub def GateOutcome: u8
```

| Name | Value | |
| :--- | :--- | :--- |
| `GATE_INACTIVE` | `0` |  |
| `GATE_ACTIVE` | `1` |  |
| `GATE_REJECTED` | `2` |  |
| `GATE_FAILED` | `3` |  |
| `GATE_AWAITING_PHASE` | `4` |  |
| `GATE_AWAITING_INSTANCE` | `5` |  |

## GateProbe

```mach
pub def GateProbe: u8
```

| Name | Value | |
| :--- | :--- | :--- |
| `GATE_PROBE_COMPTIME_PARAM` | `0` |  |
| `GATE_PROBE_TYPE_COMPARISON` | `2` |  |

## GateNodeVerdict

```mach
pub def GateNodeVerdict: u8
```

| Name | Value | |
| :--- | :--- | :--- |
| `GATE_NODE_NO` | `0` |  |
| `GATE_NODE_PRUNE` | `2` |  |

## GateProbes

```mach
pub rec GateProbes[T] {
    expression:         fun(*T, *mach.front.ast.Ast, mach.front.ast.id.ExprId) res[mach.front.ast.expr.Expr, mach.front.comptime.failure.EvalFail];
    comptime_param:     fun(*T, mach.front.ast.id.ExprId) bool;
    each_loopvar:       fun(*T, mach.front.ast.id.ExprId) bool;
    field_loopvar:      fun(*T, mach.front.ast.id.ExprId) bool;
    field_type_operand: fun(*T, mach.front.ast.id.ExprId) res[bool, mach.base.fail.Fail];
    node_action:        fun(*T, mach.front.ast.id.ExprId, *mach.front.ast.expr.Expr) res[GateNodeVerdict, mach.base.fail.Fail];
}
```

the observers a dependency probe asks of the phase walking a gate, each over the observer `T`

| Field | Type | |
| :--- | :--- | :--- |
| `expression` | `fun(*T, *mach.front.ast.Ast, mach.front.ast.id.ExprId) res[mach.front.ast.expr.Expr, mach.front.comptime.failure.EvalFail]` | an expression node as the phase reads it |
| `comptime_param` | `fun(*T, mach.front.ast.id.ExprId) bool` | whether an identifier names a `$` parameter |
| `each_loopvar` | `fun(*T, mach.front.ast.id.ExprId) bool` | whether an identifier names a `$each` loop variable |
| `field_loopvar` | `fun(*T, mach.front.ast.id.ExprId) bool` | whether a field descriptor member reads a `$each` variable over `$fields` |
| `field_type_operand` | `fun(*T, mach.front.ast.id.ExprId) res[bool, mach.base.fail.Fail]` | whether a comparison operand is a field descriptor's type |
| `node_action` | `fun(*T, mach.front.ast.id.ExprId, *mach.front.ast.expr.Expr) res[GateNodeVerdict, mach.base.fail.Fail]` | the visit of a plain walk, which decides each node itself, nil for a probe |

### gate_probes

```mach
pub fun gate_probes[T](
    comptime_param: fun(*T, mach.front.ast.id.ExprId) bool,
    each_loopvar: fun(*T, mach.front.ast.id.ExprId) bool,
    field_loopvar: fun(*T, mach.front.ast.id.ExprId) bool,
    field_type_operand: fun(*T, mach.front.ast.id.ExprId) res[bool, mach.base.fail.Fail],
    expression: fun(*T, *mach.front.ast.Ast, mach.front.ast.id.ExprId) res[mach.front.ast.expr.Expr, mach.front.comptime.failure.EvalFail]) GateProbes[T]
```

the observers of the dependency probes, with no node action

| Parameter | |
| :--- | :--- |
| **returns** | the observer set |

### gate_chain_depends_on

```mach
pub fun gate_chain_depends_on[T](
    a: *mach.front.ast.Ast, source: str, obs: *T, probes: GateProbes[T], probe: GateProbe,
    branches_start: u32, branches_len: u32) res[bool, mach.base.fail.Fail]
```

whether any arm condition of a `$if` chain depends on what `probe` looks for

| Parameter | |
| :--- | :--- |
| `a` | the tree |
| `source` | the tree's source text |
| `obs` | the observer the probes ask |
| `probes` | the observers |
| `probe` | which dependency to look for |
| `branches_start` | first arm in the comptime branch table |
| `branches_len` | how many arms |
| **returns** | whether a condition depends on it, or the failure of the walk |

### gate_chain_defers

```mach
pub fun gate_chain_defers[T](
    a: *mach.front.ast.Ast, source: str, obs: *T, probes: GateProbes[T],
    branches_start: u32, branches_len: u32) res[bool, mach.base.fail.Fail]
```

whether any arm condition of a `$if` chain depends on something only a later phase knows: a `$`
parameter, a layout intrinsic, a type comparison, a field descriptor or a `$each` variable

| Parameter | |
| :--- | :--- |
| `a` | the tree |
| `source` | the tree's source text |
| `obs` | the observer the probes ask |
| `probes` | the observers |
| `branches_start` | first arm in the comptime branch table |
| `branches_len` | how many arms |
| **returns** | whether the chain is deferred, or the failure of the walk |

### gate_depends_on

```mach
pub fun gate_depends_on[T](
    a: *mach.front.ast.Ast, source: str, obs: *T, probes: GateProbes[T], probe: GateProbe, eid: mach.front.ast.id.ExprId) res[bool, mach.base.fail.Fail]
```

whether the expression `eid` or any expression under it depends on what `probe` looks for

| Parameter | |
| :--- | :--- |
| `a` | the tree |
| `source` | the tree's source text |
| `obs` | the observer the probes ask |
| `probes` | the observers |
| `probe` | which dependency to look for |
| `eid` | the expression, EXPR_NIL for none |
| **returns** | whether it depends on it, or the failure of the walk, a nesting refusal included |

## GateScope

```mach
pub def GateScope: u8
```

| Name | Value | |
| :--- | :--- | :--- |
| `GATE_SCOPE_DECL` | `0` |  |
| `GATE_SCOPE_INSTANCE` | `1` |  |

### evaluate_gate

```mach
pub fun evaluate_gate[T](
    c: *mach.front.comptime.scope.ComptimeCtx,
    cap_ctx: *T,
    caps: mach.front.comptime.capability.PhaseCapabilities[T],
    a: *mach.front.ast.Ast,
    source: str,
    cond: mach.front.ast.id.ExprId,
    interner: *mach.base.intern.Interner,
    scope: GateScope,
    cache: bool) res[GateVerdict, mach.base.fail.Fail]
```

decide a `$if` gate in the phase `caps` describes
a nil condition, a bare `$or`, is active. a condition the phase cannot decide yet waits on its
phase or its instance, and one that is no u1 constant is refused

| Parameter | |
| :--- | :--- |
| `c` | the scope the condition is evaluated in, which caches a decided gate |
| `cap_ctx` | the phase context |
| `caps` | the phase's capabilities |
| `a` | the tree |
| `source` | the tree's source text |
| `cond` | the condition |
| `interner` | the interner names are ids in |
| `scope` | whether the gate is a declaration's or an instance's |
| `cache` | whether the verdict is read from and written to the scope's gate cache |
| **returns** | the verdict, or an internal failure |

## GateVerdict

```mach
pub rec GateVerdict {
    outcome: GateOutcome;
    fail:    mach.front.comptime.failure.EvalFailKind;
    diag:    mach.base.diagnostic.kind.Kind;
    message: str;
    span:    mach.base.source.Span;
    depends: mach.base.intern.StrId;
    value:   mach.front.comptime.value.CTValue;
}
```

what deciding one gate gave

| Field | Type | |
| :--- | :--- | :--- |
| `outcome` | `GateOutcome` | whether the arm is active, inactive, refused or waits on a later phase or an instance |
| `fail` | `mach.front.comptime.failure.EvalFailKind` | the evaluation failure kind the verdict was decided from, EVAL_FAIL_NONE otherwise |
| `diag` | `mach.base.diagnostic.kind.Kind` | the diagnostic kind a refusal is reported as, NONE for a verdict that reports nothing |
| `message` | `str` | the refusal's text, "" for a verdict that reports nothing |
| `span` | `mach.base.source.Span` | where the refusal points, the condition or the run-time binding it reads |
| `depends` | `mach.base.intern.StrId` | the unbound name a failed gate waits on, STR_NIL when it waits on none |
| `value` | `mach.front.comptime.value.CTValue` | the condition's value, where the verdict keeps it |

## Functions

### comptime_if_declares_nothing

```mach
pub fun comptime_if_declares_nothing(a: *mach.front.ast.Ast, branches_start: u32, branches_len: u32) res[bool, mach.base.fail.Fail]
```

whether every arm of a chain, at any depth, declares nothing but directives and chains; a
chain nested deeper than the stack lets the walk descend is refused (`nesting`)

### nesting

```mach
pub fun nesting() mach.base.fail.Fail
```

the refusal of a walk over nesting deeper than the stack lets it descend, which the phase
that asked reports against the node it asked about

### gate_walk

```mach
pub fun gate_walk[T](a: *mach.front.ast.Ast, source: str, obs: *T,
    action: fun(*T, mach.front.ast.id.ExprId, *mach.front.ast.expr.Expr) res[GateNodeVerdict, mach.base.fail.Fail],
    expression: fun(*T, *mach.front.ast.Ast, mach.front.ast.id.ExprId) res[mach.front.ast.expr.Expr, mach.front.comptime.failure.EvalFail],
    eid: mach.front.ast.id.ExprId) res[bool, mach.base.fail.Fail]
```

walk the condition `eid` with `action` deciding each node: a node it says yes to ends the walk,
one it prunes is not descended

| Parameter | |
| :--- | :--- |
| `a` | the tree |
| `source` | the tree's source text |
| `obs` | the walk's own state, handed to both functions |
| `action` | the visit of each node |
| `expression` | how the walk reads an expression node |
| `eid` | the root of the walk |
| **returns** | whether a node was said yes to, or the failure of the walk, a nesting refusal included |

### never_decided_message

```mach
pub fun never_decided_message(a: *std.allocator.Allocator, itn: *mach.base.intern.Interner, depends: mach.base.intern.StrId) res[str, mach.base.fail.Fail]
```

what a failed gate says when the name it waits on is a constant nowhere the
module can see, which the caller frees

### type_question

```mach
pub fun type_question(a: *mach.front.ast.Ast, source: str, eid: mach.front.ast.id.ExprId) res[mach.front.ast.id.ExprId, mach.base.fail.Fail]
```

the first call in an expression that asks a type question, EXPR_NIL when none does

### type_question_message

```mach
pub fun type_question_message(interner: *mach.base.intern.Interner, a: *mach.front.ast.Ast, source: str, call: mach.front.ast.id.ExprId, through: str) res[mach.base.intern.StrId, mach.base.fail.Fail]
```

a declaring gate may not ask a type question; `through` names the constants it asks it through,
as ` through `T` -> `S``, or is empty when the gate asks it itself

