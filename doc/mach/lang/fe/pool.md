# mach.lang.fe.pool

## fun grow

```mach
pub fun grow[T](a: *std_allocator.Allocator, data: *T, cap: usize, seed: usize, out_cap: *usize) res[*T, std_allocator.Error];
```

