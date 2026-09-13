# mach.lang.target.of.plan

## rec FileUnit

```mach
pub rec FileUnit;
```

## rec Region

```mach
pub rec Region;
```

## rec FilePlan

```mach
pub rec FilePlan;
```

## val NO_ROW

```mach
pub val NO_ROW: u32 = 0xFFFFFFFF
```

## val LIMIT_32

```mach
pub val LIMIT_32: usize = 0xFFFFFFFF
```

## fun limit_64

```mach
pub fun limit_64() usize;
```

## fun init

```mach
pub fun init(alloc: *A.Allocator, itn: *intern.Interner, format: str, limit: usize) res[FilePlan, fail.Fail];
```

## fun dnit

```mach
pub fun dnit(p: *FilePlan);
```

## fun place

```mach
pub fun place(p: *FilePlan, label: str, section: of.SectionId, align: usize, size: usize) res[u32, fail.Fail];
```

place a region at the next offset that satisfies `align`

## fun place_at

```mach
pub fun place_at(p: *FilePlan, label: str, section: of.SectionId, offset: usize, size: usize) res[u32, fail.Fail];
```

place a region whose offset is dictated; it must not overlap what came before

## fun pad_to

```mach
pub fun pad_to(p: *FilePlan, label: str, offset: usize) err[fail.Fail];
```

advance the cursor to a dictated offset without placing a region

## fun pad

```mach
pub fun pad(p: *FilePlan, label: str, align: usize) err[fail.Fail];
```

advance the cursor to `align` without placing a region; the gap is padding

## fun seal

```mach
pub fun seal(p: *FilePlan) res[usize, fail.Fail];
```

## fun cursor

```mach
pub fun cursor(p: *FilePlan) usize;
```

the next unplaced offset

## fun total

```mach
pub fun total(p: *FilePlan) usize;
```

## fun row_count

```mach
pub fun row_count(p: *FilePlan) u32;
```

## fun offset

```mach
pub fun offset(p: *FilePlan, row: u32) usize;
```

## fun size

```mach
pub fun size(p: *FilePlan, row: u32) usize;
```

## fun end

```mach
pub fun end(p: *FilePlan, row: u32) usize;
```

## fun label

```mach
pub fun label(p: *FilePlan, row: u32) str;
```

## fun section

```mach
pub fun section(p: *FilePlan, row: u32) of.SectionId;
```

## fun close

```mach
pub fun close(p: *FilePlan, row: u32, cursor: usize) err[fail.Fail];
```

a region written through a cursor must end exactly where it was planned

## fun close_size

```mach
pub fun close_size(p: *FilePlan, row: u32, written: usize) err[fail.Fail];
```

a region filled by a writer that reports the bytes it produced

## fun field_u32

```mach
pub fun field_u32(p: *FilePlan, value: usize, what: str) res[u32, fail.Fail];
```

## fun field_u16

```mach
pub fun field_u16(p: *FilePlan, value: usize, what: str) res[u16, fail.Fail];
```

## fun field_u8

```mach
pub fun field_u8(p: *FilePlan, value: usize, what: str) res[u8, fail.Fail];
```

## fun field_u64_u32

```mach
pub fun field_u64_u32(p: *FilePlan, value: u64, what: str) res[u32, fail.Fail];
```

## fun address

```mach
pub fun address(p: *FilePlan, row: u32, file_base: usize, address_base: u64) res[u64, fail.Fail];
```

a checked address: `base + (file offset - file base)`, the mapped twin of a row

## fun align_bytes

```mach
pub fun align_bytes(value: usize, align: usize) res[usize, fail.Fail];
```

a checked alignment for a writer that has no plan of its own: the same
primitive, refusing an invalid alignment and an overflowing round-up

## fun align_up

```mach
pub fun align_up(p: *FilePlan, label: str, value: usize, align: usize) res[usize, fail.Fail];
```

a checked aligned size, the size-side twin of `place`

## fun table

```mach
pub fun table(p: *FilePlan, label: str, count: usize, entry: usize) res[usize, fail.Fail];
```

a checked count-times-size product for a table region

## fun sum

```mach
pub fun sum(p: *FilePlan, label: str, left: usize, right: usize) res[usize, fail.Fail];
```

a checked sum for a size accumulated before placement

## fun count_u32

```mach
pub fun count_u32(p: *FilePlan, label: str, left: u32, right: u32) res[u32, fail.Fail];
```

a checked u32 count that a format field carries

## fun align_address

```mach
pub fun align_address(p: *FilePlan, label: str, value: u64, align: u64) res[u64, fail.Fail];
```

an aligned address in a mapped space, the address-side twin of `place`

