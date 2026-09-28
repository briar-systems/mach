# mach.lang.target.of.cfi

dwarf call frame information from the linker's frame records: how each isa's
registers read in cfi, the frame a function's prologue steps leave, and the
`.eh_frame` and `.eh_frame_hdr` tables elf images carry and mach-o falls back
to where compact unwind cannot say a frame. a table's size depends on the
steps alone, so the linker reserves it before layout and the writer fills it
after

## rec FrameIsa

```mach
pub rec FrameIsa;
```

how one isa spells its frame in cfi: dwarf register numbers, the return
address column and the frame a call leaves

## fun frame_isa

```mach
pub fun frame_isa(arch: u32) opt[FrameIsa];
```

## fun dwarf_gp

```mach
pub fun dwarf_gp(fi: *FrameIsa, reg: u8) u32;
```

## fun dwarf_vec

```mach
pub fun dwarf_vec(fi: *FrameIsa, reg: u8) u32;
```

## rec FrameSave

```mach
pub rec FrameSave;
```

where the body finds a saved register: at an offset from the call frame
address, or from the stack pointer when a realigned prologue left the
frame's distance from it unknown

## rec FrameState

```mach
pub rec FrameState;
```

the frame a function's prologue leaves: the call frame address as a register
plus an offset, and every register the prologue saved

## val DW_EH_PE_PCREL_SDATA4

```mach
pub val DW_EH_PE_PCREL_SDATA4:   u8 = 0x1B
```

pc-relative, signed four-byte pointers

## val DW_EH_PE_UDATA4

```mach
pub val DW_EH_PE_UDATA4:         u8 = 0x03
```

## val DW_EH_PE_DATAREL_SDATA4

```mach
pub val DW_EH_PE_DATAREL_SDATA4: u8 = 0x3B
```

## fun frame_state

```mach
pub fun frame_state(fi: *FrameIsa, steps: *of.FrameStep, count: u32, st: *FrameState) err[fail.Fail];
```

the frame a function's steps leave once its prologue has run

## fun cie_size

```mach
pub fun cie_size(fi: *FrameIsa) usize;
```

## fun fde_size

```mach
pub fun fde_size(fi: *FrameIsa, steps: *of.FrameStep, count: u32) res[usize, fail.Fail];
```

## rec UnwindFn

```mach
pub rec UnwindFn;
```

a function an unwind table describes, at its final address

## fun write_eh_frame

```mach
pub fun write_eh_frame(fi: *FrameIsa, buf: *u8, cap: usize, vaddr: u64, fns: *UnwindFn, count: u32,
fde_offs: *u32) res[usize, fail.Fail];
```

the `.eh_frame` bytes for `fns` in address order: one cie, an fde each and
the zero terminator. `fde_offs`, when given, receives each fde's offset

## fun eh_frame_bound

```mach
pub fun eh_frame_bound(fi: *FrameIsa, frames: *of.FrameUnwind, count: u32) res[usize, fail.Fail];
```

the most `.eh_frame` bytes the given frames can take: a function the link
keeps has at most one frame record

## fun eh_frame_hdr_size

```mach
pub fun eh_frame_hdr_size(count: u32) usize;
```

## rec HdrRow

```mach
pub rec HdrRow;
```

a row of `.eh_frame_hdr`'s search table: a function's start and its fde

## fun write_eh_frame_hdr

```mach
pub fun write_eh_frame_hdr(buf: *u8, cap: usize, vaddr: u64, eh_vaddr: u64, rows: *HdrRow, count: u32) err[fail.Fail];
```

`.eh_frame_hdr`: the frame table's address and a search table of each
function's start and its fde, both relative to the header, in address order

## rec UnwindRoom

```mach
pub rec UnwindRoom;
```

the room the linker reserved for one unwind table, found by its section flag

## fun unwind_room

```mach
pub fun unwind_room(segs: *of.LoadSegment, seg_count: u32, flag: u32) UnwindRoom;
```

## fun unwind_room_trim

```mach
pub fun unwind_room_trim(segs: *of.LoadSegment, room: *UnwindRoom, len: usize);
```

the table fills `len` bytes of its room, which is what its section header says

## fun unwind_functions

```mach
pub fun unwind_functions(alloc: *A.Allocator, segs: *of.LoadSegment, seg_count: u32,
funcs: *of.ExecFunction, func_count: u32, count: *u32) res[*UnwindFn, fail.Fail];
```

the functions with frame records at their final addresses, in address order
with an alias of an earlier start dropped; `count` entries the caller frees

## val DW_EH_PE_OMIT

```mach
pub val DW_EH_PE_OMIT: u8 = 0xFF
```

## rec EhEntry

```mach
pub rec EhEntry;
```

one entry of the stream: `off` is where its length word sits and `size` the
whole entry with that word. `cie_off` is the cie an fde names

## fun eh_entry

```mach
pub fun eh_entry(buf: *u8, len: usize, off: usize) res[opt[EhEntry], fail.Fail];
```

the entry at `off`, none at a zero terminator or where the bytes end

## rec EhCie

```mach
pub rec EhCie;
```

what a cie says about the pointers of its fdes: how each fde's function start
is encoded and whether an lsda pointer follows in each fde's augmentation

## fun eh_cie

```mach
pub fun eh_cie(buf: *u8, e: *EhEntry, ptr_size: u32) res[EhCie, fail.Fail];
```

## rec EhFde

```mach
pub rec EhFde;
```

where an fde's pointer fields sit: the function start and, when its cie
names one, the lsda. an absent lsda field has size 0

## fun eh_fde

```mach
pub fun eh_fde(buf: *u8, e: *EhEntry, cie: *EhCie, ptr_size: u32) res[EhFde, fail.Fail];
```

## fun eh_read

```mach
pub fun eh_read(buf: *u8, field: usize, size: usize, form: u8) i64;
```

a field's value as its encoding's form reads it, sign-extended for a signed form

## fun eh_pointer

```mach
pub fun eh_pointer(raw: i64, enc: u8, field_vaddr: u64) res[u64, fail.Fail];
```

the address an encoded pointer names, the field lying at `field_vaddr`

## rec EhFn

```mach
pub rec EhFn;
```

a function a carried frame description covers, and where that fde lies

## fun eh_frame_scan

```mach
pub fun eh_frame_scan(alloc: *A.Allocator, buf: *u8, cap: usize, vaddr: u64, ptr_size: u32,
fns: **EhFn, count: *u32) res[usize, fail.Fail];
```

the frame descriptions the link's foreign objects left at the start of a
frames table mapped at `vaddr`: the offset their stream ends at, which is
where the table's own entries go, and in `fns` each fde's function, `count`
entries the caller frees

