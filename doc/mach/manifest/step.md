[mach](../README.md) / [manifest](README.md) / **step**

# mach.manifest.step

the decoding of the `[step.<name>]` tables

<sub>source: [`src/manifest/step.mach`](../../../src/manifest/step.mach) · imported by 1 module</sub>

<sub>`A` is `std.allocator` · `intern` is [`mach.base.intern`](../base/intern.md) · `toml_value` is `toml.value` · `fail` is [`mach.base.fail`](../base/fail.md)</sub>

| Functions | |
| :--- | :--- |
| [`parse_steps`](#parse_steps) | |

---

## Functions

### parse_steps

```mach
pub fun parse_steps(alloc: *A.Allocator, itn: *intern.Interner, t: *toml_value.Table, m: *Manifest) err[fail.Fail]
```

