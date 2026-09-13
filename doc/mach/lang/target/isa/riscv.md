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

## val X12

```mach
pub val X12: i32 = 12
```

## val X13

```mach
pub val X13: i32 = 13
```

## val X14

```mach
pub val X14: i32 = 14
```

## val X15

```mach
pub val X15: i32 = 15
```

## val X16

```mach
pub val X16: i32 = 16
```

## val X17

```mach
pub val X17: i32 = 17
```

## val X18

```mach
pub val X18: i32 = 18
```

## val X19

```mach
pub val X19: i32 = 19
```

## val X20

```mach
pub val X20: i32 = 20
```

## val X21

```mach
pub val X21: i32 = 21
```

## val X22

```mach
pub val X22: i32 = 22
```

## val X23

```mach
pub val X23: i32 = 23
```

## val X24

```mach
pub val X24: i32 = 24
```

## val X25

```mach
pub val X25: i32 = 25
```

## val X26

```mach
pub val X26: i32 = 26
```

## val X27

```mach
pub val X27: i32 = 27
```

## val X28

```mach
pub val X28: i32 = 28
```

## val X29

```mach
pub val X29: i32 = 29
```

## val X30

```mach
pub val X30: i32 = 30
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

## val STACK_REG

```mach
pub val STACK_REG: i32 = SP
```

## val FRAME_REG

```mach
pub val FRAME_REG: i32 = FP
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

## val T5

```mach
pub val T5: i32 = X30
```

## val T6

```mach
pub val T6: i32 = X31
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

## val F1

```mach
pub val F1:  i32 = 1
```

## val F2

```mach
pub val F2:  i32 = 2
```

## val F3

```mach
pub val F3:  i32 = 3
```

## val F4

```mach
pub val F4:  i32 = 4
```

## val F5

```mach
pub val F5:  i32 = 5
```

## val F6

```mach
pub val F6:  i32 = 6
```

## val F7

```mach
pub val F7:  i32 = 7
```

## val F8

```mach
pub val F8:  i32 = 8
```

## val F9

```mach
pub val F9:  i32 = 9
```

## val F10

```mach
pub val F10: i32 = 10
```

## val F11

```mach
pub val F11: i32 = 11
```

## val F12

```mach
pub val F12: i32 = 12
```

## val F13

```mach
pub val F13: i32 = 13
```

## val F14

```mach
pub val F14: i32 = 14
```

## val F15

```mach
pub val F15: i32 = 15
```

## val F16

```mach
pub val F16: i32 = 16
```

## val F17

```mach
pub val F17: i32 = 17
```

## val F18

```mach
pub val F18: i32 = 18
```

## val F19

```mach
pub val F19: i32 = 19
```

## val F20

```mach
pub val F20: i32 = 20
```

## val F21

```mach
pub val F21: i32 = 21
```

## val F22

```mach
pub val F22: i32 = 22
```

## val F23

```mach
pub val F23: i32 = 23
```

## val F24

```mach
pub val F24: i32 = 24
```

## val F25

```mach
pub val F25: i32 = 25
```

## val F26

```mach
pub val F26: i32 = 26
```

## val F27

```mach
pub val F27: i32 = 27
```

## val F28

```mach
pub val F28: i32 = 28
```

## val F29

```mach
pub val F29: i32 = 29
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

