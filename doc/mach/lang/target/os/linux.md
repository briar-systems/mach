# mach.lang.target.os.linux

## val LINUX_BASE_ADDR

```mach
pub val LINUX_BASE_ADDR: u64 = 0x400000
```

## val LINUX_PAGE_SIZE

```mach
pub val LINUX_PAGE_SIZE: u64 = 4096
```

## fun register_linux

```mach
pub fun register_linux(reg: *lang_target_os.OsRegistry) err[fail.Fail];
```

