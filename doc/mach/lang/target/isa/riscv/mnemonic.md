# mach.lang.target.isa.riscv.mnemonic

the inline assembly mnemonic rows: each spelling, its operand pattern and the machines that take it

## val PATTERN_ECALL

```mach
pub val PATTERN_ECALL: u16 = 1
```

## val PATTERN_EBREAK

```mach
pub val PATTERN_EBREAK: u16 = 2
```

## val PATTERN_NOP

```mach
pub val PATTERN_NOP: u16 = 3
```

## val PATTERN_RET

```mach
pub val PATTERN_RET: u16 = 4
```

## val PATTERN_FENCE

```mach
pub val PATTERN_FENCE: u16 = 5
```

## val PATTERN_PAUSE

```mach
pub val PATTERN_PAUSE: u16 = 6
```

## val PATTERN_RRR

```mach
pub val PATTERN_RRR: u16 = 7
```

## val PATTERN_RRI

```mach
pub val PATTERN_RRI: u16 = 8
```

## val PATTERN_SHIFT

```mach
pub val PATTERN_SHIFT: u16 = 9
```

## val PATTERN_LOAD

```mach
pub val PATTERN_LOAD: u16 = 10
```

## val PATTERN_STORE

```mach
pub val PATTERN_STORE: u16 = 11
```

## val PATTERN_UIMM

```mach
pub val PATTERN_UIMM: u16 = 12
```

## val PATTERN_BRANCH

```mach
pub val PATTERN_BRANCH: u16 = 13
```

## val PATTERN_BRANCH_SW

```mach
pub val PATTERN_BRANCH_SW: u16 = 14
```

## val PATTERN_BRANCH_Z1

```mach
pub val PATTERN_BRANCH_Z1: u16 = 15
```

## val PATTERN_BRANCH_Z2

```mach
pub val PATTERN_BRANCH_Z2: u16 = 16
```

## val PATTERN_J

```mach
pub val PATTERN_J: u16 = 17
```

## val PATTERN_JAL

```mach
pub val PATTERN_JAL: u16 = 18
```

## val PATTERN_JR

```mach
pub val PATTERN_JR: u16 = 19
```

## val PATTERN_JALR

```mach
pub val PATTERN_JALR: u16 = 20
```

## val PATTERN_MV

```mach
pub val PATTERN_MV: u16 = 21
```

## val PATTERN_NOT

```mach
pub val PATTERN_NOT: u16 = 22
```

## val PATTERN_SEQZ

```mach
pub val PATTERN_SEQZ: u16 = 23
```

## val PATTERN_NEG

```mach
pub val PATTERN_NEG: u16 = 24
```

## val PATTERN_SNEZ

```mach
pub val PATTERN_SNEZ: u16 = 25
```

## val PATTERN_LI

```mach
pub val PATTERN_LI: u16 = 26
```

## val PATTERN_LA

```mach
pub val PATTERN_LA: u16 = 27
```

## val PATTERN_CALL

```mach
pub val PATTERN_CALL: u16 = 28
```

## val PATTERN_TAIL

```mach
pub val PATTERN_TAIL: u16 = 29
```

## val PATTERN_AMO

```mach
pub val PATTERN_AMO: u16 = 30
```

## val PATTERN_AMO_LR

```mach
pub val PATTERN_AMO_LR: u16 = 31
```

## val PATTERN_CSR

```mach
pub val PATTERN_CSR: u16 = 32
```

## val PATTERN_CSR_I

```mach
pub val PATTERN_CSR_I: u16 = 33
```

## val PATTERN_CSRR

```mach
pub val PATTERN_CSRR: u16 = 34
```

## val PATTERN_CSRW

```mach
pub val PATTERN_CSRW: u16 = 35
```

## val PATTERN_CSRWI

```mach
pub val PATTERN_CSRWI: u16 = 36
```

## val PATTERN_RDCYCLE

```mach
pub val PATTERN_RDCYCLE: u16 = 37
```

## val PATTERN_RDTIME

```mach
pub val PATTERN_RDTIME: u16 = 38
```

## val PATTERN_RDINSTRET

```mach
pub val PATTERN_RDINSTRET: u16 = 39
```

## val FLAG_AQ

```mach
pub val FLAG_AQ: u16 = 0x0100
```

## val FLAG_RL

```mach
pub val FLAG_RL: u16 = 0x0200
```

## fun pattern

```mach
pub fun pattern(flags: u16) u16;
```

## val ROW_COUNT

```mach
pub val ROW_COUNT: usize = 107
```

## val ROWS

```mach
pub val ROWS: [ROW_COUNT]isa_asm.Mnemonic = [ROW_COUNT]isa_asm.Mnemonic;
```

