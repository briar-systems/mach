[mach](../README.md) / [manifest](README.md) / **profile**

# mach.manifest.profile

the decoding of the `[profile.<name>]` tables, and the rule of at most one default

<sub>source: [`src/manifest/profile.mach`](../../../src/manifest/profile.mach) · imported by 1 module</sub>

<sub>`A` is `std.allocator` · `intern` is [`mach.base.intern`](../base/intern.md) · `fail` is [`mach.base.fail`](../base/fail.md) · `toml_value` is `toml.value`</sub>

| Functions | |
| :--- | :--- |
| [`default_validate`](#default_validate) | more than one `default = true` profile is refused, pointing at the first `default` value and naming the others as related |
| [`parse_profiles`](#parse_profiles) | the `[profile.*]` tables; the schema check admits no manifest without one |

---

## Functions

### default_validate

```mach
pub fun default_validate(alloc: *A.Allocator, itn: *intern.Interner, m: *Manifest) err[fail.Fail]
```

more than one `default = true` profile is refused, pointing at the first
`default` value and naming the others as related

### parse_profiles

```mach
pub fun parse_profiles(alloc: *A.Allocator, itn: *intern.Interner, t: *toml_value.Table, m: *Manifest) err[fail.Fail]
```

the `[profile.*]` tables; the schema check admits no manifest without one

