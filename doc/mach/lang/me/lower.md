# mach.lang.me.lower

## fun lower_module

```mach
pub fun lower_module(req: *lower_context.LowerRequest, m: *mirl_module.Module) res[lower_context.Unit, fail.Fail];
```

the module `req` names lowered into the mirl module `m`, and the unit of facts kept of it. an
instance its code calls is only declared: its instance unit defines it (lower_unit). holding
the module to mirl's verifier is verify_module's, a stage of its own

## fun lower_unit

```mach
pub fun lower_unit(req: *lower_context.LowerRequest, name: intern.StrId, members: **sema_product.InstanceResult, count: u32,
m: *mirl_module.Module) res[lower_context.Unit, fail.Fail];
```

the instance unit `name` lowered into the mirl module `m`: each member, in the order given,
read through the frames of its own typing, and the unit of facts kept of it. the order is the
members' mangled names, so the module does not depend on the order anything was typed in

## fun verify_module

```mach
pub fun verify_module(machine: *lower_context.Machine, a: *A.Allocator, name: str, m: *mirl_module.Module) err[fail.Fail];
```

the lowered module `m` named `name`, held to mirl's verifier on `machine`. a module the
verifier refuses is the lowering's own fault, never the source's

