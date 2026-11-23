[mach](../README.md) / [manifest](README.md) / **step**

# mach.manifest.step

the decoding of the `[step.<name>]` tables

Source: [`src/manifest/step.mach`](../../../src/manifest/step.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.base.intern`](../base/intern.md).

| Functions | |
| :--- | :--- |
| [`parse_steps`](#parse_steps) | decode every `[step.<name>]` table of `t` into `m.steps`, nothing when there is none |

---

## Functions

### parse_steps

```mach
pub fun parse_steps(alloc: *std.allocator.Allocator, itn: *mach.base.intern.Interner, t: *toml.value.Table, m: *Manifest) err[mach.base.fail.Fail]
```

decode every `[step.<name>]` table of `t` into `m.steps`, nothing when there is none

| Parameter | |
| :--- | :--- |
| `alloc` | owns the step array, its entries and a refusal's text |
| `itn` | interns the names, words and paths |
| `t` | the manifest's root table |
| `m` | the manifest the steps are set on |
| **returns** | ok, or the refusal of the first step that does not decode or of more steps than a u32 counts |

