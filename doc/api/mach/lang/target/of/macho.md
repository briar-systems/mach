# mach.lang.target.of.macho

## fun machine_for_arch

```mach
pub fun machine_for_arch(arch_id: u32) u32;
```

## fun register_macho

```mach
pub fun register_macho(reg: *of.OfRegistry) err[fail.Fail];
```

## fun parse_object

```mach
pub fun parse_object(alloc: *A.Allocator, itn: *intern.Interner, buf: *u8, buf_size: usize, out: *of.ObjectImage) err[fail.Fail];
```

