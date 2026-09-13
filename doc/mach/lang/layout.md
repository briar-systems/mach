# mach.lang.layout

## def CheckCause

```mach
pub def CheckCause: u8
```

## val CHECK_OK

```mach
pub val CHECK_OK:                  CheckCause = 0
```

## val CHECK_ADD_OVERFLOW

```mach
pub val CHECK_ADD_OVERFLOW:        CheckCause = 1
```

## val CHECK_SUB_UNDERFLOW

```mach
pub val CHECK_SUB_UNDERFLOW:       CheckCause = 2
```

## val CHECK_MUL_OVERFLOW

```mach
pub val CHECK_MUL_OVERFLOW:        CheckCause = 3
```

## val CHECK_INVALID_ALIGNMENT

```mach
pub val CHECK_INVALID_ALIGNMENT:   CheckCause = 4
```

## val CHECK_NON_POWER_ALIGNMENT

```mach
pub val CHECK_NON_POWER_ALIGNMENT: CheckCause = 5
```

## val CHECK_INVALID_RANGE

```mach
pub val CHECK_INVALID_RANGE:       CheckCause = 6
```

## val CHECK_NARROWING

```mach
pub val CHECK_NARROWING:           CheckCause = 7
```

## val CHECK_ID_RANGE

```mach
pub val CHECK_ID_RANGE:            CheckCause = 8
```

## val CHECK_NIL_POINTER

```mach
pub val CHECK_NIL_POINTER:         CheckCause = 9
```

## val CHECK_ZERO_SIZED_ELEMENT

```mach
pub val CHECK_ZERO_SIZED_ELEMENT:  CheckCause = 10
```

## val CHECK_POINTER_ALIGNMENT

```mach
pub val CHECK_POINTER_ALIGNMENT:   CheckCause = 11
```

## val CHECK_POINTER_OVERFLOW

```mach
pub val CHECK_POINTER_OVERFLOW:    CheckCause = 12
```

## rec ByteUnit

```mach
pub rec ByteUnit;
```

## rec MemorySpace

```mach
pub rec MemorySpace;
```

## rec CheckedCount

```mach
pub rec CheckedCount[Unit];
```

## rec CheckedOffset

```mach
pub rec CheckedOffset[Unit];
```

## rec CheckedAddress

```mach
pub rec CheckedAddress[Space];
```

## rec CheckedRange

```mach
pub rec CheckedRange[Unit];
```

## rec CheckedAlignment

```mach
pub rec CheckedAlignment[Unit];
```

## rec CheckedPowerAlignment

```mach
pub rec CheckedPowerAlignment[Unit];
```

## rec CheckedId

```mach
pub rec CheckedId[Domain];
```

## rec CheckedSpan

```mach
pub rec CheckedSpan[T];
```

## fun usize_add

```mach
pub fun usize_add(left: usize, right: usize) res[usize, CheckCause];
```

## fun usize_mul

```mach
pub fun usize_mul(left: usize, right: usize) res[usize, CheckCause];
```

## fun count

```mach
pub fun count[Unit](value: usize) CheckedCount[Unit];
```

## fun offset

```mach
pub fun offset[Unit](value: usize) CheckedOffset[Unit];
```

## fun address

```mach
pub fun address[Space](value: usize) CheckedAddress[Space];
```

## fun count_add

```mach
pub fun count_add[Unit](left: CheckedCount[Unit], right: CheckedCount[Unit])
res[CheckedCount[Unit], CheckCause];
```

## fun count_sub

```mach
pub fun count_sub[Unit](left: CheckedCount[Unit], right: CheckedCount[Unit])
res[CheckedCount[Unit], CheckCause];
```

## fun count_mul

```mach
pub fun count_mul[Unit](value: CheckedCount[Unit], factor: usize)
res[CheckedCount[Unit], CheckCause];
```

## fun count_grow

```mach
pub fun count_grow[Unit](current: CheckedCount[Unit], required: CheckedCount[Unit])
res[CheckedCount[Unit], CheckCause];
```

## fun offset_add

```mach
pub fun offset_add[Unit](base: CheckedOffset[Unit], amount: CheckedCount[Unit])
res[CheckedOffset[Unit], CheckCause];
```

## fun address_add

```mach
pub fun address_add[Space](base: CheckedAddress[Space], amount: CheckedCount[ByteUnit])
res[CheckedAddress[Space], CheckCause];
```

## fun checked_range

```mach
pub fun checked_range[Unit](start: CheckedOffset[Unit], end: CheckedOffset[Unit])
res[CheckedRange[Unit], CheckCause];
```

## fun range_from_count

```mach
pub fun range_from_count[Unit](start: CheckedOffset[Unit], length: CheckedCount[Unit])
res[CheckedRange[Unit], CheckCause];
```

## fun range_count

```mach
pub fun range_count[Unit](value: CheckedRange[Unit]) res[CheckedCount[Unit], CheckCause];
```

## fun range_contains

```mach
pub fun range_contains[Unit](value: CheckedRange[Unit], position: CheckedOffset[Unit]) bool;
```

## fun range_contains_range

```mach
pub fun range_contains_range[Unit](outer: CheckedRange[Unit], inner: CheckedRange[Unit]) bool;
```

## fun range_fits

```mach
pub fun range_fits[Unit](available: CheckedCount[Unit], start: CheckedOffset[Unit],
length: CheckedCount[Unit]) bool;
```

## fun alignment

```mach
pub fun alignment[Unit](value: usize) res[CheckedAlignment[Unit], CheckCause];
```

## fun power_alignment

```mach
pub fun power_alignment[Unit](value: usize) res[CheckedPowerAlignment[Unit], CheckCause];
```

## fun general_alignment

```mach
pub fun general_alignment[Unit](value: CheckedPowerAlignment[Unit])
res[CheckedAlignment[Unit], CheckCause];
```

## fun align_offset_up

```mach
pub fun align_offset_up[Unit](value: CheckedOffset[Unit], align: CheckedAlignment[Unit])
res[CheckedOffset[Unit], CheckCause];
```

## fun align_u64_up

```mach
pub fun align_u64_up(value: u64, align: u64) res[u64, CheckCause];
```

the address-space twin of align_offset_up: a u64 value rounded up to a
power-of-two alignment, refusing an invalid alignment and an overflow

## fun align_offset_down

```mach
pub fun align_offset_down[Unit](value: CheckedOffset[Unit], align: CheckedAlignment[Unit])
res[CheckedOffset[Unit], CheckCause];
```

## fun id

```mach
pub fun id[Domain](value: usize) CheckedId[Domain];
```

## fun id_below

```mach
pub fun id_below[Domain](value: usize, upper: CheckedCount[Domain])
res[CheckedId[Domain], CheckCause];
```

## fun id_at_most

```mach
pub fun id_at_most[Domain](value: usize, maximum: CheckedId[Domain])
res[CheckedId[Domain], CheckCause];
```

## fun usize_to_u8

```mach
pub fun usize_to_u8(value: usize) res[u8, CheckCause];
```

## fun usize_to_u16

```mach
pub fun usize_to_u16(value: usize) res[u16, CheckCause];
```

## fun usize_to_u32

```mach
pub fun usize_to_u32(value: usize) res[u32, CheckCause];
```

## fun u64_to_usize

```mach
pub fun u64_to_usize(value: u64) res[usize, CheckCause];
```

## fun u64_to_u32

```mach
pub fun u64_to_u32(value: u64) res[u32, CheckCause];
```

## fun i64_to_i32

```mach
pub fun i64_to_i32(value: i64) res[i32, CheckCause];
```

## fun i64_to_u32

```mach
pub fun i64_to_u32(value: i64) res[u32, CheckCause];
```

## fun usize_to_i32

```mach
pub fun usize_to_i32(value: usize) res[i32, CheckCause];
```

## fun pointer_count

```mach
pub fun pointer_count[T](data: *T, count: usize) res[CheckedSpan[T], CheckCause];
```

## fun span_valid

```mach
pub fun span_valid[T](value: *CheckedSpan[T]) bool;
```

## fun check_cause_name

```mach
pub fun check_cause_name(cause: CheckCause) opt[str];
```

the text of a checked-arithmetic cause; absent for a tag outside the catalog

## fun read_u8

```mach
pub fun read_u8(buf: *u8, len: usize, off: usize) res[u8, CheckCause];
```

## fun read_i8

```mach
pub fun read_i8(buf: *u8, len: usize, off: usize) res[i8, CheckCause];
```

## fun write_u8

```mach
pub fun write_u8(buf: *u8, len: usize, off: usize, value: u8) res[bool, CheckCause];
```

## fun write_i8

```mach
pub fun write_i8(buf: *u8, len: usize, off: usize, value: i8) res[bool, CheckCause];
```

## fun read_u16_le

```mach
pub fun read_u16_le(buf: *u8, len: usize, off: usize) res[u16, CheckCause];
```

## fun read_u16_be

```mach
pub fun read_u16_be(buf: *u8, len: usize, off: usize) res[u16, CheckCause];
```

## fun read_i16_le

```mach
pub fun read_i16_le(buf: *u8, len: usize, off: usize) res[i16, CheckCause];
```

## fun read_i16_be

```mach
pub fun read_i16_be(buf: *u8, len: usize, off: usize) res[i16, CheckCause];
```

## fun write_u16_le

```mach
pub fun write_u16_le(buf: *u8, len: usize, off: usize, value: u16) res[bool, CheckCause];
```

## fun write_u16_be

```mach
pub fun write_u16_be(buf: *u8, len: usize, off: usize, value: u16) res[bool, CheckCause];
```

## fun write_i16_le

```mach
pub fun write_i16_le(buf: *u8, len: usize, off: usize, value: i16) res[bool, CheckCause];
```

## fun write_i16_be

```mach
pub fun write_i16_be(buf: *u8, len: usize, off: usize, value: i16) res[bool, CheckCause];
```

## fun read_u24_le

```mach
pub fun read_u24_le(buf: *u8, len: usize, off: usize) res[u32, CheckCause];
```

## fun read_u24_be

```mach
pub fun read_u24_be(buf: *u8, len: usize, off: usize) res[u32, CheckCause];
```

## fun read_i24_le

```mach
pub fun read_i24_le(buf: *u8, len: usize, off: usize) res[i32, CheckCause];
```

## fun read_i24_be

```mach
pub fun read_i24_be(buf: *u8, len: usize, off: usize) res[i32, CheckCause];
```

## fun write_u24_le

```mach
pub fun write_u24_le(buf: *u8, len: usize, off: usize, value: u32) res[bool, CheckCause];
```

## fun write_u24_be

```mach
pub fun write_u24_be(buf: *u8, len: usize, off: usize, value: u32) res[bool, CheckCause];
```

## fun write_i24_le

```mach
pub fun write_i24_le(buf: *u8, len: usize, off: usize, value: i32) res[bool, CheckCause];
```

## fun write_i24_be

```mach
pub fun write_i24_be(buf: *u8, len: usize, off: usize, value: i32) res[bool, CheckCause];
```

## fun read_u32_le

```mach
pub fun read_u32_le(buf: *u8, len: usize, off: usize) res[u32, CheckCause];
```

## fun read_u32_be

```mach
pub fun read_u32_be(buf: *u8, len: usize, off: usize) res[u32, CheckCause];
```

## fun read_i32_le

```mach
pub fun read_i32_le(buf: *u8, len: usize, off: usize) res[i32, CheckCause];
```

## fun read_i32_be

```mach
pub fun read_i32_be(buf: *u8, len: usize, off: usize) res[i32, CheckCause];
```

## fun write_u32_le

```mach
pub fun write_u32_le(buf: *u8, len: usize, off: usize, value: u32) res[bool, CheckCause];
```

## fun write_u32_be

```mach
pub fun write_u32_be(buf: *u8, len: usize, off: usize, value: u32) res[bool, CheckCause];
```

## fun write_i32_le

```mach
pub fun write_i32_le(buf: *u8, len: usize, off: usize, value: i32) res[bool, CheckCause];
```

## fun write_i32_be

```mach
pub fun write_i32_be(buf: *u8, len: usize, off: usize, value: i32) res[bool, CheckCause];
```

## fun read_u64_le

```mach
pub fun read_u64_le(buf: *u8, len: usize, off: usize) res[u64, CheckCause];
```

## fun read_u64_be

```mach
pub fun read_u64_be(buf: *u8, len: usize, off: usize) res[u64, CheckCause];
```

## fun read_i64_le

```mach
pub fun read_i64_le(buf: *u8, len: usize, off: usize) res[i64, CheckCause];
```

## fun read_i64_be

```mach
pub fun read_i64_be(buf: *u8, len: usize, off: usize) res[i64, CheckCause];
```

## fun write_u64_le

```mach
pub fun write_u64_le(buf: *u8, len: usize, off: usize, value: u64) res[bool, CheckCause];
```

## fun write_u64_be

```mach
pub fun write_u64_be(buf: *u8, len: usize, off: usize, value: u64) res[bool, CheckCause];
```

## fun write_i64_le

```mach
pub fun write_i64_le(buf: *u8, len: usize, off: usize, value: i64) res[bool, CheckCause];
```

## fun write_i64_be

```mach
pub fun write_i64_be(buf: *u8, len: usize, off: usize, value: i64) res[bool, CheckCause];
```

## def NodeKind

```mach
pub def NodeKind: u8
```

## val NODE_UNKNOWN

```mach
pub val NODE_UNKNOWN:   NodeKind = 0
```

## val NODE_SCALAR

```mach
pub val NODE_SCALAR:    NodeKind = 1
```

## val NODE_POINTER

```mach
pub val NODE_POINTER:   NodeKind = 2
```

## val NODE_VECTOR

```mach
pub val NODE_VECTOR:    NodeKind = 3
```

## val NODE_ARRAY

```mach
pub val NODE_ARRAY:     NodeKind = 4
```

## val NODE_AGGREGATE

```mach
pub val NODE_AGGREGATE: NodeKind = 5
```

## val NODE_ZERO

```mach
pub val NODE_ZERO:      NodeKind = 6
```

## val NODE_OPAQUE

```mach
pub val NODE_OPAQUE:    NodeKind = 7
```

## val NODE_TAG

```mach
pub val NODE_TAG:       NodeKind = 8
```

## rec Machine

```mach
pub rec Machine;
```

## fun machine

```mach
pub fun machine(ptr_width: u32, vector_bits: u32) Machine;
```

## rec Node

```mach
pub rec Node;
```

## def ExtentCause

```mach
pub def ExtentCause: u8
```

## val EXTENT_OK

```mach
pub val EXTENT_OK:             ExtentCause = 0
```

## val EXTENT_UNKNOWN_NODE

```mach
pub val EXTENT_UNKNOWN_NODE:   ExtentCause = 1
```

## val EXTENT_DEPTH

```mach
pub val EXTENT_DEPTH:          ExtentCause = 2
```

## val EXTENT_ARRAY_OVERFLOW

```mach
pub val EXTENT_ARRAY_OVERFLOW: ExtentCause = 3
```

## val EXTENT_NOT_AGGREGATE

```mach
pub val EXTENT_NOT_AGGREGATE:  ExtentCause = 4
```

## val EXTENT_FIELD_RANGE

```mach
pub val EXTENT_FIELD_RANGE:    ExtentCause = 5
```

## val EXTENT_SIZE_OVERFLOW

```mach
pub val EXTENT_SIZE_OVERFLOW:  ExtentCause = 6
```

## val EXTENT_INVALID_ALIGN

```mach
pub val EXTENT_INVALID_ALIGN:  ExtentCause = 7
```

## val EXTENT_INVALID_GRAPH

```mach
pub val EXTENT_INVALID_GRAPH:  ExtentCause = 8
```

## val EXTENT_EMPTY_TAG

```mach
pub val EXTENT_EMPTY_TAG:      ExtentCause = 9
```

## rec Extent

```mach
pub rec Extent;
```

## val DEPTH_UNBOUNDED

```mach
pub val DEPTH_UNBOUNDED: u32 = 0
```

## fun unknown_extent

```mach
pub fun unknown_extent(cause: ExtentCause) Extent;
```

## fun extent

```mach
pub fun extent(size: u32, align: u32) Extent;
```

## fun node

```mach
pub fun node(kind: NodeKind) Node;
```

## fun checked_round_up

```mach
pub fun checked_round_up(value: u32, align: u32) Extent;
```

## fun extent_of

```mach
pub fun extent_of[T](ctx: *T, describe: fun(*T, u32) Node, field: fun(*T, u32, u32) u32,
id: u32, m: Machine, max_depth: u32) Extent;
```

## fun field_offset_of

```mach
pub fun field_offset_of[T](ctx: *T, describe: fun(*T, u32) Node,
field: fun(*T, u32, u32) u32, id: u32, field_ix: u32,
m: Machine, max_depth: u32) Extent;
```

## fun discriminator_bytes

```mach
pub fun discriminator_bytes(case_count: u64) u32;
```

the narrowest discriminator that can number every case

## fun cause_name

```mach
pub fun cause_name(cause: ExtentCause) opt[str];
```

the text of an extent cause; absent for a tag outside the catalog

