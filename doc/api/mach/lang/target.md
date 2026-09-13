# mach.lang.target

## fun host_os_id

```mach
pub fun host_os_id() u32;
```

## fun host_arch_id

```mach
pub fun host_arch_id() u32;
```

## fun host_pointer_width

```mach
pub fun host_pointer_width() u32;
```

## fun host_va_list

```mach
pub fun host_va_list() opt[os.VaList];
```

## fun host_abi_id

```mach
pub fun host_abi_id() u32;
```

## def DebugDescriptorProvider

```mach
pub def DebugDescriptorProvider: fun() res[of.DebugVTable, fail.Fail]
```

## fun register_all

```mach
pub fun register_all(reg: *TargetRegistry, debug_provider: DebugDescriptorProvider) err[fail.Fail];
```

## rec TargetRequest

```mach
pub rec TargetRequest;
```

## fun target_request

```mach
pub fun target_request(isa_name: str, os_name: str, abi_name: str) TargetRequest;
```

## fun with_of

```mach
pub fun with_of(req: *TargetRequest, of_name: str);
```

## fun with_env

```mach
pub fun with_env(req: *TargetRequest, env_name: str, label: str);
```

## fun with_image

```mach
pub fun with_image(req: *TargetRequest, base: u64, stack_reserve: u64, stack_commit: u64);
```

## val WITHDRAWN_MOS6502

```mach
pub val WITHDRAWN_MOS6502:     str = "mos6502"
```

the MOS 6502 target was withdrawn and deleted in 5.0.0 (#3226, #3112); its
spelling is refused by name so a manifest that still names it learns why

## val WITHDRAWN_MOS6502_MSG

```mach
pub val WITHDRAWN_MOS6502_MSG: str = "target 'mos6502' was withdrawn and removed in 5.0.0
```

## fun select_of

```mach
pub fun select_of(reg: *TargetRegistry, isa_name: str, os_name: str, abi_name: str, of_name: str) res[resolved.Target, fail.Fail];
```

## fun resolve

```mach
pub fun resolve(reg: *TargetRegistry, req: *TargetRequest) res[resolved.Target, fail.Fail];
```

## val TARGET_FINGERPRINT_VERSION

```mach
pub val TARGET_FINGERPRINT_VERSION: u8 = 3
```

## fun fingerprint

```mach
pub fun fingerprint(t: *resolved.Target, e: *binary.Encoder) bool;
```

## fun select

```mach
pub fun select(reg: *TargetRegistry, isa_name: str, os_name: str, abi_name: str) res[resolved.Target, fail.Fail];
```

## fun artifact_naming

```mach
pub fun artifact_naming(tgt: *resolved.Target, kind: of.ArtifactOutputKind) res[of.ArtifactName, fail.Fail];
```

## val TUPLE_OK

```mach
pub val TUPLE_OK:              u32 = 0
```

## val TUPLE_NO_CODEGEN

```mach
pub val TUPLE_NO_CODEGEN:      u32 = 1
```

## val TUPLE_ABI_MISMATCH

```mach
pub val TUPLE_ABI_MISMATCH:    u32 = 2
```

## val TUPLE_OS_NO_OF

```mach
pub val TUPLE_OS_NO_OF:        u32 = 3
```

## val TUPLE_OF_UNCOVERED

```mach
pub val TUPLE_OF_UNCOVERED:    u32 = 4
```

## val TUPLE_OF_SHAPE

```mach
pub val TUPLE_OF_SHAPE:        u32 = 5
```

## val TUPLE_OS_NO_ISA

```mach
pub val TUPLE_OS_NO_ISA:       u32 = 6
```

## val TUPLE_ABI_NEEDS_FLOAT

```mach
pub val TUPLE_ABI_NEEDS_FLOAT: u32 = 7
```

## val TUPLE_ABI_TRANSPORT

```mach
pub val TUPLE_ABI_TRANSPORT:   u32 = 8
```

## fun tuple_capability

```mach
pub fun tuple_capability(os_vt: *os.OsVTable, isa_vt: *isa.IsaVTable, abi_vt: *abi.AbiVTable, of_vt: *of.OfVTable) u32;
```

