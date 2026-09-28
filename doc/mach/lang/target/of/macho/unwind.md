# mach.lang.target.of.macho.unwind

mach-o unwind: `__TEXT,__unwind_info` holds one compact encoding per
function, and a frame compact unwind cannot say points into `__TEXT,__eh_frame`
instead. both are sized from the frame records' steps before layout and
filled once every function has its address

## fun macho_unwind_shape

```mach
pub fun macho_unwind_shape(arch_id: u32, frames: *of.FrameUnwind, count: u32, foreign: *of.ForeignUnwind) res[of.UnwindShape, fail.Fail];
```

`__unwind_info` for a function and a gap after each, and `__eh_frame` for
the frames compact unwind cannot say, after the frame descriptions the
foreign objects carry in. a foreign function has an entry from its object's
compact unwind or its frame description, so every one is counted twice at most

## fun fill_unwind

```mach
pub fun fill_unwind(alloc: *A.Allocator, arch_id: u32, segs: *of.LoadSegment, seg_count: u32,
funcs: *of.ExecFunction, func_count: u32, mh_addr: u64) err[fail.Fail];
```

fills the unwind tables the linker reserved: an encoding per function in
address order, a zero one across each gap no function covers, and the dwarf
descriptions the encodings that need them point to. a mach function's entry
follows from its frame record, a foreign one's from its object's compact
unwind or, lacking one, from the frame description its object carried into
`__eh_frame` ahead of this table's own. every function offset counts from
`mh_addr`, where the mach header is mapped

