[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **capability**

# mach.front.sema.capability

the comptime capabilities a sema walk publishes for each of its phases

<sub>source: [`src/front/sema/capability.mach`](../../../../src/front/sema/capability.mach) · imported by 1 module</sub>

<sub>`comptime_capability` is [`mach.front.comptime.capability`](../comptime/capability.md) · `sema_context` is [`mach.front.sema.context`](context.md) · `fail` is [`mach.base.fail`](../../base/fail.md)</sub>

| Functions | |
| :--- | :--- |
| [`semantic_name_caps`](#semantic_name_caps) · [`semantic_type_caps`](#semantic_type_caps) | |

---

## Functions

### semantic_name_caps

```mach
pub fun semantic_name_caps() res[comptime_capability.PhaseCapabilities[sema_context.SemaContext], fail.Fail]
```

### semantic_type_caps

```mach
pub fun semantic_type_caps() res[comptime_capability.PhaseCapabilities[sema_context.SemaContext], fail.Fail]
```

