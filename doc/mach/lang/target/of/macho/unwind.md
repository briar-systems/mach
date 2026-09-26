# mach.lang.target.of.macho.unwind

## fun macho_unwind_shape

```mach
pub fun macho_unwind_shape(arch_id: u32, frames: *of.FrameUnwind, count: u32) res[of.UnwindShape, fail.Fail];
```

`__unwind_info` for a function and a gap after each, and `__eh_frame` for
the frames compact unwind cannot say

## fun fill_unwind

```mach
pub fun fill_unwind(alloc: *A.Allocator, arch_id: u32, segs: *of.LoadSegment, seg_count: u32,
funcs: *of.ExecFunction, func_count: u32, mh_addr: u64) err[fail.Fail];
```

fills the unwind tables the linker reserved: an encoding per function with a
frame record in address order, a zero one across each gap no such function
covers, and the dwarf descriptions the encodings that need them point to.
every function offset counts from `mh_addr`, where the mach header is mapped

