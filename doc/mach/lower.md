[mach](README.md) / **lower**

# mach.lower

lowering of a checked module into one mirl module

a module lowers its top-level declarations, then every instance and class copy its code reaches,
directly or through other instances, so its object holds each instance its code calls and the
link keeps one copy. `verify_module` holds the result to mirl's verifier, and a refusal there is
the lowering's own fault, never the source's.

Source: [`src/lower.mach`](../../src/lower.mach)

Uses types from [`mach.base.fail`](base/fail.md), [`mach.lower.context`](lower/context.md).

| Functions | |
| :--- | :--- |
| [`lower_module`](#lower_module) | the module `req` names lowered into the mirl module `m`, and the unit of facts kept of it |
| [`verify_module`](#verify_module) | the lowered module `m` named `name`, held to mirl's verifier on `machine` |

---

## Functions

### lower_module

```mach
pub fun lower_module(req: *mach.lower.context.LowerRequest, m: *mirl.ir.module.Module) res[mach.lower.context.Unit, mach.base.fail.Fail]
```

the module `req` names lowered into the mirl module `m`, and the unit of facts kept of it.
holding the module to mirl's verifier is verify_module's, a stage of its own

### verify_module

```mach
pub fun verify_module(machine: *mach.lower.context.Machine, a: *std.allocator.Allocator, name: str, m: *mirl.ir.module.Module) err[mach.base.fail.Fail]
```

the lowered module `m` named `name`, held to mirl's verifier on `machine`. a module the
verifier refuses is the lowering's own fault, never the source's

