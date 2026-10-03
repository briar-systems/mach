# mach.lang.target.isa.arm64

## val X0

```mach
pub val X0:  i32 = 0
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
pub val V0:  i32 = 0
```

## val V30

```mach
pub val V30: i32 = 30
```

## val V31

```mach
pub val V31: i32 = 31
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

## val VEC_SHL

```mach
pub val VEC_SHL:   Opcode = 0x10e
```

the lane-wise shifts, by an immediate or by a uniform or per-lane count

## val VEC_SHR_U

```mach
pub val VEC_SHR_U: Opcode = 0x10f
```

## val VEC_SHR_S

```mach
pub val VEC_SHR_S: Opcode = 0x110
```

## rec SysReg

```mach
pub rec SysReg;
```

a system register the inline assembly names, by its encoding fields

## val SYSREG_COUNT

```mach
pub val SYSREG_COUNT: u32 = 44
```

## val SYSREGS

```mach
pub val SYSREGS: [SYSREG_COUNT]SysReg = [SYSREG_COUNT]SysReg;
```

## rec PstateField

```mach
pub rec PstateField;
```

a PSTATE field msr writes by immediate, by its op1 and op2: the one table the
listing prints from and the inline assembly grammar reads

## val PSTATE_FIELD_COUNT

```mach
pub val PSTATE_FIELD_COUNT: u32 = 8
```

## val PSTATE_FIELDS

```mach
pub val PSTATE_FIELDS: [PSTATE_FIELD_COUNT]PstateField = [PSTATE_FIELD_COUNT]PstateField;
```

