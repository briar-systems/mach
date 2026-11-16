[mach](../README.md) / [manifest](README.md) / **step**

# mach.manifest.step

the decoding of the `[step.<name>]` tables

Source: [`src/manifest/step.mach`](../../../src/manifest/step.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.base.intern`](../base/intern.md).

| Functions | |
| :--- | :--- |
| [`parse_steps`](#parse_steps) | |

---

## Functions

### parse_steps

```mach
pub fun parse_steps(alloc: *std.allocator.Allocator, itn: *mach.base.intern.Interner, t: *toml.value.Table, m: *Manifest) err[mach.base.fail.Fail]
```

