# mach.lang.target.isa.spirv.emit

a whole SPIR-V module emitted from a MIR unit, phase by phase, into the object image

## fun module_emit

```mach
pub fun module_emit(out_alloc: *A.Allocator, tgt: *isa.BackendTarget, u: *mir_unit.Unit,
dbg: *mir_debug.ModuleDebug, image: *target_of.ObjectImage) err[fail.Fail];
```

