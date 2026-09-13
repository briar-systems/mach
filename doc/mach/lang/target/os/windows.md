# mach.lang.target.os.windows

## val WINDOWS_BASE_ADDR

```mach
pub val WINDOWS_BASE_ADDR: u64 = 0x140000000
```

## val WINDOWS_PAGE_SIZE

```mach
pub val WINDOWS_PAGE_SIZE: u64 = 4096
```

## fun windows_va_list

```mach
pub fun windows_va_list(arch_id: u32) opt[os.VaList];
```

## fun register_windows

```mach
pub fun register_windows(reg: *os.OsRegistry) err[fail.Fail];
```

