# mach.lang.be.linker.needs

the runtime needs a linked executable records for its start code (#3508).
codegen marks a module that multiplies a secret in a cell admitted only under
PSTATE.DIT with the local absolute symbol `ct.DIT_NEED_MARKER`; the link
unions the marks and, on a target whose os declares the mode guaranteed,
appends one synthetic input defining `ct.DIT_REQUIRED_CELL`: a hidden
one-byte read-only object the std start code reads, 1 when any input carries
the mark and 0 otherwise. the cell is an ordinary input, so resolution and
dead-stripping treat it like any other definition: a start code that never
reads it leaves nothing in the image. a target whose os declares nothing gets
no cell, and its DIT rows are refused before any mark can exist

## fun synthesize_runtime_needs

```mach
pub fun synthesize_runtime_needs(s: *session.Session, tgt: *lang_target.Target,
modules: *target_of.ObjectImage, module_count: u32, mode: LinkMode,
out: *target_of.ObjectImage) res[bool, fail.Fail];
```

the synthetic input carrying the cell; false when this link defines none

