# mach.lang.target.of.coff

## fun machine_for_arch

```mach
pub fun machine_for_arch(arch_id: u32) u16;
```

## fun looks_like_dll

```mach
pub fun looks_like_dll(buf: *u8, buf_size: usize, expect_machine: u16) bool;
```

## val VTABLE

```mach
pub val VTABLE: target_of.OfVTable = target_of.OfVTable;
```

