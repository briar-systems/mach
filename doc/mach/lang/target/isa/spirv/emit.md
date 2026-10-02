# mach.lang.target.isa.spirv.emit

## fun emit_module

```mach
pub fun emit_module(out_alloc: *std_allocator.Allocator, tgt: *isa.BackendTarget, u: *be_codegen_unit.Unit,
dbg: *debug_input.ModuleDebug, image: *target_of.ObjectImage) err[fail.Fail];
```

