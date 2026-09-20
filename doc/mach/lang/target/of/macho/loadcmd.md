# mach.lang.target.of.macho.loadcmd

## fun header_span

```mach
pub fun header_span(shape: *of.HeaderShape, page: u64) u64;
```

## fun dylinker_cmd_size

```mach
pub fun dylinker_cmd_size() usize;
```

## fun dylib_cmd_size

```mach
pub fun dylib_cmd_size(name: str) usize;
```

## fun rpath_cmd_size

```mach
pub fun rpath_cmd_size(name: str) usize;
```

## fun write_load_dylinker

```mach
pub fun write_load_dylinker(buf: *u8, off: usize) usize;
```

## fun write_load_dylib

```mach
pub fun write_load_dylib(buf: *u8, off: usize, name: str) usize;
```

## fun write_rpath

```mach
pub fun write_rpath(buf: *u8, off: usize, name: str) usize;
```

## fun rpath_seen_before

```mach
pub fun rpath_seen_before(libs: *of.DynLib, end: u32, rpath: intern.StrId) bool;
```

## fun write_unixthread

```mach
pub fun write_unixthread(buf: *u8, off: usize, arch_id: u32, entry: u64, cmd_size: usize) usize;
```

## fun write_segment_cmd

```mach
pub fun write_segment_cmd(buf: *u8, off: usize, name: str, vmaddr: u64, vmsize: u64,
fileoff: u64, filesize: u64, maxprot: u32, initprot: u32, nsects: u32) usize;
```

## fun write_section_64

```mach
pub fun write_section_64(buf: *u8, off: usize, sectname: str, segname: str, addr: u64, size: u64,
fileoff: u32, align: u32, flags: u32) usize;
```

## fun frame_cmd_count

```mach
pub fun frame_cmd_count(dyn: *of.DynamicInfo, segs: *of.LoadSegment, seg_count: u32, pagezero: bool) u32;
```

## fun frame_cmd_bytes

```mach
pub fun frame_cmd_bytes(dyn: *of.DynamicInfo, segs: *of.LoadSegment, seg_count: u32, pagezero: bool) usize;
```

## fun frame_sect_total

```mach
pub fun frame_sect_total(segs: *of.LoadSegment, seg_count: u32) u32;
```

## fun write_frame_cmds

```mach
pub fun write_frame_cmds(buf: *u8, fp: *plan.FilePlan, lc: usize, itn: *intern.Interner, dyn: *of.DynamicInfo,
segs: *of.LoadSegment, seg_count: u32, seg_offsets: *usize,
pagezero_size: u64, text_va: u64, hdr_span: usize, exec_page: u64) res[usize, fail.Fail];
```

## fun write_uuid_cmd

```mach
pub fun write_uuid_cmd(buf: *u8, lc: usize) usize;
```

an LC_UUID whose payload is filled by seal_build_id once the image is complete

