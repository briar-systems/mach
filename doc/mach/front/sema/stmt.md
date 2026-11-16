[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **stmt**

# mach.front.sema.stmt

typing statements, with the comptime `$if` and `$each` that stand among them

Source: [`src/front/sema/stmt.mach`](../../../../src/front/sema/stmt.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.base.source`](../../base/source.md), [`mach.front.ast.id`](../ast/id.md), [`mach.front.comptime.gate`](../comptime/gate.md), [`mach.front.sema.context`](context.md), [`mach.type.kind`](../../type/kind.md).

| Functions | |
| :--- | :--- |
| [`test_return_type`](#test_return_type) | |
| [`gate_condition_check`](#gate_condition_check) | a statement gate's verdict in the walk's frame, recorded there for lowering once decided types a gate's condition inside the gate and checks that it is a condition |
| [`deferred_gate_decide`](#deferred_gate_decide) | the verdict of a deferred chain's gate, none when the gate is not a condition or its verdict is neither active nor inactive, which is reported at `cond_span` |
| [`gate_verdict_truth`](#gate_verdict_truth), [`infer_stmt`](#infer_stmt), [`eval_comptime_directive`](#eval_comptime_directive), [`check_binding`](#check_binding) | |

---

## Functions

### test_return_type

```mach
pub fun test_return_type(sc: *mach.front.sema.context.SemaContext) mach.type.kind.TypeId
```

### gate_condition_check

```mach
pub fun gate_condition_check(sc: *mach.front.sema.context.SemaContext, cond: mach.front.ast.id.ExprId) bool
```

a statement gate's verdict in the walk's frame, recorded there for lowering once decided
types a gate's condition inside the gate and checks that it is a condition

### deferred_gate_decide

```mach
pub fun deferred_gate_decide(sc: *mach.front.sema.context.SemaContext, cond: mach.front.ast.id.ExprId, cond_span: mach.base.source.Span) res[opt[mach.front.comptime.gate.GateVerdict], mach.base.fail.Fail]
```

the verdict of a deferred chain's gate, none when the gate is not a condition or its
verdict is neither active nor inactive, which is reported at `cond_span`

### gate_verdict_truth

```mach
pub fun gate_verdict_truth(v: mach.front.comptime.gate.GateVerdict) bool
```

### infer_stmt

```mach
pub fun infer_stmt(sc: *mach.front.sema.context.SemaContext, sid: mach.front.ast.id.StmtId, fn_ret: mach.type.kind.TypeId) err[mach.base.fail.Fail]
```

### eval_comptime_directive

```mach
pub fun eval_comptime_directive(sc: *mach.front.sema.context.SemaContext, target: mach.front.ast.id.ExprId, span: mach.base.source.Span)
```

### check_binding

```mach
pub fun check_binding(sc: *mach.front.sema.context.SemaContext, expected: mach.type.kind.TypeId, eid: mach.front.ast.id.ExprId, actual: mach.type.kind.TypeId, span: mach.base.source.Span)
```

