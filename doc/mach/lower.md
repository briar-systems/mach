[mach](README.md) / **lower**

# mach.lower

lowering of checked modules into one mirl module

a unit lowers the top-level declarations of each of its modules, one module for a target that
compiles each apart and every module of the artifact for one that compiles it whole, then every
instance and class copy their code reaches, directly or through other instances, so its object
holds each instance its code calls and the link keeps one copy. `verify_module` holds the result
to mirl's verifier, and a refusal there is the lowering's own fault, never the source's.

Source: [`src/lower.mach`](../../src/lower.mach)

Uses types from [`mach.base.fail`](base/fail.md), [`mach.lower.context`](lower/context.md).

| Functions | |
| :--- | :--- |
| [`lower_modules`](#lower_modules) | the modules the `count` requests at `reqs` name lowered into the one mirl module `m`, in their order, and the unit of facts kept of it, named for the last |
| [`verify_module`](#verify_module) | the lowered module `m` named `name`, held to mirl's verifier on `machine` |

---

## Functions

### lower_modules

```mach
pub fun lower_modules(reqs: *mach.lower.context.LowerRequest, count: usize, m: *mirl.ir.module.Module) res[mach.lower.context.Unit, mach.base.fail.Fail]
```

the modules the `count` requests at `reqs` name lowered into the one mirl module `m`, in their
order, and the unit of facts kept of it, named for the last. a module's reference to another's
declaration declares it, and that module's definition takes the declaration's place. each
instance the modules reach is lowered once, after every module's declarations. holding the
module to mirl's verifier is verify_module's, a stage of its own

### verify_module

```mach
pub fun verify_module(machine: *mach.lower.context.Machine, a: *std.allocator.Allocator, name: str, m: *mirl.ir.module.Module) err[mach.base.fail.Fail]
```

the lowered module `m` named `name`, held to mirl's verifier on `machine`. a module the
verifier refuses is the lowering's own fault, never the source's

