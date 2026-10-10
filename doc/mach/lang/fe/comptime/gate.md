# mach.lang.fe.comptime.gate

comptime gates: which later phase a gate's condition depends on, and the verdict that
selects or discards its arm

## def GateOutcome

```mach
pub def GateOutcome: u8
```

## val GATE_INACTIVE

```mach
pub val GATE_INACTIVE:          GateOutcome = 0
```

## val GATE_ACTIVE

```mach
pub val GATE_ACTIVE:            GateOutcome = 1
```

## val GATE_REJECTED

```mach
pub val GATE_REJECTED:          GateOutcome = 2
```

## val GATE_FAILED

```mach
pub val GATE_FAILED:            GateOutcome = 3
```

## val GATE_AWAITING_PHASE

```mach
pub val GATE_AWAITING_PHASE:    GateOutcome = 4
```

## val GATE_AWAITING_INSTANCE

```mach
pub val GATE_AWAITING_INSTANCE: GateOutcome = 5
```

## fun comptime_if_declares_nothing

```mach
pub fun comptime_if_declares_nothing(a: *ast.Ast, branches_start: u32, branches_len: u32) res[bool, fail.Fail];
```

whether every arm of a chain, at any depth, declares nothing but directives and chains; a
chain nested deeper than the stack lets the walk descend is refused (`nesting`)

## fun nesting

```mach
pub fun nesting() fail.Fail;
```

the refusal of a walk over nesting deeper than the stack lets it descend, which the phase
that asked reports against the node it asked about

## def GateProbe

```mach
pub def GateProbe: u8
```

## val GATE_PROBE_COMPTIME_PARAM

```mach
pub val GATE_PROBE_COMPTIME_PARAM:   GateProbe = 0
```

## val GATE_PROBE_TYPE_COMPARISON

```mach
pub val GATE_PROBE_TYPE_COMPARISON:  GateProbe = 2
```

## def GateNodeVerdict

```mach
pub def GateNodeVerdict: u8
```

## val GATE_NODE_NO

```mach
pub val GATE_NODE_NO:    GateNodeVerdict = 0
```

## val GATE_NODE_PRUNE

```mach
pub val GATE_NODE_PRUNE: GateNodeVerdict = 2
```

## rec GateProbes

```mach
pub rec GateProbes[T];
```

## fun gate_probes

```mach
pub fun gate_probes[T](
comptime_param: fun(*T, ast_id.ExprId) bool,
each_loopvar: fun(*T, ast_id.ExprId) bool,
field_loopvar: fun(*T, ast_id.ExprId) bool,
field_type_operand: fun(*T, ast_id.ExprId) res[bool, fail.Fail],
expression: fun(*T, *ast.Ast, ast_id.ExprId) res[ast_expr.Expr, comptime_failure.EvalFail]) GateProbes[T];
```

## fun gate_walk

```mach
pub fun gate_walk[T](a: *ast.Ast, source: str, obs: *T,
action: fun(*T, ast_id.ExprId, *ast_expr.Expr) res[GateNodeVerdict, fail.Fail],
expression: fun(*T, *ast.Ast, ast_id.ExprId) res[ast_expr.Expr, comptime_failure.EvalFail],
eid: ast_id.ExprId) res[bool, fail.Fail];
```

## fun gate_chain_depends_on

```mach
pub fun gate_chain_depends_on[T](
a: *ast.Ast, source: str, obs: *T, probes: GateProbes[T], probe: GateProbe,
branches_start: u32, branches_len: u32) res[bool, fail.Fail];
```

## def GateScope

```mach
pub def GateScope: u8
```

## val GATE_SCOPE_DECL

```mach
pub val GATE_SCOPE_DECL:     GateScope = 0
```

## val GATE_SCOPE_INSTANCE

```mach
pub val GATE_SCOPE_INSTANCE: GateScope = 1
```

## rec GateVerdict

```mach
pub rec GateVerdict;
```

## fun never_decided_message

```mach
pub fun never_decided_message(a: *A.Allocator, itn: *intern.Interner, depends: intern.StrId) res[str, fail.Fail];
```

what a failed gate says when the name it waits on is a constant nowhere the
module can see, which the caller frees

## fun evaluate_gate

```mach
pub fun evaluate_gate[T](
c: *comptime_scope.ComptimeCtx,
cap_ctx: *T,
caps: comptime_capability.PhaseCapabilities[T],
a: *ast.Ast,
source: str,
cond: ast_id.ExprId,
interner: *intern.Interner,
scope: GateScope,
cache: bool) res[GateVerdict, fail.Fail];
```

## fun gate_chain_defers

```mach
pub fun gate_chain_defers[T](
a: *ast.Ast, source: str, obs: *T, probes: GateProbes[T],
branches_start: u32, branches_len: u32) res[bool, fail.Fail];
```

## fun gate_depends_on

```mach
pub fun gate_depends_on[T](
a: *ast.Ast, source: str, obs: *T, probes: GateProbes[T], probe: GateProbe, eid: ast_id.ExprId) res[bool, fail.Fail];
```

## fun type_question

```mach
pub fun type_question(a: *ast.Ast, source: str, eid: ast_id.ExprId) res[ast_id.ExprId, fail.Fail];
```

the first call in an expression that asks a type question, EXPR_NIL when none does

## fun type_question_message

```mach
pub fun type_question_message(interner: *intern.Interner, a: *ast.Ast, source: str, call: ast_id.ExprId, through: str) res[intern.StrId, fail.Fail];
```

a declaring gate may not ask a type question; `through` names the constants it asks it through,
as ` through `T` -> `S``, or is empty when the gate asks it itself

