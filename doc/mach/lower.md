[mach](README.md) / **lower**

# mach.lower

lowering of a checked module, or a unit of instances, into one mirl module

a module lowers its top-level declarations, then the local instances and class copies its code
asked for. an instance it calls is only declared there and is defined by its instance unit,
whose members lower in the order of their mangled names. `verify_module` holds the result to
mirl's verifier, and a refusal there is the lowering's own fault, never the source's.

<sub>source: [`src/lower.mach`](../../src/lower.mach) · imported by 1 module</sub>

<sub>`lower_context` is [`mach.lower.context`](lower/context.md) · `mirl_module` is `mirl.ir.module` · `fail` is [`mach.base.fail`](base/fail.md) · `intern` is [`mach.base.intern`](base/intern.md) · `sema_product` is [`mach.front.sema.product`](front/sema/product.md) · `A` is `std.allocator`</sub>

| Functions | |
| :--- | :--- |
| [`lower_module`](#lower_module) | the module `req` names lowered into the mirl module `m`, and the unit of facts kept of it |
| [`lower_unit`](#lower_unit) | the instance unit `name` lowered into the mirl module `m`: each member, in the order given, read through the frames of its own typing, and the unit of facts kept of it |
| [`verify_module`](#verify_module) | the lowered module `m` named `name`, held to mirl's verifier on `machine` |

---

## Functions

### lower_module

```mach
pub fun lower_module(req: *lower_context.LowerRequest, m: *mirl_module.Module) res[lower_context.Unit, fail.Fail]
```

the module `req` names lowered into the mirl module `m`, and the unit of facts kept of it. an
instance its code calls is only declared: its instance unit defines it (lower_unit). holding
the module to mirl's verifier is verify_module's, a stage of its own

### lower_unit

```mach
pub fun lower_unit(req: *lower_context.LowerRequest, name: intern.StrId, members: **sema_product.InstanceResult, defining: *lower_context.Defining,
    count: u32, m: *mirl_module.Module) res[lower_context.Unit, fail.Fail]
```

the instance unit `name` lowered into the mirl module `m`: each member, in the order given,
read through the frames of its own typing, and the unit of facts kept of it. the order is the
members' mangled names, so the module does not depend on the order anything was typed in

### verify_module

```mach
pub fun verify_module(machine: *lower_context.Machine, a: *A.Allocator, name: str, m: *mirl_module.Module) err[fail.Fail]
```

the lowered module `m` named `name`, held to mirl's verifier on `machine`. a module the
verifier refuses is the lowering's own fault, never the source's

