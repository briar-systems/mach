[mach](../README.md) / [base](README.md) / **checked**

# mach.base.checked

checked arithmetic: counts, offsets, addresses, alignments and ids typed by
their unit, and the narrowing conversions, each refusing overflow with a cause

<sub>source: [`src/base/checked.mach`](../../../src/base/checked.mach) · imported by 1 module</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Cause`](#cause) | `def` |  |
| [`ByteUnit`](#byteunit) | `rec` |  |
| [`MemorySpace`](#memoryspace) | `rec` |  |
| [`Count`](#count) | `rec` |  |
| [`Offset`](#offset) | `rec` |  |
| [`Address`](#address) | `rec` |  |
| [`Alignment`](#alignment) | `rec` |  |
| [`PowerAlignment`](#poweralignment) | `rec` |  |
| [`Id`](#id) | `rec` |  |

| On `Cause` | |
| :--- | :--- |
| [`ADD_OVERFLOW`](#cause) · [`usize_add`](#usize_add) · [`usize_mul`](#usize_mul) | |
| [`align_u64_up`](#align_u64_up) | the address-space twin of align_offset_up: a u64 value rounded up to a power-of-two alignment, refusing an invalid alignment and an overflow |
| [`usize_to_u16`](#usize_to_u16) · [`usize_to_u32`](#usize_to_u32) · [`u64_to_u32`](#u64_to_u32) · [`usize_to_i32`](#usize_to_i32) | |

| On `Count` | |
| :--- | :--- |
| [`count`](#count-1) · [`count_add`](#count_add) · [`count_mul`](#count_mul) · [`range_fits`](#range_fits) · [`id_below`](#id_below) | |

| On `Offset` | |
| :--- | :--- |
| [`offset`](#offset-1) · [`offset_add`](#offset_add) · [`align_offset_up`](#align_offset_up) | |

| On `Address` | |
| :--- | :--- |
| [`address`](#address-1) · [`address_add`](#address_add) | |

| On `Alignment` | |
| :--- | :--- |
| [`alignment`](#alignment-1) | |

| On `PowerAlignment` | |
| :--- | :--- |
| [`power_alignment`](#power_alignment) | |

| On `Id` | |
| :--- | :--- |
| [`id`](#id-1) | |

---

## Cause

```mach
pub def Cause: u8
```

| Name | Value | |
| :--- | :--- | :--- |
| `ADD_OVERFLOW` | `1` |  |

### usize_add

```mach
pub fun usize_add(left: usize, right: usize) res[usize, Cause]
```

### usize_mul

```mach
pub fun usize_mul(left: usize, right: usize) res[usize, Cause]
```

### align_u64_up

```mach
pub fun align_u64_up(value: u64, align: u64) res[u64, Cause]
```

the address-space twin of align_offset_up: a u64 value rounded up to a
power-of-two alignment, refusing an invalid alignment and an overflow

### usize_to_u16

```mach
pub fun usize_to_u16(value: usize) res[u16, Cause]
```

### usize_to_u32

```mach
pub fun usize_to_u32(value: usize) res[u32, Cause]
```

### u64_to_u32

```mach
pub fun u64_to_u32(value: u64) res[u32, Cause]
```

### usize_to_i32

```mach
pub fun usize_to_i32(value: usize) res[i32, Cause]
```

## ByteUnit

```mach
pub rec ByteUnit {}
```

## MemorySpace

```mach
pub rec MemorySpace {}
```

## Count

```mach
pub rec Count[Unit] {
    value: usize;
}
```

### count

```mach
pub fun count[Unit](value: usize) Count[Unit]
```

### count_add

```mach
pub fun count_add[Unit](left: Count[Unit], right: Count[Unit])
    res[Count[Unit], Cause]
```

### count_mul

```mach
pub fun count_mul[Unit](value: Count[Unit], factor: usize)
    res[Count[Unit], Cause]
```

### range_fits

```mach
pub fun range_fits[Unit](available: Count[Unit], start: Offset[Unit],
    length: Count[Unit]) bool
```

### id_below

```mach
pub fun id_below[Domain](value: usize, upper: Count[Domain])
    res[Id[Domain], Cause]
```

## Offset

```mach
pub rec Offset[Unit] {
    value: usize;
}
```

### offset

```mach
pub fun offset[Unit](value: usize) Offset[Unit]
```

### offset_add

```mach
pub fun offset_add[Unit](base: Offset[Unit], amount: Count[Unit])
    res[Offset[Unit], Cause]
```

### align_offset_up

```mach
pub fun align_offset_up[Unit](value: Offset[Unit], align: Alignment[Unit])
    res[Offset[Unit], Cause]
```

## Address

```mach
pub rec Address[Space] {
    value: usize;
}
```

### address

```mach
pub fun address[Space](value: usize) Address[Space]
```

### address_add

```mach
pub fun address_add[Space](base: Address[Space], amount: Count[ByteUnit])
    res[Address[Space], Cause]
```

## Alignment

```mach
pub rec Alignment[Unit] {
    value: usize;
}
```

### alignment

```mach
pub fun alignment[Unit](value: usize) res[Alignment[Unit], Cause]
```

## PowerAlignment

```mach
pub rec PowerAlignment[Unit] {
    value: usize;
}
```

### power_alignment

```mach
pub fun power_alignment[Unit](value: usize) res[PowerAlignment[Unit], Cause]
```

## Id

```mach
pub rec Id[Domain] {
    value: usize;
}
```

### id

```mach
pub fun id[Domain](value: usize) Id[Domain]
```

