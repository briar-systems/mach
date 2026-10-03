# mach.lang.target.abi.sysv

## val EB_SSE_LO

```mach
pub val EB_SSE_LO:    u8 = 1
```

the eightbyte classes of a small aggregate, the mask a classifier reads:
bit 0 and bit 1 mark eightbyte 0 and 1 as SSE, bit 2 and bit 3 mark them as
holding no leaf at all (padding only), and an unmarked eightbyte inside the
object is INTEGER; bit 4 marks an aggregate holding a field at an offset that
is not a multiple of the field's own alignment (a packed record), which is
MEMORY regardless of what its eightbytes hold

## val EB_SSE_HI

```mach
pub val EB_SSE_HI:    u8 = 2
```

## val EB_EMPTY_LO

```mach
pub val EB_EMPTY_LO:  u8 = 4
```

## val EB_EMPTY_HI

```mach
pub val EB_EMPTY_HI:  u8 = 8
```

## val EB_UNALIGNED

```mach
pub val EB_UNALIGNED: u8 = 16
```

## fun eightbyte_mask

```mach
pub fun eightbyte_mask(vt: *abi.AbiVTable, view: *abi.TypeView, ty: u32) u8;
```

the eightbyte mask of `ty`, 0 for a type that is no aggregate of one or two eightbytes

## val VTABLE

```mach
pub val VTABLE: abi.AbiVTable = abi.AbiVTable;
```

