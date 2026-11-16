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
| [`GateProbes`](#gateprobes) | `rec` |  |
| [`GateScope`](#gatescope) | `def` |  |
| [`GateVerdict`](#gateverdict) | `rec` |  |

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
| [`gate_probes`](#gate_probes), [`gate_chain_depends_on`](#gate_chain_depends_on), [`gate_chain_defers`](#gate_chain_defers), [`gate_depends_on`](#gate_depends_on) | |

| On `GateScope` | |
| :--- | :--- |
| [`GATE_SCOPE_DECL`](#gatescope), [`GATE_SCOPE_INSTANCE`](#gatescope), [`evaluate_gate`](#evaluate_gate) | |

| Functions | |
| :--- | :--- |
| [`comptime_if_declares_nothing`](#comptime_if_declares_nothing) | whether every arm of a chain, at any depth, declares nothing but directives and chains; a chain nested deeper than the stack lets the walk descend is refused (`nesting`) |
| [`nesting`](#nesting) | the refusal of a walk over nesting deeper than the stack lets it descend, which the phase that asked reports against the node it asked about |
| [`gate_walk`](#gate_walk) | |
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

### gate_probes

```mach
pub fun gate_probes[T](
    comptime_param: fun(*T, mach.front.ast.id.ExprId) bool,
    each_loopvar: fun(*T, mach.front.ast.id.ExprId) bool,
    field_loopvar: fun(*T, mach.front.ast.id.ExprId) bool,
    field_type_operand: fun(*T, mach.front.ast.id.ExprId) res[bool, mach.base.fail.Fail],
    expression: fun(*T, *mach.front.ast.Ast, mach.front.ast.id.ExprId) res[mach.front.ast.expr.Expr, mach.front.comptime.failure.EvalFail]) GateProbes[T]
```

### gate_chain_depends_on

```mach
pub fun gate_chain_depends_on[T](
    a: *mach.front.ast.Ast, source: str, obs: *T, probes: GateProbes[T], probe: GateProbe,
    branches_start: u32, branches_len: u32) res[bool, mach.base.fail.Fail]
```

### gate_chain_defers

```mach
pub fun gate_chain_defers[T](
    a: *mach.front.ast.Ast, source: str, obs: *T, probes: GateProbes[T],
    branches_start: u32, branches_len: u32) res[bool, mach.base.fail.Fail]
```

### gate_depends_on

```mach
pub fun gate_depends_on[T](
    a: *mach.front.ast.Ast, source: str, obs: *T, probes: GateProbes[T], probe: GateProbe, eid: mach.front.ast.id.ExprId) res[bool, mach.base.fail.Fail]
```

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

