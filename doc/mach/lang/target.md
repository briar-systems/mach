# mach.lang.target

## fwd target_model.ENDIAN_LITTLE

```mach
fwd target_model.ENDIAN_LITTLE
```

forwards [`mach.lang.target.model.ENDIAN_LITTLE`](target/model.md#val-endian_little)

## fwd target_model.ENDIAN_BIG

```mach
fwd target_model.ENDIAN_BIG
```

forwards [`mach.lang.target.model.ENDIAN_BIG`](target/model.md#val-endian_big)

## fwd binding.Binding

```mach
fwd binding.Binding
```

forwards [`mach.lang.target.binding.Binding`](target/binding.md#rec-binding)

## fwd binding.live

```mach
fwd binding.live
```

forwards [`mach.lang.target.binding.live`](target/binding.md#fun-live)

## fwd resolved.ct_mul_admitted

```mach
fwd resolved.ct_mul_admitted
```

forwards [`mach.lang.target.resolved.ct_mul_admitted`](target/resolved.md#fun-ct_mul_admitted)

## fwd resolved.ct_mul_dit_cells

```mach
fwd resolved.ct_mul_dit_cells
```

forwards [`mach.lang.target.resolved.ct_mul_dit_cells`](target/resolved.md#fun-ct_mul_dit_cells)

## fwd resolved.dit_mode_guaranteed

```mach
fwd resolved.dit_mode_guaranteed
```

forwards [`mach.lang.target.resolved.dit_mode_guaranteed`](target/resolved.md#fun-dit_mode_guaranteed)

## fwd target_registry.TargetRegistry

```mach
fwd target_registry.TargetRegistry
```

forwards [`mach.lang.target.registry.TargetRegistry`](target/registry.md#rec-targetregistry)

## fwd target_registry.registry_init_with_allocator

```mach
fwd target_registry.registry_init_with_allocator
```

forwards [`mach.lang.target.registry.registry_init_with_allocator`](target/registry.md#fun-registry_init_with_allocator)

## fwd target_registry.registry_new

```mach
fwd target_registry.registry_new
```

forwards [`mach.lang.target.registry.registry_new`](target/registry.md#fun-registry_new)

## fwd target_registry.registry_dnit

```mach
fwd target_registry.registry_dnit
```

forwards [`mach.lang.target.registry.registry_dnit`](target/registry.md#fun-registry_dnit)

## fwd target_registry.registry_published

```mach
fwd target_registry.registry_published
```

forwards [`mach.lang.target.registry.registry_published`](target/registry.md#fun-registry_published)

## fun host_os_id

```mach
pub fun host_os_id() u32;
```

## fun host_arch_id

```mach
pub fun host_arch_id() u32;
```

## fun host_abi_id

```mach
pub fun host_abi_id() u32;
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

## fun with_extensions

```mach
pub fun with_extensions(req: *TargetRequest, names: *str, count: u32);
```

## fun with_of

```mach
pub fun with_of(req: *TargetRequest, of_name: str);
```

## fun with_env

```mach
pub fun with_env(req: *TargetRequest, env_name: str, origin: str);
```

## fun with_image

```mach
pub fun with_image(req: *TargetRequest, base: u64, stack_reserve: u64, stack_commit: u64);
```

## fun select_of

```mach
pub fun select_of(reg: *TargetRegistry, isa_name: str, os_name: str, abi_name: str, of_name: str) res[binding.Binding, fail.Fail];
```

## fun resolve

```mach
pub fun resolve(reg: *TargetRegistry, req: *TargetRequest) res[binding.Binding, fail.Fail];
```

## fun fingerprint

```mach
pub fun fingerprint(t: *binding.Binding, s: *wire.Sink);
```

## fun registered_selection

```mach
pub fun registered_selection(arch_vt: *isa.IsaVTable, model: *target_model.Machine) isa.IsaVTable;
```

the instruction set a [target.*] table selects by naming a registered isa: a
riscv template narrowed to the default extension set its registered name
parses to, every other template as registered. model backs the returned
vtable's model pointer and must outlive it

## fun registered_tuple_capability

```mach
pub fun registered_tuple_capability(os_vt: *lang_target_os.OsVTable, arch_vt: *isa.IsaVTable, abi_vt: *abi.AbiVTable, of_vt: *target_of.OfVTable) u32;
```

the capability of one cell of the target matrix as a [target.*] table naming
these four registered spellings composes it: the registered template alone
overstates a riscv isa, whose default selection may lack the float registers
an abi passes in

## fun selection_spelling

```mach
pub fun selection_spelling(isa_vt: *isa.IsaVTable, model: *target_model.Machine, buf: *u8, cap: usize) str;
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
pub fun select(reg: *TargetRegistry, isa_name: str, os_name: str, abi_name: str) res[binding.Binding, fail.Fail];
```

## fun artifact_naming

```mach
pub fun artifact_naming(tgt: *binding.Binding, kind: target_of.ArtifactOutputKind) res[target_of.ArtifactName, fail.Fail];
```

## val TUPLE_OK

```mach
pub val TUPLE_OK:              u32 = 0
```

