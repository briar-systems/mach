# mach.lang.be.linker.needs

## fun inputs_need_dit

```mach
pub fun inputs_need_dit(s: *session.Session, modules: *of.ObjectImage, module_count: u32) bool;
```

whether any input carries the DIT mark: the per-module fact, unioned

## fun defines_dit_cell

```mach
pub fun defines_dit_cell(tgt: *target.Target, mode: LinkMode) bool;
```

whether a link of this mode on this target defines the cell: an executable on
an os that declares the DIT guarantee for the instruction set. a shared
library runs no start code of its own and records nothing

## fun synthesize_runtime_needs

```mach
pub fun synthesize_runtime_needs(s: *session.Session, tgt: *target.Target,
modules: *of.ObjectImage, module_count: u32, mode: LinkMode,
out: *of.ObjectImage) res[bool, fail.Fail];
```

the synthetic input carrying the cell; false when this link defines none

