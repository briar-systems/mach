# mach.lang.me.lower

## fwd context.LoopFrame

```mach
fwd context.LoopFrame
```

forwards [`mach.lang.me.lower.context.LoopFrame`](lower/context.md#rec-loopframe)

## fwd context.LowerContext

```mach
fwd context.LowerContext
```

forwards [`mach.lang.me.lower.context.LowerContext`](lower/context.md#rec-lowercontext)

## fwd context.LowerRequest

```mach
fwd context.LowerRequest
```

forwards [`mach.lang.me.lower.context.LowerRequest`](lower/context.md#rec-lowerrequest)

## fwd context.NormalObject

```mach
fwd context.NormalObject
```

forwards [`mach.lang.me.lower.context.NormalObject`](lower/context.md#rec-normalobject)

## fwd context.BindSweep

```mach
fwd context.BindSweep
```

forwards [`mach.lang.me.lower.context.BindSweep`](lower/context.md#def-bindsweep)

## fwd context.SWEEP_NORMAL

```mach
fwd context.SWEEP_NORMAL
```

forwards [`mach.lang.me.lower.context.SWEEP_NORMAL`](lower/context.md#val-sweep_normal)

## fwd context.SWEEP_TESTING

```mach
fwd context.SWEEP_TESTING
```

forwards [`mach.lang.me.lower.context.SWEEP_TESTING`](lower/context.md#val-sweep_testing)

## fwd context.ModuleScope

```mach
fwd context.ModuleScope
```

forwards [`mach.lang.me.lower.context.ModuleScope`](lower/context.md#rec-modulescope)

## fwd context.ScopeMark

```mach
fwd context.ScopeMark
```

forwards [`mach.lang.me.lower.context.ScopeMark`](lower/context.md#rec-scopemark)

## fwd context.request

```mach
fwd context.request
```

forwards [`mach.lang.me.lower.context.request`](lower/context.md#fun-request)

## fwd context.scope_enter

```mach
fwd context.scope_enter
```

forwards [`mach.lang.me.lower.context.scope_enter`](lower/context.md#fun-scope_enter)

## fwd context.scope_leave

```mach
fwd context.scope_leave
```

forwards [`mach.lang.me.lower.context.scope_leave`](lower/context.md#fun-scope_leave)

## fwd context.init_context

```mach
fwd context.init_context
```

forwards [`mach.lang.me.lower.context.init_context`](lower/context.md#fun-init_context)

## fwd context.dnit_context

```mach
fwd context.dnit_context
```

forwards [`mach.lang.me.lower.context.dnit_context`](lower/context.md#fun-dnit_context)

## fwd context.begin_function

```mach
fwd context.begin_function
```

forwards [`mach.lang.me.lower.context.begin_function`](lower/context.md#fun-begin_function)

## fwd context.bind_local

```mach
fwd context.bind_local
```

forwards [`mach.lang.me.lower.context.bind_local`](lower/context.md#fun-bind_local)

## fwd context.lookup_local

```mach
fwd context.lookup_local
```

forwards [`mach.lang.me.lower.context.lookup_local`](lower/context.md#fun-lookup_local)

## fwd context.push_loop

```mach
fwd context.push_loop
```

forwards [`mach.lang.me.lower.context.push_loop`](lower/context.md#fun-push_loop)

## fwd context.pop_loop

```mach
fwd context.pop_loop
```

forwards [`mach.lang.me.lower.context.pop_loop`](lower/context.md#fun-pop_loop)

## fwd context.current_loop

```mach
fwd context.current_loop
```

forwards [`mach.lang.me.lower.context.current_loop`](lower/context.md#fun-current_loop)

## fwd context.push_fin

```mach
fwd context.push_fin
```

forwards [`mach.lang.me.lower.context.push_fin`](lower/context.md#fun-push_fin)

## fwd context.fin_count

```mach
fwd context.fin_count
```

forwards [`mach.lang.me.lower.context.fin_count`](lower/context.md#fun-fin_count)

## fwd context.fin_at

```mach
fwd context.fin_at
```

forwards [`mach.lang.me.lower.context.fin_at`](lower/context.md#fun-fin_at)

## fwd context.lower_type

```mach
fwd context.lower_type
```

forwards [`mach.lang.me.lower.context.lower_type`](lower/context.md#fun-lower_type)

## fwd context.expr_type_of

```mach
fwd context.expr_type_of
```

forwards [`mach.lang.me.lower.context.expr_type_of`](lower/context.md#fun-expr_type_of)

## fwd context.type_resolved_of

```mach
fwd context.type_resolved_of
```

forwards [`mach.lang.me.lower.context.type_resolved_of`](lower/context.md#fun-type_resolved_of)

## fwd context.decl_type_of

```mach
fwd context.decl_type_of
```

forwards [`mach.lang.me.lower.context.decl_type_of`](lower/context.md#fun-decl_type_of)

## fwd context.expr_symbol_of

```mach
fwd context.expr_symbol_of
```

forwards [`mach.lang.me.lower.context.expr_symbol_of`](lower/context.md#fun-expr_symbol_of)

## fwd context.symbol_of

```mach
fwd context.symbol_of
```

forwards [`mach.lang.me.lower.context.symbol_of`](lower/context.md#fun-symbol_of)

## fun lower_module

```mach
pub fun lower_module(req: *context.LowerRequest) res[ir.Module, fail.Fail];
```

## fun bind_constants

```mach
pub fun bind_constants(req: *context.LowerRequest, sweep: context.BindSweep) err[fail.Fail];
```

binds the module's constants for `sweep` and emits nothing (see context.BindSweep)

## fun constants_pending

```mach
pub fun constants_pending(s: *session.Session, a: *ast.Ast, ctx: *comptime.ComptimeCtx, sweep: context.BindSweep) bool;
```

a bind sweep over the module has work: a module-scope global of the sweep's kind
whose constant is not bound yet, in any branch of a comptime `if`

