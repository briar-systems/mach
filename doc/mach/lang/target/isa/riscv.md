# mach.lang.target.isa.riscv

## val HAS_FLOAT

```mach
pub val HAS_FLOAT: bool = true
```

## val X0

```mach
pub val X0:  i32 = 0
```

## val X1

```mach
pub val X1:  i32 = 1
```

## val X2

```mach
pub val X2:  i32 = 2
```

## val X3

```mach
pub val X3:  i32 = 3
```

## val X4

```mach
pub val X4:  i32 = 4
```

## val X5

```mach
pub val X5:  i32 = 5
```

## val X6

```mach
pub val X6:  i32 = 6
```

## val X7

```mach
pub val X7:  i32 = 7
```

## val X8

```mach
pub val X8:  i32 = 8
```

## val X28

```mach
pub val X28: i32 = 28
```

## val X29

```mach
pub val X29: i32 = 29
```

## val X31

```mach
pub val X31: i32 = 31
```

## val ZERO

```mach
pub val ZERO: i32 = X0
```

## val RA

```mach
pub val RA:   i32 = X1
```

## val SP

```mach
pub val SP:   i32 = X2
```

## val GP

```mach
pub val GP:   i32 = X3
```

## val TP

```mach
pub val TP:   i32 = X4
```

## val FP

```mach
pub val FP:   i32 = X8
```

## val T0

```mach
pub val T0: i32 = X5
```

## val T1

```mach
pub val T1: i32 = X6
```

## val T2

```mach
pub val T2: i32 = X7
```

## val T3

```mach
pub val T3: i32 = X28
```

## val T4

```mach
pub val T4: i32 = X29
```

## val SCRATCH_REG

```mach
pub val SCRATCH_REG:  i32 = T0
```

## val SCRATCH_REG2

```mach
pub val SCRATCH_REG2: i32 = T1
```

## val F0

```mach
pub val F0:  i32 = 0
```

## val F30

```mach
pub val F30: i32 = 30
```

## val F31

```mach
pub val F31: i32 = 31
```

## val GPR_COUNT

```mach
pub val GPR_COUNT: i32 = 32
```

## val FPR_COUNT

```mach
pub val FPR_COUNT: i32 = 30
```

## def Opcode

```mach
pub def Opcode: u16
```

## val NOP

```mach
pub val NOP:    Opcode = 0
```

## val RET

```mach
pub val RET:    Opcode = 1
```

## val ADD

```mach
pub val ADD:    Opcode = 2
```

## val SUB

```mach
pub val SUB:    Opcode = 3
```

## val MUL

```mach
pub val MUL:    Opcode = 4
```

## val AND

```mach
pub val AND:    Opcode = 5
```

## val OR

```mach
pub val OR:     Opcode = 6
```

## val XOR

```mach
pub val XOR:    Opcode = 7
```

## val SLL

```mach
pub val SLL:    Opcode = 8
```

## val SRL

```mach
pub val SRL:    Opcode = 9
```

## val SRA

```mach
pub val SRA:    Opcode = 10
```

## val NEG

```mach
pub val NEG:    Opcode = 11
```

## val NOT

```mach
pub val NOT:    Opcode = 12
```

## val MOV

```mach
pub val MOV:    Opcode = 13
```

## val JMP

```mach
pub val JMP:    Opcode = 14
```

## val EBREAK

```mach
pub val EBREAK: Opcode = 15
```

## val FMV

```mach
pub val FMV:    Opcode = 16
```

## val MULHU

```mach
pub val MULHU:  Opcode = 17
```

## val MULH

```mach
pub val MULH:   Opcode = 18
```

