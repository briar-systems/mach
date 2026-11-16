[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **stmt**

# mach.front.sema.stmt

typing statements, with the comptime `$if` and `$each` that stand among them

<sub>source: [`src/front/sema/stmt.mach`](../../../../src/front/sema/stmt.mach) · imported by 1 module</sub>

<sub>`sema_context` is [`mach.front.sema.context`](context.md) · `type_kind` is [`mach.type.kind`](../../type/kind.md) · `ast_id` is [`mach.front.ast.id`](../ast/id.md) · `base_source` is [`mach.base.source`](../../base/source.md) · `comptime_gate` is [`mach.front.comptime.gate`](../comptime/gate.md) · `fail` is [`mach.base.fail`](../../base/fail.md)</sub>

| Functions | |
| :--- | :--- |
| [`test_return_type`](#test_return_type) | |
| [`gate_condition_check`](#gate_condition_check) | a statement gate's verdict in the walk's frame, recorded there for lowering once decided types a gate's condition inside the gate and checks that it is a condition |
| [`deferred_gate_decide`](#deferred_gate_decide) | the verdict of a deferred chain's gate, none when the gate is not a condition or its verdict is neither active nor inactive, which is reported at `cond_span` |
| [`gate_verdict_truth`](#gate_verdict_truth) · [`infer_stmt`](#infer_stmt) · [`eval_comptime_directive`](#eval_comptime_directive) · [`check_binding`](#check_binding) | |

---

## Functions

### test_return_type

```mach
pub fun test_return_type(sc: *sema_context.SemaContext) type_kind.TypeId
```

### gate_condition_check

```mach
pub fun gate_condition_check(sc: *sema_context.SemaContext, cond: ast_id.ExprId) bool
```

a statement gate's verdict in the walk's frame, recorded there for lowering once decided
types a gate's condition inside the gate and checks that it is a condition

### deferred_gate_decide

```mach
pub fun deferred_gate_decide(sc: *sema_context.SemaContext, cond: ast_id.ExprId, cond_span: base_source.Span) res[opt[comptime_gate.GateVerdict], fail.Fail]
```

the verdict of a deferred chain's gate, none when the gate is not a condition or its
verdict is neither active nor inactive, which is reported at `cond_span`

### gate_verdict_truth

```mach
pub fun gate_verdict_truth(v: comptime_gate.GateVerdict) bool
```

### infer_stmt

```mach
pub fun infer_stmt(sc: *sema_context.SemaContext, sid: ast_id.StmtId, fn_ret: type_kind.TypeId) err[fail.Fail]
```

### eval_comptime_directive

```mach
pub fun eval_comptime_directive(sc: *sema_context.SemaContext, target: ast_id.ExprId, span: base_source.Span)
```

### check_binding

```mach
pub fun check_binding(sc: *sema_context.SemaContext, expected: type_kind.TypeId, eid: ast_id.ExprId, actual: type_kind.TypeId, span: base_source.Span)
```

