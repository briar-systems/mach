# mach.lang.fe.sema.stmt

typing statements, with the comptime `$if` and `$each` that stand among them

## fun test_return_type

```mach
pub fun test_return_type(sc: *sema_context.SemaContext) type.TypeId;
```

## fun gate_condition_check

```mach
pub fun gate_condition_check(sc: *sema_context.SemaContext, cond: ast_id.ExprId) bool;
```

a statement gate's verdict in the walk's frame, recorded there for lowering once decided
types a gate's condition inside the gate and checks that it is a condition

## fun deferred_gate_decide

```mach
pub fun deferred_gate_decide(sc: *sema_context.SemaContext, cond: ast_id.ExprId, cond_span: lang_source.Span) res[opt[comptime_gate.GateVerdict], fail.Fail];
```

the verdict of a deferred chain's gate, none when the gate is not a condition or its
verdict is neither active nor inactive, which is reported at `cond_span`

## fun gate_verdict_truth

```mach
pub fun gate_verdict_truth(v: comptime_gate.GateVerdict) bool;
```

## fun infer_stmt

```mach
pub fun infer_stmt(sc: *sema_context.SemaContext, sid: ast_id.StmtId, fn_ret: type.TypeId) err[fail.Fail];
```

## fun eval_comptime_directive

```mach
pub fun eval_comptime_directive(sc: *sema_context.SemaContext, target: ast_id.ExprId, span: lang_source.Span);
```

## fun check_binding

```mach
pub fun check_binding(sc: *sema_context.SemaContext, expected: type.TypeId, eid: ast_id.ExprId, actual: type.TypeId, span: lang_source.Span);
```

