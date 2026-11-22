[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **stmt**

# mach.front.sema.stmt

typing statements, with the comptime `$if` and `$each` that stand among them

Source: [`src/front/sema/stmt.mach`](../../../../src/front/sema/stmt.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.base.source`](../../base/source.md), [`mach.front.ast.decl`](../ast/decl.md), [`mach.front.ast.id`](../ast/id.md), [`mach.front.comptime.gate`](../comptime/gate.md), [`mach.front.sema.context`](context.md), [`mach.type.kind`](../../type/kind.md).

| Type | | |
| :--- | :--- | :--- |
| [`Flow`](#flow) | `rec` | how control leaves a statement the walk typed, read from the arms the build selects |

| On `Flow` | |
| :--- | :--- |
| [`infer_stmt`](#infer_stmt) | types the statement `sid` and everything under it, reporting what it finds to `sc` |

| Functions | |
| :--- | :--- |
| [`test_return_type`](#test_return_type) | the type a test body returns, `i32` |
| [`gate_condition_check`](#gate_condition_check) | types a gate's condition inside the gate and checks that it is a condition |
| [`deferred_gate_decide`](#deferred_gate_decide) | the verdict of a deferred chain's gate, none when the gate is not a condition or its verdict is neither active nor inactive, which is reported at `cond_span` |
| [`gate_verdict_truth`](#gate_verdict_truth) | whether a gate's verdict selects its arm |
| [`infer_fun_body`](#infer_fun_body) | types the body of the function `d` against its result type and holds where the body ends to the rules: neither a `noreturn` function nor one with a result type may reach the end of its body |
| [`eval_comptime_directive`](#eval_comptime_directive) | whether the directive at `span` is a `$error` that fired, reported as the walk reports |
| [`check_binding`](#check_binding) | holds the initializer `eid` of type `actual` to a binding's declared type `expected` |

---

## Flow

```mach
pub rec Flow {
    passes: bool;
    breaks: bool;
}
```

how control leaves a statement the walk typed, read from the arms the build selects

| Field | Type | |
| :--- | :--- | :--- |
| `passes` | `bool` | control can reach the statement's end and go on to the one after it |
| `breaks` | `bool` | a `brk` in it leaves the innermost loop around it |

### infer_stmt

```mach
pub fun infer_stmt(sc: *mach.front.sema.context.SemaContext, sid: mach.front.ast.id.StmtId, fn_ret: mach.type.kind.TypeId) res[Flow, mach.base.fail.Fail]
```

types the statement `sid` and everything under it, reporting what it finds to `sc`

| Parameter | |
| :--- | :--- |
| `sc` | the sema context of the module being typed |
| `sid` | the statement, nil for none |
| `fn_ret` | the return type of the enclosing function, which a `ret` is held to |
| **returns** | how control leaves the statement, or an internal failure: a statement id out of range, an unknown statement kind, or a failure of a walk under it |

## Functions

### test_return_type

```mach
pub fun test_return_type(sc: *mach.front.sema.context.SemaContext) mach.type.kind.TypeId
```

the type a test body returns, `i32`

| Parameter | |
| :--- | :--- |
| `sc` | the sema context whose type table is read |
| **returns** | the `i32` type, the error type when it is not interned |

### gate_condition_check

```mach
pub fun gate_condition_check(sc: *mach.front.sema.context.SemaContext, cond: mach.front.ast.id.ExprId) bool
```

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

whether a gate's verdict selects its arm

| Parameter | |
| :--- | :--- |
| `v` | the verdict |
| **returns** | true only when the gate is active |

### infer_fun_body

```mach
pub fun infer_fun_body(sc: *mach.front.sema.context.SemaContext, d: *mach.front.ast.decl.Decl, fn_ret: mach.type.kind.TypeId) err[mach.base.fail.Fail]
```

types the body of the function `d` against its result type and holds where the body ends to
the rules: neither a `noreturn` function nor one with a result type may reach the end of its
body. a `naked` body is assembly, which ends however its instructions do, so only the
`noreturn` rule holds there

| Parameter | |
| :--- | :--- |
| `sc` | the sema context of the module or instance being typed |
| `d` | the function, which has a body |
| `fn_ret` | its result type, nil for none |
| **returns** | ok, or an internal failure of the walk |

### eval_comptime_directive

```mach
pub fun eval_comptime_directive(sc: *mach.front.sema.context.SemaContext, target: mach.front.ast.id.ExprId, span: mach.base.source.Span) bool
```

whether the directive at `span` is a `$error` that fired, reported as the walk reports

### check_binding

```mach
pub fun check_binding(sc: *mach.front.sema.context.SemaContext, expected: mach.type.kind.TypeId, eid: mach.front.ast.id.ExprId, actual: mach.type.kind.TypeId, span: mach.base.source.Span, written: mach.front.ast.id.TypeId)
```

holds the initializer `eid` of type `actual` to a binding's declared type `expected`

| Parameter | |
| :--- | :--- |
| `sc` | the sema context of the module being typed |
| `expected` | the declared type |
| `eid` | the initializer |
| `actual` | the initializer's type |
| `span` | where a refusal is reported |
| `written` | the binding's annotation, whose spelling a mismatch names |

