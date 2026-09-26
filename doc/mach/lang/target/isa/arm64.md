# mach.lang.target.isa.arm64

## val HAS_FLOAT

```mach
pub val HAS_FLOAT: bool = true
```

## val EXT_SHA2

```mach
pub val EXT_SHA2: u64 = 0x1
```

the aarch64 extension vocabulary over the AdvSIMD baseline

## val EXT_SB

```mach
pub val EXT_SB: u64 = 0x2
```

FEAT_SB, the speculation barrier `sb` (Armv8.0 optional, Armv8.5 mandatory)

## val EXT_AES

```mach
pub val EXT_AES: u64 = 0x4
```

FEAT_AES, the aese, aesd, aesmc and aesimc rounds

## val EXT_PMULL

```mach
pub val EXT_PMULL: u64 = 0x8
```

FEAT_PMULL, the 64x64 carry-less pmull and pmull2. the architecture reports
it as a higher value of the one ID_AA64ISAR0_EL1.AES field, so it brings aes

## val EXTENSION_COUNT

```mach
pub val EXTENSION_COUNT: u32 = 4
```

## val EXTENSIONS

```mach
pub val EXTENSIONS: [EXTENSION_COUNT]extension.Extension = [EXTENSION_COUNT]extension.Extension;
```

## val X0

```mach
pub val X0: i32 = 0
```

## val X9

```mach
pub val X9:  i32 = 9
```

## val X10

```mach
pub val X10: i32 = 10
```

## val X11

```mach
pub val X11: i32 = 11
```

## val IP0

```mach
pub val IP0: i32 = 16
```

## val IP1

```mach
pub val IP1: i32 = 17
```

## val FP

```mach
pub val FP: i32 = 29
```

## val LR

```mach
pub val LR: i32 = 30
```

## val SP

```mach
pub val SP: i32 = 31
```

## val V0

```mach
pub val V0: i32 = 0
```

## val V30

```mach
pub val V30: i32 = 30
```

## val V31

```mach
pub val V31: i32 = 31
```

## val GPR_COUNT

```mach
pub val GPR_COUNT: i32 = 31
```

## val VECTOR_COUNT

```mach
pub val VECTOR_COUNT: i32 = 30
```

## def Opcode

```mach
pub def Opcode: u16
```

## val NOP

```mach
pub val NOP:  Opcode = 0
```

## val RET

```mach
pub val RET:  Opcode = 1
```

## val ADD

```mach
pub val ADD:  Opcode = 2
```

## val SUB

```mach
pub val SUB:  Opcode = 3
```

## val MUL

```mach
pub val MUL:  Opcode = 4
```

## val AND

```mach
pub val AND:  Opcode = 5
```

## val ORR

```mach
pub val ORR:  Opcode = 6
```

## val EOR

```mach
pub val EOR:  Opcode = 7
```

## val LSL

```mach
pub val LSL:  Opcode = 8
```

## val LSR

```mach
pub val LSR:  Opcode = 9
```

## val ASR

```mach
pub val ASR:  Opcode = 10
```

## val NEG

```mach
pub val NEG:  Opcode = 11
```

## val MVN

```mach
pub val MVN:  Opcode = 12
```

## val MOV

```mach
pub val MOV:  Opcode = 13
```

## val B

```mach
pub val B:    Opcode = 14
```

## val BRK

```mach
pub val BRK:  Opcode = 15
```

## val FMOV

```mach
pub val FMOV: Opcode = 16
```

## val UMULH

```mach
pub val UMULH: Opcode = 17
```

the high half of a 64x64 product; 64-bit only

## val SMULH

```mach
pub val SMULH: Opcode = 18
```

## val VEC_ADD

```mach
pub val VEC_ADD:  Opcode = 0x100
```

## val VEC_SUB

```mach
pub val VEC_SUB:  Opcode = 0x101
```

## val VEC_MUL

```mach
pub val VEC_MUL:  Opcode = 0x102
```

## val VEC_DIV

```mach
pub val VEC_DIV:  Opcode = 0x103
```

## val VEC_AND

```mach
pub val VEC_AND:  Opcode = 0x104
```

## val VEC_ORR

```mach
pub val VEC_ORR:  Opcode = 0x105
```

## val VEC_EOR

```mach
pub val VEC_EOR:  Opcode = 0x106
```

## val VEC_NOT

```mach
pub val VEC_NOT:  Opcode = 0x107
```

## val VEC_CMEQ

```mach
pub val VEC_CMEQ: Opcode = 0x108
```

## val VEC_CMNE

```mach
pub val VEC_CMNE: Opcode = 0x109
```

## val VEC_CMGT

```mach
pub val VEC_CMGT: Opcode = 0x10a
```

## val VEC_CMGE

```mach
pub val VEC_CMGE: Opcode = 0x10b
```

## val VEC_CMHI

```mach
pub val VEC_CMHI: Opcode = 0x10c
```

## val VEC_CMHS

```mach
pub val VEC_CMHS: Opcode = 0x10d
```

