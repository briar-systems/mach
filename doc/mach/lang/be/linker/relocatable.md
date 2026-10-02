# mach.lang.be.linker.relocatable

## fun combine

```mach
pub fun combine(alloc: *A.Allocator, itn: *intern.Interner,
inputs: *target_of.ObjectImage, count: u32, machine_flags: u32,
attributes: *u8, attributes_len: u32) res[target_of.ObjectImage, fail.Fail];
```

