[mach](../README.md) / [manifest](README.md) / **profile**

# mach.manifest.profile

the decoding of the `[profile.<name>]` tables, and the rule of at most one default

Source: [`src/manifest/profile.mach`](../../../src/manifest/profile.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.base.intern`](../base/intern.md).

| Functions | |
| :--- | :--- |
| [`default_validate`](#default_validate) | more than one `default = true` profile is refused, pointing at the first `default` value and naming the others as related |
| [`parse_profiles`](#parse_profiles) | the `[profile.*]` tables; the schema check admits no manifest without one |

---

## Functions

### default_validate

```mach
pub fun default_validate(alloc: *std.allocator.Allocator, itn: *mach.base.intern.Interner, m: *Manifest) err[mach.base.fail.Fail]
```

more than one `default = true` profile is refused, pointing at the first
`default` value and naming the others as related

### parse_profiles

```mach
pub fun parse_profiles(alloc: *std.allocator.Allocator, itn: *mach.base.intern.Interner, t: *toml.value.Table, m: *Manifest) err[mach.base.fail.Fail]
```

the `[profile.*]` tables; the schema check admits no manifest without one

