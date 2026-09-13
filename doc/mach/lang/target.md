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

## fun registered_selection

```mach
pub fun registered_selection(arch_vt: *isa.IsaVTable, model: *isa.MachineModel) isa.IsaVTable;
```

the instruction set a [target.*] table selects by naming a registered isa: a
riscv template narrowed to the default extension set its registered name
parses to, every other template as registered. model backs the returned
vtable's model pointer and must outlive it

## fun registered_tuple_capability

```mach
pub fun registered_tuple_capability(os_vt: *os.OsVTable, arch_vt: *isa.IsaVTable, abi_vt: *abi.AbiVTable, of_vt: *of.OfVTable) u32;
```

the capability of one cell of the target matrix as a [target.*] table naming
these four registered spellings composes it: the registered template alone
overstates a riscv isa, whose default selection may lack the float registers
an abi passes in

## fun selection_spelling

```mach
pub fun selection_spelling(isa_vt: *isa.IsaVTable, model: *isa.MachineModel, buf: *u8, cap: usize) str;
```

the canonical selection string for an instruction set and the model selected
on it (a resolved target's `model`, or the one registered_selection filled),
written into buf: a riscv isa spells the extensions the model holds, every
other isa its name

## fun host_request

```mach
pub fun host_request() TargetRequest;
```

the request a `native` selection, or a [target.*] table naming the host
without an object format, resolves: the host os, isa and abi by their
registered names, the object format left to the os default

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

## val TUPLE_OS_ABI

```mach
pub val TUPLE_OS_ABI:          u32 = 9
```

## fun tuple_capability

```mach
pub fun tuple_capability(os_vt: *os.OsVTable, isa_vt: *isa.IsaVTable, abi_vt: *abi.AbiVTable, of_vt: *of.OfVTable) u32;
```

