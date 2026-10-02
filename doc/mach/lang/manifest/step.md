# mach.lang.manifest.step

## fun parse_steps

```mach
pub fun parse_steps(alloc: *std_allocator.Allocator, itn: *intern.Interner, t: *toml.Table, m: *Manifest, as_root: bool) err[outcome.Fail];
```

## fun plan_visit

```mach
pub fun plan_visit(alloc: *std_allocator.Allocator, itn: *intern.Interner, m: *Manifest, idx: u32, state: *u8, order: *u32, order_len: *u32) err[outcome.Fail];
```

visit step `idx` and every step its `need` reaches, appending each to `order`
after the steps it needs; a step reached again while its own visit is open is
a `need` cycle, refused at its first edge's `need` entry with the others related

