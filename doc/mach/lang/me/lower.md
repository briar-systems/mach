# mach.lang.me.lower

## fun lower_module

```mach
pub fun lower_module(req: *lower_context.LowerRequest, m: *mirl_module.Module) res[lower_context.Unit, fail.Fail];
```

the module `req` names lowered into the mirl module `m`, and the unit of facts kept of it.
holding the module to mirl's verifier is verify_module's, a stage of its own

## fun verify_module

```mach
pub fun verify_module(req: *lower_context.LowerRequest, m: *mirl_module.Module) err[fail.Fail];
```

the module `m`, lowered for `req`, held to mirl's verifier. a module the verifier refuses is
the lowering's own fault, never the source's

