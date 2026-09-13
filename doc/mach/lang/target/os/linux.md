# mach.lang.target.os.linux

## val LINUX_BASE_ADDR

```mach
pub val LINUX_BASE_ADDR: u64 = 0x400000
```

## val LINUX_PAGE_SIZE

```mach
pub val LINUX_PAGE_SIZE: u64 = 4096
```

## val LINUX_X86_64_DYNAMIC_LINKER

```mach
pub val LINUX_X86_64_DYNAMIC_LINKER:  str = "/lib64/ld-linux-x86-64.so.2"
```

## val LINUX_AARCH64_DYNAMIC_LINKER

```mach
pub val LINUX_AARCH64_DYNAMIC_LINKER: str = "/lib/ld-linux-aarch64.so.1"
```

## val LINUX_RISCV64_DYNAMIC_LINKER

```mach
pub val LINUX_RISCV64_DYNAMIC_LINKER: str = "/lib/ld-linux-riscv64-lp64d.so.1"
```

## fun linux_va_list

```mach
pub fun linux_va_list(arch_id: u32) opt[os.VaList];
```

## fun register_linux

```mach
pub fun register_linux(reg: *os.OsRegistry) err[fail.Fail];
```

