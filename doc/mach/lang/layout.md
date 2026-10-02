# mach.lang.layout

## def CheckCause

```mach
pub def CheckCause: u8
```

## val CHECK_ADD_OVERFLOW

```mach
pub val CHECK_ADD_OVERFLOW:        CheckCause = 1
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

## fun id

```mach
pub fun id[Domain](value: usize) CheckedId[Domain];
```

## fun id_below

```mach
pub fun id_below[Domain](value: usize, upper: CheckedCount[Domain])
res[CheckedId[Domain], CheckCause];
```

## fun usize_to_u16

```mach
pub fun usize_to_u16(value: usize) res[u16, CheckCause];
```

## fun usize_to_u32

```mach
pub fun usize_to_u32(value: usize) res[u32, CheckCause];
```

## fun u64_to_u32

```mach
pub fun u64_to_u32(value: u64) res[u32, CheckCause];
```

## fun usize_to_i32

```mach
pub fun usize_to_i32(value: usize) res[i32, CheckCause];
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

`vector_bits` is the narrowest vector register and `register_bits` the
widest the selected extensions give, each a power of two times the one
before: 128 and 256 on x86-64 with avx (#4128)

## fun machine

```mach
pub fun machine(ptr_width: u32, vector_bits: u32, register_bits: u32) Machine;
```

## rec Node

```mach
pub rec Node;
```

## def ExtentCause

```mach
pub def ExtentCause: u8
```

## val EXTENT_UNKNOWN_NODE

```mach
pub val EXTENT_UNKNOWN_NODE:   ExtentCause = 1
```

## val EXTENT_FIELD_RANGE

```mach
pub val EXTENT_FIELD_RANGE:    ExtentCause = 5
```

## val EXTENT_SIZE_OVERFLOW

```mach
pub val EXTENT_SIZE_OVERFLOW:  ExtentCause = 6
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

## fun cause_name

```mach
pub fun cause_name(cause: ExtentCause) opt[str];
```

the text of an extent cause; absent for a tag outside the catalog

