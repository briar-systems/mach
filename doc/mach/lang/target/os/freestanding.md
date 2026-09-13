# mach.lang.target.os.freestanding

## val FREESTANDING_BASE_ADDR

```mach
pub val FREESTANDING_BASE_ADDR: u64 = 0
```

## val FREESTANDING_PAGE_SIZE

```mach
pub val FREESTANDING_PAGE_SIZE: u64 = 1
```

## fun freestanding_va_list

```mach
pub fun freestanding_va_list(arch_id: u32) opt[os.VaList];
```

## fun register_freestanding

```mach
pub fun register_freestanding(reg: *os.OsRegistry) err[fail.Fail];
```

