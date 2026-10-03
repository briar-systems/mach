# mach.lang.manifest.profile

## fun default_validate

```mach
pub fun default_validate(alloc: *A.Allocator, itn: *intern.Interner, m: *Manifest) err[fail.Fail];
```

more than one `default = true` profile is refused, pointing at the first
`default` value and naming the others as related

## fun parse_profiles

```mach
pub fun parse_profiles(alloc: *A.Allocator, itn: *intern.Interner, t: *toml.Table, m: *Manifest) err[fail.Fail];
```

the `[profile.*]` tables; the schema check admits no manifest without one

