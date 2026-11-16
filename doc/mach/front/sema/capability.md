[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **capability**

# mach.front.sema.capability

the comptime capabilities a sema walk publishes for each of its phases

Source: [`src/front/sema/capability.mach`](../../../../src/front/sema/capability.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.front.comptime.capability`](../comptime/capability.md), [`mach.front.sema.context`](context.md).

| Functions | |
| :--- | :--- |
| [`semantic_name_caps`](#semantic_name_caps), [`semantic_type_caps`](#semantic_type_caps) | |

---

## Functions

### semantic_name_caps

```mach
pub fun semantic_name_caps() res[mach.front.comptime.capability.PhaseCapabilities[mach.front.sema.context.SemaContext], mach.base.fail.Fail]
```

### semantic_type_caps

```mach
pub fun semantic_type_caps() res[mach.front.comptime.capability.PhaseCapabilities[mach.front.sema.context.SemaContext], mach.base.fail.Fail]
```

