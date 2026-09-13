# mach.lang.target.os

## val OS_UNKNOWN

```mach
pub val OS_UNKNOWN:      u32 = 0
```

## val OS_LINUX

```mach
pub val OS_LINUX:        u32 = 1
```

## val OS_DARWIN

```mach
pub val OS_DARWIN:       u32 = 2
```

## val OS_WINDOWS

```mach
pub val OS_WINDOWS:      u32 = 3
```

## val OS_FREESTANDING

```mach
pub val OS_FREESTANDING: u32 = 4
```

## val OS_CATALOG_VERSION

```mach
pub val OS_CATALOG_VERSION: u8 = 1
```

## fun os_id_for

```mach
pub fun os_id_for(name: str) u32;
```

## fun os_name_for

```mach
pub fun os_name_for(id: u32) str;
```

## fun os_catalog_len

```mach
pub fun os_catalog_len() usize;
```

## fun os_catalog_name

```mach
pub fun os_catalog_name(index: usize) str;
```

## fun os_fingerprint_tag

```mach
pub fun os_fingerprint_tag(id: u32) u8;
```

## val OS_LIBDIR_MAX

```mach
pub val OS_LIBDIR_MAX: u32 = 8
```

## val OS_OF_MAX

```mach
pub val OS_OF_MAX: u32 = 8
```

## val OS_ISA_MAX

```mach
pub val OS_ISA_MAX: u32 = 8
```

## def PieExecFn

```mach
pub def PieExecFn: fun(u32) bool
```

## def PageSizeFn

```mach
pub def PageSizeFn: fun(u32) u64
```

## def NativeAbiFn

```mach
pub def NativeAbiFn: fun(u32) str
```

## def VariadicStackFn

```mach
pub def VariadicStackFn: fun(u32) bool
```

## def NaturalStackArgsFn

```mach
pub def NaturalStackArgsFn: fun(u32) bool
```

## def ReservedGpFn

```mach
pub def ReservedGpFn: fun(u32) u32
```

## rec VaList

```mach
pub rec VaList;
```

## fun va_list

```mach
pub fun va_list(size: u32, align: u32) VaList;
```

## def VaListFn

```mach
pub def VaListFn: fun(u32) opt[VaList]
```

## rec FsPolicy

```mach
pub rec FsPolicy;
```

## def FsPolicyFn

```mach
pub def FsPolicyFn: fun(u32) opt[FsPolicy]
```

## rec OsVTable

```mach
pub rec OsVTable;
```

## rec OsRegistry

```mach
pub rec OsRegistry;
```

## fun registry_init_with_allocator

```mach
pub fun registry_init_with_allocator(alloc: *A.Allocator) OsRegistry;
```

## fun registry_init

```mach
pub fun registry_init() OsRegistry;
```

## fun registry_dnit

```mach
pub fun registry_dnit(reg: *OsRegistry);
```

## fun registry_validate

```mach
pub fun registry_validate(reg: *OsRegistry) err[fail.Fail];
```

## fun register

```mach
pub fun register(reg: *OsRegistry, vt: *OsVTable) err[fail.Fail];
```

## fun lookup

```mach
pub fun lookup(reg: *OsRegistry, name: str) opt[*OsVTable];
```

## fun registered_count

```mach
pub fun registered_count(reg: *OsRegistry) u32;
```

## fun registered

```mach
pub fun registered(reg: *OsRegistry, idx: u32) opt[*OsVTable];
```

## fun support_of

```mach
pub fun support_of(vt: *OsVTable, of_name: str);
```

## fun supports_of

```mach
pub fun supports_of(vt: *OsVTable, of_name: str) bool;
```

## fun support_isa

```mach
pub fun support_isa(vt: *OsVTable, arch_id: u32);
```

## fun supports_isa

```mach
pub fun supports_isa(vt: *OsVTable, arch_id: u32) bool;
```

## fun native_abi_for

```mach
pub fun native_abi_for(vt: *OsVTable, arch_id: u32) str;
```

## fun page_size_checked_for

```mach
pub fun page_size_checked_for(vt: *OsVTable, arch_id: u32) res[u64, fail.Fail];
```

## fun variadic_stack_for

```mach
pub fun variadic_stack_for(vt: *OsVTable, arch_id: u32) bool;
```

## fun reserved_gp_for

```mach
pub fun reserved_gp_for(vt: *OsVTable, arch_id: u32) u32;
```

## fun natural_stack_args_for

```mach
pub fun natural_stack_args_for(vt: *OsVTable, arch_id: u32) bool;
```

## fun va_list_for

```mach
pub fun va_list_for(vt: *OsVTable, arch_id: u32) opt[VaList];
```

## fun fs_policy_for

```mach
pub fun fs_policy_for(vt: *OsVTable, arch_id: u32) opt[FsPolicy];
```

## fun va_list_checked_for

```mach
pub fun va_list_checked_for(vt: *OsVTable, arch_id: u32)
res[opt[VaList], fail.Fail];
```

## fun fs_policy_checked_for

```mach
pub fun fs_policy_checked_for(vt: *OsVTable, arch_id: u32)
res[opt[FsPolicy], fail.Fail];
```

