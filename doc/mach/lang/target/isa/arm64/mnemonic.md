# mach.lang.target.isa.arm64.mnemonic

the inline assembly mnemonic rows: each spelling, its operand pattern and the registers it clobbers

## val PATTERN_NULLARY

```mach
pub val PATTERN_NULLARY: u16 = 1
```

## val PATTERN_RET_REG

```mach
pub val PATTERN_RET_REG: u16 = 2
```

## val PATTERN_IMM16

```mach
pub val PATTERN_IMM16: u16 = 3
```

## val PATTERN_MOV

```mach
pub val PATTERN_MOV: u16 = 4
```

## val PATTERN_MOVEWIDE

```mach
pub val PATTERN_MOVEWIDE: u16 = 5
```

## val PATTERN_ADDSUB

```mach
pub val PATTERN_ADDSUB: u16 = 6
```

## val PATTERN_LOGICAL

```mach
pub val PATTERN_LOGICAL: u16 = 7
```

## val PATTERN_CMP

```mach
pub val PATTERN_CMP: u16 = 8
```

## val PATTERN_CSET

```mach
pub val PATTERN_CSET: u16 = 9
```

## val PATTERN_ADRP

```mach
pub val PATTERN_ADRP: u16 = 10
```

## val PATTERN_BL

```mach
pub val PATTERN_BL: u16 = 11
```

## val PATTERN_BR_REG

```mach
pub val PATTERN_BR_REG: u16 = 12
```

## val PATTERN_B

```mach
pub val PATTERN_B: u16 = 13
```

## val PATTERN_BCOND

```mach
pub val PATTERN_BCOND: u16 = 14
```

## val PATTERN_CBR

```mach
pub val PATTERN_CBR: u16 = 15
```

## val PATTERN_LDST

```mach
pub val PATTERN_LDST: u16 = 16
```

## val PATTERN_LDSTP

```mach
pub val PATTERN_LDSTP: u16 = 17
```

## val PATTERN_DMB

```mach
pub val PATTERN_DMB: u16 = 18
```

## val PATTERN_LDORD

```mach
pub val PATTERN_LDORD: u16 = 19
```

## val PATTERN_STLXR

```mach
pub val PATTERN_STLXR: u16 = 20
```

## val PATTERN_SHIFT

```mach
pub val PATTERN_SHIFT: u16 = 21
```

## val PATTERN_MSR

```mach
pub val PATTERN_MSR: u16 = 22
```

## val PATTERN_MRS

```mach
pub val PATTERN_MRS: u16 = 23
```

## val PATTERN_V3

```mach
pub val PATTERN_V3: u16 = 24
```

## val PATTERN_V2

```mach
pub val PATTERN_V2: u16 = 25
```

## val PATTERN_VELEM

```mach
pub val PATTERN_VELEM: u16 = 26
```

## val PATTERN_VCRYPTO

```mach
pub val PATTERN_VCRYPTO: u16 = 27
```

the fixed-arrangement crypto rows (sha2, aes, pmull): the operands the
member's mop.CRYPTO_ROWS entry names

## val PATTERN_ISB

```mach
pub val PATTERN_ISB: u16 = 28
```

isb: bare, or the one option `sy` the architecture defines

## val PATTERN_MULH

```mach
pub val PATTERN_MULH: u16 = 29
```

the high multiplies: three x registers, no w form

## val PATTERN_REV

```mach
pub val PATTERN_REV: u16 = 30
```

rev: two general registers of one width

## val PATTERN_VEXT

```mach
pub val PATTERN_VEXT: u16 = 31
```

ext: three .16b registers and the first byte, 0 to 15

## val PATTERN_VDUP

```mach
pub val PATTERN_VDUP: u16 = 32
```

dup: a vector at an arrangement from one lane of that width or a general register

## val PATTERN_VINS

```mach
pub val PATTERN_VINS: u16 = 33
```

ins, and the mov spelling of it: one lane from a lane of that width or a general register

## val COND_SHIFT

```mach
pub val COND_SHIFT: u16 = 8
```

## val COND_MASK

```mach
pub val COND_MASK: u16 = 0x0F00
```

## val CALLER_SAVED

```mach
pub val CALLER_SAVED: u32 = 0x4007FFFF
```

## val SMCCC_CLOBBER

```mach
pub val SMCCC_CLOBBER: u32 = 0x0003FFFF
```

## val ROW_COUNT

```mach
pub val ROW_COUNT: usize = 86
```

## val ROWS

```mach
pub val ROWS: [ROW_COUNT]isa_asm.Mnemonic = [ROW_COUNT]isa_asm.Mnemonic;
```

## val VEC_TWIN_COUNT

```mach
pub val VEC_TWIN_COUNT: usize = 6
```

the vector members that share a spelling with a general-purpose row; vector operands select them

## val VEC_TWINS

```mach
pub val VEC_TWINS: [VEC_TWIN_COUNT]isa_asm.Mnemonic = [VEC_TWIN_COUNT]isa_asm.Mnemonic;
```

## fun pattern

```mach
pub fun pattern(flags: u16) u16;
```

## fun cond

```mach
pub fun cond(flags: u16) u32;
```

