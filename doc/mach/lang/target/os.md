# mach.lang.target.os

## rec AbiSet

```mach
pub rec AbiSet;
```

the calling conventions an os accepts on one instruction set, the native one
first: a hosted os names the conventions its platform can carry and refuses
the rest, a freestanding os declares itself unconstrained and accepts every
convention the instruction set covers, its first name being only the default
`mach init` scaffolds. an empty constrained set means the os has no port to
that instruction set

## fun abi_set_none

```mach
pub fun abi_set_none() AbiSet;
```

## fun abi_set_one

```mach
pub fun abi_set_one(native: str) AbiSet;
```

## fun abi_set_add

```mach
pub fun abi_set_add(s: *AbiSet, name: str);
```

a further accepted convention. total over the set's declared capacity, which
every port's list fits: a set past OS_ABI_MAX is a defect of the descriptor

## fun abi_set_unconstrained

```mach
pub fun abi_set_unconstrained(native: str) AbiSet;
```

## rec VaList

```mach
pub rec VaList;
```

## fun va_list

```mach
pub fun va_list(size: u32, align: u32) VaList;
```

## rec OsVTable

```mach
pub rec OsVTable;
```

## fun validate

```mach
pub fun validate(a: *A.Allocator, vt: *OsVTable) err[fail.Fail];
```

why a descriptor is malformed, read once when a registry adds it: its
lists, and the answers its callbacks give for every isa it names

a: formats the refusal
vt: the descriptor

## fun name_of

```mach
pub fun name_of(vt: *OsVTable) str;
```

## fun id_of

```mach
pub fun id_of(vt: *OsVTable) u32;
```

## fun supports_of

```mach
pub fun supports_of(vt: *OsVTable, of_name: str) bool;
```

## fun supports_isa

```mach
pub fun supports_isa(vt: *OsVTable, arch_id: u32) bool;
```

## fun accepted_abis

```mach
pub fun accepted_abis(vt: *OsVTable, arch_id: u32) AbiSet;
```

the accepted abi set the os declares for an instruction set; an os with no
declaration accepts nothing

## fun accepts_abi

```mach
pub fun accepts_abi(vt: *OsVTable, arch_id: u32, abi_name: str) bool;
```

whether the os accepts a calling convention on an instruction set: every
convention when it declares itself unconstrained, else one of its names

## fun native_abi_for

```mach
pub fun native_abi_for(vt: *OsVTable, arch_id: u32) str;
```

the native calling convention, the first of the accepted set: what `native`
resolution and `mach init` use; "" when the os has no port to the isa

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

## fun dit_guaranteed_for

```mach
pub fun dit_guaranteed_for(vt: *OsVTable, arch_id: u32) bool;
```

the one door onto the DIT declaration: an os that declares nothing
guarantees nothing

## fun natural_stack_args_for

```mach
pub fun natural_stack_args_for(vt: *OsVTable, arch_id: u32) bool;
```

## fun reg_arg_align_for

```mach
pub fun reg_arg_align_for(vt: *OsVTable, arch_id: u32) u32;
```

## fun reg_arg_extend_for

```mach
pub fun reg_arg_extend_for(vt: *OsVTable, arch_id: u32) u32;
```

## fun declared_agg_align_for

```mach
pub fun declared_agg_align_for(vt: *OsVTable, arch_id: u32) bool;
```

## fun va_list_checked_for

```mach
pub fun va_list_checked_for(vt: *OsVTable, arch_id: u32)
res[opt[VaList], fail.Fail];
```

## fun fs_policy_checked_for

```mach
pub fun fs_policy_checked_for(vt: *OsVTable, arch_id: u32)
res[opt[catalog_os.FsPolicy], fail.Fail];
```

