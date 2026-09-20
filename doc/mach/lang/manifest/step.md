# mach.lang.manifest.step

## fun parse_steps

```mach
pub fun parse_steps(alloc: *A.Allocator, itn: *intern.Interner, t: *toml.Table, m: *Manifest, as_root: bool) err[outcome.Fail];
```

## fun plan_visit

```mach
pub fun plan_visit(alloc: *A.Allocator, itn: *intern.Interner, m: *Manifest, idx: u32, state: *u8, order: *u32, order_len: *u32) err[outcome.Fail];
```

