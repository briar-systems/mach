# mach.lang.target.isa

## val ISA_LABEL

```mach
pub val ISA_LABEL:       u16 = 0xFFFF
```

## val ISA_PCOPY

```mach
pub val ISA_PCOPY:       u16 = 0xFFFD
```

## val ISA_PCOPY_FENCE

```mach
pub val ISA_PCOPY_FENCE: u16 = 0xFFFC
```

## val ISA_USE

```mach
pub val ISA_USE:         u16 = 0xFFFB
```

## def AsmCtScanFn

```mach
pub def AsmCtScanFn: fun(str, *ct.AsmSecret, u32, ct.CtMulMask, bool, *A.Allocator) err[ct.AsmRefusal]
```

the constant-time scan of an asm body: the tracked bindings (`ct.AsmSecret`, by the name
their `{name}` operand spells), the target's multiply admission and shift trust

## def DwarfRegFn

```mach
pub def DwarfRegFn: catalog_arch.DwarfRegFn
```

## def CvRegFn

```mach
pub def CvRegFn: catalog_arch.CvRegFn
```

## def RegFileFn

```mach
pub def RegFileFn: fun(*target_model.Register) i32
```

## rec BackendTarget

```mach
pub rec BackendTarget;
```

## rec AssemblyCapabilities

```mach
pub rec AssemblyCapabilities;
```

## rec RegMachine

```mach
pub rec RegMachine;
```

## fun body_writes_sp

```mach
pub fun body_writes_sp(m: *RegMachine, func: *lang_mir.MirFunction, sp: lang_mir.PRegId) bool;
```

whether the body may move the stack pointer. the encoders write sp only in
the prologue and epilogue, outgoing arguments live in the fixed frame, and no
instruction set allocates stack dynamically, so the writers are an inline-asm
block that writes sp and an instruction naming sp as a register operand

## rec ModuleEmitter

```mach
pub rec ModuleEmitter;
```

## def RelocSeam

```mach
pub def RelocSeam: target_of.RelocationCapabilities
```

## fun environment_lookup

```mach
pub fun environment_lookup(vt: *IsaVTable, name: str) u32;
```

## fun environment_profile

```mach
pub fun environment_profile(vt: *IsaVTable, env_id: u32) u32;
```

## fun environment_extensions

```mach
pub fun environment_extensions(vt: *IsaVTable, env_id: u32) u64;
```

the extensions the target's environment guarantees, which its selection holds
beside the ones it names

## rec Environment

```mach
pub rec Environment;
```

an execution environment an isa defines: its name, its profile, and the
extensions of the isa's vocabulary it guarantees (spirv's `zero_init_workgroup`
from vulkan1.3)

## val ENV_NONE

```mach
pub val ENV_NONE: u32 = 0xFFFFFFFF
```

## rec IsaVTable

```mach
pub rec IsaVTable;
```

## fun declares_local_got

```mach
pub fun declares_local_got(tgt_isa: *IsaVTable) bool;
```

## fun local_got_kind

```mach
pub fun local_got_kind(tgt_isa: *IsaVTable, kind: target_of.RelocKind) bool;
```

## fun declares_branch_thunks

```mach
pub fun declares_branch_thunks(tgt_isa: *IsaVTable) bool;
```

## fun branch_reach

```mach
pub fun branch_reach(tgt_isa: *IsaVTable, kind: target_of.RelocKind) opt[target_of.BranchReach];
```

the reach of a direct branch a thunk can extend, none for any other kind or
an instruction set that places no thunks

## fun declares_machine_flags

```mach
pub fun declares_machine_flags(tgt_isa: *IsaVTable) bool;
```

## fun machine_flags

```mach
pub fun machine_flags(tgt_isa: *IsaVTable, float_arg_bits: u32,
has_compressed: bool) u32;
```

## fun declares_attributes

```mach
pub fun declares_attributes(tgt_isa: *IsaVTable) bool;
```

## fun build_attributes

```mach
pub fun build_attributes(tgt_isa: *IsaVTable, model: *target_model.Machine, alloc: *A.Allocator, float_arg_bits: u32,
has_compressed: bool, out_len: *u32) res[*u8, fail.Fail];
```

## fun validate_attributes

```mach
pub fun validate_attributes(tgt_isa: *IsaVTable, model: *target_model.Machine,
bytes: *u8, len: u32, flags: u32) err[fail.Fail];
```

## fun merge_attributes

```mach
pub fun merge_attributes(tgt_isa: *IsaVTable, alloc: *A.Allocator, acc: *u8, acc_len: u32,
add: *u8, add_len: u32, out_len: *u32) res[*u8, fail.Fail];
```

## fun object_target

```mach
pub fun object_target(vt: *IsaVTable, out: *target_of.ObjectTarget);
```

## fun validate

```mach
pub fun validate(a: *A.Allocator, vt: *IsaVTable) err[fail.Fail];
```

why a descriptor is malformed, read once when a registry adds it; a refusal
names the instruction set and the rule it breaks

a: formats the refusal
vt: the descriptor

## fun name_of

```mach
pub fun name_of(vt: *IsaVTable) str;
```

## fun id_of

```mach
pub fun id_of(vt: *IsaVTable) u32;
```

## fun has_codegen

```mach
pub fun has_codegen(vt: *IsaVTable) bool;
```

## fun emits_whole_module

```mach
pub fun emits_whole_module(vt: *IsaVTable) bool;
```

## fun has_assembly

```mach
pub fun has_assembly(vt: *IsaVTable) bool;
```

whether source may carry `asm` blocks for this instruction set, whichever backend family it has

## fun emits_relocations

```mach
pub fun emits_relocations(vt: *IsaVTable) bool;
```

