# mach.lang.target.extension.arm64

## val SHA2

```mach
pub val SHA2: u64 = 0x1
```

the aarch64 extension vocabulary over the AdvSIMD baseline

## val SB

```mach
pub val SB: u64 = 0x2
```

FEAT_SB, the speculation barrier `sb` (Armv8.0 optional, Armv8.5 mandatory)

## val AES

```mach
pub val AES: u64 = 0x4
```

FEAT_AES, the aese, aesd, aesmc and aesimc rounds

## val PMULL

```mach
pub val PMULL: u64 = 0x8
```

FEAT_PMULL, the 64x64 carry-less pmull and pmull2. the architecture reports
it as a higher value of the one ID_AA64ISAR0_EL1.AES field, so it brings aes

## val FP16

```mach
pub val FP16: u64 = 0x10
```

FEAT_FP16, half-precision arithmetic, comparison and integer conversion on
the h registers (Armv8.2 optional). converting between half and single or
double precision is the base FP and needs no extension

## val NAME_COUNT

```mach
pub val NAME_COUNT: u32 = 5
```

## val NAMES

```mach
pub val NAMES: [NAME_COUNT]extension.Extension = [NAME_COUNT]extension.Extension;
```

