# mach.lang.target.tuple

whether one cell of the target matrix, an os, isa, abi and object format, composes, and the refusal that names why not

## val OK

```mach
pub val OK:              u32 = 0
```

## val NO_CODEGEN

```mach
pub val NO_CODEGEN:      u32 = 1
```

## val OS_NO_ISA

```mach
pub val OS_NO_ISA:       u32 = 6
```

## val ABI_NEEDS_FLOAT

```mach
pub val ABI_NEEDS_FLOAT: u32 = 7
```

## val ABI_TRANSPORT

```mach
pub val ABI_TRANSPORT:   u32 = 8
```

## val OS_ABI

```mach
pub val OS_ABI:          u32 = 9
```

## fun capability

```mach
pub fun capability(os_vt: *lang_target_os.OsVTable, isa_vt: *isa.IsaVTable, abi_vt: *abi.AbiVTable, of_vt: *target_of.OfVTable) u32;
```

## fun refusal

```mach
pub fun refusal(a: *A.Allocator, code: u32, os_vt: *lang_target_os.OsVTable, isa_vt: *isa.IsaVTable, abi_vt: *abi.AbiVTable, of_vt: *target_of.OfVTable) fail.Fail;
```

