# mach.lang.checked

checked arithmetic: counts, offsets, addresses, alignments and ids typed by
their unit, and the narrowing conversions, each refusing overflow with a cause

## def Cause

```mach
pub def Cause: u8
```

## val ADD_OVERFLOW

```mach
pub val ADD_OVERFLOW:        Cause = 1
```

## rec ByteUnit

```mach
pub rec ByteUnit;
```

## rec MemorySpace

```mach
pub rec MemorySpace;
```

## rec Count

```mach
pub rec Count[Unit];
```

## rec Offset

```mach
pub rec Offset[Unit];
```

## rec Address

```mach
pub rec Address[Space];
```

## rec Alignment

```mach
pub rec Alignment[Unit];
```

## rec PowerAlignment

```mach
pub rec PowerAlignment[Unit];
```

## rec Id

```mach
pub rec Id[Domain];
```

## fun usize_add

```mach
pub fun usize_add(left: usize, right: usize) res[usize, Cause];
```

## fun usize_mul

```mach
pub fun usize_mul(left: usize, right: usize) res[usize, Cause];
```

## fun count

```mach
pub fun count[Unit](value: usize) Count[Unit];
```

## fun offset

```mach
pub fun offset[Unit](value: usize) Offset[Unit];
```

## fun address

```mach
pub fun address[Space](value: usize) Address[Space];
```

## fun count_add

```mach
pub fun count_add[Unit](left: Count[Unit], right: Count[Unit])
res[Count[Unit], Cause];
```

## fun count_mul

```mach
pub fun count_mul[Unit](value: Count[Unit], factor: usize)
res[Count[Unit], Cause];
```

## fun offset_add

```mach
pub fun offset_add[Unit](base: Offset[Unit], amount: Count[Unit])
res[Offset[Unit], Cause];
```

## fun address_add

```mach
pub fun address_add[Space](base: Address[Space], amount: Count[ByteUnit])
res[Address[Space], Cause];
```

## fun range_fits

```mach
pub fun range_fits[Unit](available: Count[Unit], start: Offset[Unit],
length: Count[Unit]) bool;
```

## fun alignment

```mach
pub fun alignment[Unit](value: usize) res[Alignment[Unit], Cause];
```

## fun power_alignment

```mach
pub fun power_alignment[Unit](value: usize) res[PowerAlignment[Unit], Cause];
```

## fun align_offset_up

```mach
pub fun align_offset_up[Unit](value: Offset[Unit], align: Alignment[Unit])
res[Offset[Unit], Cause];
```

## fun align_u64_up

```mach
pub fun align_u64_up(value: u64, align: u64) res[u64, Cause];
```

the address-space twin of align_offset_up: a u64 value rounded up to a
power-of-two alignment, refusing an invalid alignment and an overflow

## fun id

```mach
pub fun id[Domain](value: usize) Id[Domain];
```

## fun id_below

```mach
pub fun id_below[Domain](value: usize, upper: Count[Domain])
res[Id[Domain], Cause];
```

## fun usize_to_u16

```mach
pub fun usize_to_u16(value: usize) res[u16, Cause];
```

## fun usize_to_u32

```mach
pub fun usize_to_u32(value: usize) res[u32, Cause];
```

## fun u64_to_u32

```mach
pub fun u64_to_u32(value: u64) res[u32, Cause];
```

## fun usize_to_i32

```mach
pub fun usize_to_i32(value: usize) res[i32, Cause];
```

