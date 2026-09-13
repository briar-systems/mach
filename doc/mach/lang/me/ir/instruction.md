# mach.lang.me.ir.instruction

## def InstrKind

```mach
pub def InstrKind: u8
```

## val OP_ADD

```mach
pub val OP_ADD:   InstrKind = 0
```

## val OP_SUB

```mach
pub val OP_SUB:   InstrKind = 1
```

## val OP_MUL

```mach
pub val OP_MUL:   InstrKind = 2
```

## val OP_DIV_S

```mach
pub val OP_DIV_S: InstrKind = 3
```

## val OP_DIV_U

```mach
pub val OP_DIV_U: InstrKind = 4
```

## val OP_REM_S

```mach
pub val OP_REM_S: InstrKind = 5
```

## val OP_REM_U

```mach
pub val OP_REM_U: InstrKind = 6
```

## val OP_NEG

```mach
pub val OP_NEG:   InstrKind = 7
```

## val OP_AND

```mach
pub val OP_AND:   InstrKind = 8
```

## val OP_OR

```mach
pub val OP_OR:    InstrKind = 9
```

## val OP_XOR

```mach
pub val OP_XOR:   InstrKind = 10
```

## val OP_SHL

```mach
pub val OP_SHL:   InstrKind = 11
```

## val OP_SHR_S

```mach
pub val OP_SHR_S: InstrKind = 12
```

## val OP_SHR_U

```mach
pub val OP_SHR_U: InstrKind = 13
```

## val OP_NOT

```mach
pub val OP_NOT:   InstrKind = 14
```

## val OP_CMP_EQ

```mach
pub val OP_CMP_EQ:   InstrKind = 15
```

## val OP_CMP_NE

```mach
pub val OP_CMP_NE:   InstrKind = 16
```

## val OP_CMP_LT_S

```mach
pub val OP_CMP_LT_S: InstrKind = 17
```

## val OP_CMP_LT_U

```mach
pub val OP_CMP_LT_U: InstrKind = 18
```

## val OP_CMP_LE_S

```mach
pub val OP_CMP_LE_S: InstrKind = 19
```

## val OP_CMP_LE_U

```mach
pub val OP_CMP_LE_U: InstrKind = 20
```

## val OP_TRUNC

```mach
pub val OP_TRUNC:    InstrKind = 21
```

## val OP_SEXT

```mach
pub val OP_SEXT:     InstrKind = 22
```

## val OP_ZEXT

```mach
pub val OP_ZEXT:     InstrKind = 23
```

## val OP_FP_TRUNC

```mach
pub val OP_FP_TRUNC: InstrKind = 24
```

## val OP_FP_EXT

```mach
pub val OP_FP_EXT:   InstrKind = 25
```

## val OP_FP_TO_SI

```mach
pub val OP_FP_TO_SI: InstrKind = 26
```

## val OP_FP_TO_UI

```mach
pub val OP_FP_TO_UI: InstrKind = 27
```

## val OP_SI_TO_FP

```mach
pub val OP_SI_TO_FP: InstrKind = 28
```

## val OP_UI_TO_FP

```mach
pub val OP_UI_TO_FP: InstrKind = 29
```

## val OP_BITCAST

```mach
pub val OP_BITCAST:  InstrKind = 30
```

## val OP_ALLOCA

```mach
pub val OP_ALLOCA: InstrKind = 31
```

## val OP_LOAD

```mach
pub val OP_LOAD:   InstrKind = 32
```

## val OP_STORE

```mach
pub val OP_STORE:  InstrKind = 33
```

## val OP_GEP

```mach
pub val OP_GEP:    InstrKind = 34
```

## val OP_EXTRACT

```mach
pub val OP_EXTRACT: InstrKind = 35
```

## val OP_INSERT

```mach
pub val OP_INSERT:  InstrKind = 36
```

## val OP_PHI

```mach
pub val OP_PHI:         InstrKind = 37
```

## val OP_CALL

```mach
pub val OP_CALL:        InstrKind = 38
```

## val OP_BR

```mach
pub val OP_BR:          InstrKind = 39
```

## val OP_CBR

```mach
pub val OP_CBR:         InstrKind = 40
```

## val OP_RET

```mach
pub val OP_RET:         InstrKind = 41
```

## val OP_UNREACHABLE

```mach
pub val OP_UNREACHABLE: InstrKind = 42
```

## val OP_ASM

```mach
pub val OP_ASM: InstrKind = 43
```

## val OP_MEMZERO

```mach
pub val OP_MEMZERO: InstrKind = 44
```

## val OP_DBG_VALUE

```mach
pub val OP_DBG_VALUE: InstrKind = 45
```

## val OP_DECLASSIFY

```mach
pub val OP_DECLASSIFY: InstrKind = 46
```

## val OP_VEC_EXTRACT

```mach
pub val OP_VEC_EXTRACT: InstrKind = 47
```

## val OP_VEC_INSERT

```mach
pub val OP_VEC_INSERT:  InstrKind = 48
```

## val OP_VEC_BUILD

```mach
pub val OP_VEC_BUILD:   InstrKind = 49
```

## val INSTR_FLAG_NSW

```mach
pub val INSTR_FLAG_NSW:      u16 = 0x01
```

## val INSTR_FLAG_NUW

```mach
pub val INSTR_FLAG_NUW:      u16 = 0x02
```

## val INSTR_FLAG_EXACT

```mach
pub val INSTR_FLAG_EXACT:    u16 = 0x04
```

## val INSTR_FLAG_VOLATILE

```mach
pub val INSTR_FLAG_VOLATILE: u16 = 0x08
```

## rec Instruction

```mach
pub rec Instruction;
```

## fun mark_result_secret

```mach
pub fun mark_result_secret(instrs: *Instruction, len: u32, v: value.Value);
```

