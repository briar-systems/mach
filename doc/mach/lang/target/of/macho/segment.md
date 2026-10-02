# mach.lang.target.of.macho.segment

## fun exec_segname

```mach
pub fun exec_segname(flags: u32, rebased: bool) str;
```

## rec SegmentRows

```mach
pub rec SegmentRows;
```

## fun segment_rows_free

```mach
pub fun segment_rows_free(alloc: *A.Allocator, s: *SegmentRows, seg_count: u32);
```

## fun place_segments

```mach
pub fun place_segments(alloc: *A.Allocator, fp: *of_plan.FilePlan, segs: *target_of.LoadSegment, seg_count: u32,
hdr_span: usize, page: usize, first_at_span: bool) res[SegmentRows, fail.Fail];
```

the segment rows of an image: the first segment starts at the header
reservation when `first_at_span` (its bytes follow the load commands in the
same mapping), otherwise on the next page; the rest start on a page

## fun write_segment_bytes

```mach
pub fun write_segment_bytes(buf: *u8, segs: *target_of.LoadSegment, seg_count: u32, seg_offsets: *usize);
```

## fun seg_is_data_const

```mach
pub fun seg_is_data_const(dyn: *target_of.DynamicInfo, segs: *target_of.LoadSegment, ls: u32) bool;
```

a read-only segment dyld writes into before it maps it read-only

## fun seg_vm_prot

```mach
pub fun seg_vm_prot(dyn: *target_of.DynamicInfo, segs: *target_of.LoadSegment, ls: u32) u32;
```

## fun seg_group_of

```mach
pub fun seg_group_of(dyn: *target_of.DynamicInfo, segs: *target_of.LoadSegment, ls: u32) u32;
```

## fun seg_group_first

```mach
pub fun seg_group_first(dyn: *target_of.DynamicInfo, segs: *target_of.LoadSegment, ls: u32) u32;
```

## fun seg_group_last

```mach
pub fun seg_group_last(dyn: *target_of.DynamicInfo, segs: *target_of.LoadSegment, seg_count: u32, first: u32) u32;
```

## fun seg_group_count

```mach
pub fun seg_group_count(dyn: *target_of.DynamicInfo, segs: *target_of.LoadSegment, seg_count: u32) u32;
```

## fun data_const_seg_count

```mach
pub fun data_const_seg_count(dyn: *target_of.DynamicInfo, segs: *target_of.LoadSegment, seg_count: u32) u32;
```

## fun data_const_conflict_message

```mach
pub fun data_const_conflict_message(itn: *intern.Interner, alloc: *A.Allocator, dyn: *target_of.DynamicInfo,
segs: *target_of.LoadSegment, seg_count: u32) str;
```

