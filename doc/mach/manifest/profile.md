[mach](../README.md) / [manifest](README.md) / **profile**

# mach.manifest.profile

the decoding of the `[profile.<name>]` tables, and the rule of at most one default

Source: [`src/manifest/profile.mach`](../../../src/manifest/profile.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.base.intern`](../base/intern.md).

| Functions | |
| :--- | :--- |
| [`default_validate`](#default_validate) | more than one `default = true` profile is refused, pointing at the first `default` value and naming the others as related |
| [`profile_set`](#profile_set) | every key of a profile the table `sub` holds, decoded over `pf`: `optimize`, `debug`, `linker_relax`, `relax`, `simd` and `allow` |
| [`parse_profiles`](#parse_profiles) | the `[profile.*]` tables; the schema check admits no manifest without one |

---

## Functions

### default_validate

```mach
pub fun default_validate(alloc: *std.allocator.Allocator, itn: *mach.base.intern.Interner, m: *Manifest) err[mach.base.fail.Fail]
```

more than one `default = true` profile is refused, pointing at the first
`default` value and naming the others as related

### profile_set

```mach
pub fun profile_set(alloc: *std.allocator.Allocator, itn: *mach.base.intern.Interner, sub: *toml.value.Table, lead: str, pf: *ProfileDef) err[mach.base.fail.Fail]
```

every key of a profile the table `sub` holds, decoded over `pf`: `optimize`, `debug`,
`linker_relax`, `relax`, `simd` and `allow`. a key `sub` does not hold leaves `pf` as it is,
and `default`, which picks the profile a build selects, is read by the caller

| Parameter | |
| :--- | :--- |
| `alloc` | owns a refusal's text and the lists it decodes |
| `itn` | receives the strings |
| `sub` | the table |
| `lead` | the table's label in mach.toml, nil for a `--set` value |
| `pf` | the profile |
| **returns** | ok, or the refusal of a value its key does not take, or a refused allocation |

### parse_profiles

```mach
pub fun parse_profiles(alloc: *std.allocator.Allocator, itn: *mach.base.intern.Interner, t: *toml.value.Table, m: *Manifest) err[mach.base.fail.Fail]
```

the `[profile.*]` tables; the schema check admits no manifest without one

