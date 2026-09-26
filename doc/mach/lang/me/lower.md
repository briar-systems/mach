# mach.lang.me.lower

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

