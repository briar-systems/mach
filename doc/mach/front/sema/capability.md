[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **capability**

# mach.front.sema.capability

the comptime capabilities a sema walk publishes for each of its phases

Source: [`src/front/sema/capability.mach`](../../../../src/front/sema/capability.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.front.comptime.capability`](../comptime/capability.md), [`mach.front.sema.context`](context.md).

| Functions | |
| :--- | :--- |
| [`semantic_name_caps`](#semantic_name_caps) | the capabilities a comptime evaluation has in sema before types are known: members, constants, stores, runtime and secret checks, expressions and type heads, with no type queries |
| [`semantic_type_caps`](#semantic_type_caps) | the whole semantic capability set: everything semantic_name_caps gives, plus type comparison, the type store, field offsets, zero values, type and layout queries and scalar casts |

---

## Functions

### semantic_name_caps

```mach
pub fun semantic_name_caps() res[mach.front.comptime.capability.PhaseCapabilities[mach.front.sema.context.SemaContext], mach.base.fail.Fail]
```

the capabilities a comptime evaluation has in sema before types are known: members, constants,
stores, runtime and secret checks, expressions and type heads, with no type queries

| Parameter | |
| :--- | :--- |
| **returns** | the capability set, or the failure building it |

### semantic_type_caps

```mach
pub fun semantic_type_caps() res[mach.front.comptime.capability.PhaseCapabilities[mach.front.sema.context.SemaContext], mach.base.fail.Fail]
```

the whole semantic capability set: everything semantic_name_caps gives, plus type
comparison, the type store, field offsets, zero values, type and layout queries and scalar casts

| Parameter | |
| :--- | :--- |
| **returns** | the capability set, or the failure building it |

