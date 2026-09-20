# mach.lang.target.isa.spirv.emit

## fun emit_module

```mach
pub fun emit_module(out_alloc: *A.Allocator, tgt: *isa.BackendTarget, u: *unit.Unit,
dbg: *debug_input.ModuleDebug, out_bytes: **u8, out_len: *u32) err[fail.Fail];
```

