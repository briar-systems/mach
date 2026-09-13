# mach.lang.target.os.darwin

## val DARWIN_BASE_ADDR

```mach
pub val DARWIN_BASE_ADDR: u64 = 0x100000000
```

## val DARWIN_PAGE_SIZE

```mach
pub val DARWIN_PAGE_SIZE: u64 = 4096
```

## fun darwin_va_list

```mach
pub fun darwin_va_list(arch_id: u32) opt[os.VaList];
```

## fun register_darwin

```mach
pub fun register_darwin(reg: *os.OsRegistry) err[fail.Fail];
```

