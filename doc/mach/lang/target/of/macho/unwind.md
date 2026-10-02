# mach.lang.target.of.macho.unwind

mach-o unwind: `__TEXT,__unwind_info` holds one compact encoding per
function, with the personalities and lsdas foreign functions name, and a
frame compact unwind cannot say points into `__TEXT,__eh_frame` instead. both
are sized from the frame records' steps before layout and filled once every
function has its address

## fun macho_unwind_shape

```mach
pub fun macho_unwind_shape(arch_id: u32, frames: *target_of.FrameUnwind, count: u32, foreign: *target_of.ForeignUnwind) res[target_of.UnwindShape, fail.Fail];
```

`__unwind_info` for a function and a gap after each, with a full personality
table and an lsda for every foreign entry when there are any, and
`__eh_frame` for the frames compact unwind cannot say, after the frame
descriptions the foreign objects carry in. a foreign function has an entry
from its object's compact unwind or its frame description, so every one is
counted twice at most

## fun fill_unwind

```mach
pub fun fill_unwind(alloc: *A.Allocator, arch_id: u32, segs: *target_of.LoadSegment, seg_count: u32,
funcs: *target_of.ExecFunction, func_count: u32, mh_addr: u64, dyn: *target_of.DynamicInfo) err[fail.Fail];
```

fills the unwind tables the linker reserved: an encoding per function in
address order, a zero one across each gap no function covers, and the dwarf
descriptions the encodings that need them point to. a mach function's entry
follows from its frame record, a foreign one's from its object's compact
unwind or, lacking one, from the frame description its object carried into
`__eh_frame` ahead of this table's own. a foreign function's personality is
read through a slot the link filled or through `dyn`'s import GOT, nil for
an image that binds nothing. every function offset counts from `mh_addr`,
where the mach header is mapped

