# mach.lang.target.isa.riscv.inst

## def MachOp

```mach
pub def MachOp: u16
```

## val MOP_NONE

```mach
pub val MOP_NONE: MachOp = 0
```

## val ADD

```mach
pub val ADD:  MachOp = 1
```

## val SUB

```mach
pub val SUB:  MachOp = 2
```

## val SLL

```mach
pub val SLL:  MachOp = 3
```

## val SLT

```mach
pub val SLT:  MachOp = 4
```

## val SLTU

```mach
pub val SLTU: MachOp = 5
```

## val XOR

```mach
pub val XOR:  MachOp = 6
```

## val SRL

```mach
pub val SRL:  MachOp = 7
```

## val SRA

```mach
pub val SRA:  MachOp = 8
```

## val OR

```mach
pub val OR:   MachOp = 9
```

## val AND

```mach
pub val AND:  MachOp = 10
```

## val ADDW

```mach
pub val ADDW: MachOp = 11
```

## val SUBW

```mach
pub val SUBW: MachOp = 12
```

## val SLLW

```mach
pub val SLLW: MachOp = 13
```

## val SRLW

```mach
pub val SRLW: MachOp = 14
```

## val SRAW

```mach
pub val SRAW: MachOp = 15
```

## val MUL

```mach
pub val MUL:    MachOp = 16
```

## val MULH

```mach
pub val MULH:   MachOp = 17
```

## val MULHSU

```mach
pub val MULHSU: MachOp = 18
```

## val MULHU

```mach
pub val MULHU:  MachOp = 19
```

## val DIV

```mach
pub val DIV:    MachOp = 20
```

## val DIVU

```mach
pub val DIVU:   MachOp = 21
```

## val REM

```mach
pub val REM:    MachOp = 22
```

## val REMU

```mach
pub val REMU:   MachOp = 23
```

## val MULW

```mach
pub val MULW:  MachOp = 24
```

## val DIVW

```mach
pub val DIVW:  MachOp = 25
```

## val DIVUW

```mach
pub val DIVUW: MachOp = 26
```

## val REMW

```mach
pub val REMW:  MachOp = 27
```

## val REMUW

```mach
pub val REMUW: MachOp = 28
```

## val ADDI

```mach
pub val ADDI:  MachOp = 29
```

## val SLTI

```mach
pub val SLTI:  MachOp = 30
```

## val SLTIU

```mach
pub val SLTIU: MachOp = 31
```

## val XORI

```mach
pub val XORI:  MachOp = 32
```

## val ORI

```mach
pub val ORI:   MachOp = 33
```

## val ANDI

```mach
pub val ANDI:  MachOp = 34
```

## val SLLI

```mach
pub val SLLI:  MachOp = 35
```

## val SRLI

```mach
pub val SRLI:  MachOp = 36
```

## val SRAI

```mach
pub val SRAI:  MachOp = 37
```

## val ADDIW

```mach
pub val ADDIW: MachOp = 38
```

## val SLLIW

```mach
pub val SLLIW: MachOp = 39
```

## val SRLIW

```mach
pub val SRLIW: MachOp = 40
```

## val SRAIW

```mach
pub val SRAIW: MachOp = 41
```

## val LB

```mach
pub val LB:  MachOp = 42
```

## val LH

```mach
pub val LH:  MachOp = 43
```

## val LW

```mach
pub val LW:  MachOp = 44
```

## val LD

```mach
pub val LD:  MachOp = 45
```

## val LBU

```mach
pub val LBU: MachOp = 46
```

## val LHU

```mach
pub val LHU: MachOp = 47
```

## val LWU

```mach
pub val LWU: MachOp = 48
```

## val SB

```mach
pub val SB: MachOp = 49
```

## val SH

```mach
pub val SH: MachOp = 50
```

## val SW

```mach
pub val SW: MachOp = 51
```

## val SD

```mach
pub val SD: MachOp = 52
```

## val BEQ

```mach
pub val BEQ:  MachOp = 53
```

## val BNE

```mach
pub val BNE:  MachOp = 54
```

## val BLT

```mach
pub val BLT:  MachOp = 55
```

## val BGE

```mach
pub val BGE:  MachOp = 56
```

## val BLTU

```mach
pub val BLTU: MachOp = 57
```

## val BGEU

```mach
pub val BGEU: MachOp = 58
```

## val JAL

```mach
pub val JAL:  MachOp = 59
```

## val JALR

```mach
pub val JALR: MachOp = 60
```

## val LUI

```mach
pub val LUI:   MachOp = 61
```

## val AUIPC

```mach
pub val AUIPC: MachOp = 62
```

## val ECALL

```mach
pub val ECALL:  MachOp = 63
```

## val EBREAK

```mach
pub val EBREAK: MachOp = 64
```

## val FENCE

```mach
pub val FENCE:  MachOp = 65
```

## val PAUSE

```mach
pub val PAUSE:  MachOp = 66
```

## val FLW

```mach
pub val FLW: MachOp = 67
```

## val FLD

```mach
pub val FLD: MachOp = 68
```

## val FSW

```mach
pub val FSW: MachOp = 69
```

## val FSD

```mach
pub val FSD: MachOp = 70
```

## val FADD_S

```mach
pub val FADD_S: MachOp = 71
```

## val FADD_D

```mach
pub val FADD_D: MachOp = 72
```

## val FSUB_S

```mach
pub val FSUB_S: MachOp = 73
```

## val FSUB_D

```mach
pub val FSUB_D: MachOp = 74
```

## val FMUL_S

```mach
pub val FMUL_S: MachOp = 75
```

## val FMUL_D

```mach
pub val FMUL_D: MachOp = 76
```

## val FDIV_S

```mach
pub val FDIV_S: MachOp = 77
```

## val FDIV_D

```mach
pub val FDIV_D: MachOp = 78
```

## val FSGNJ_S

```mach
pub val FSGNJ_S:  MachOp = 79
```

## val FSGNJ_D

```mach
pub val FSGNJ_D:  MachOp = 80
```

## val FSGNJN_S

```mach
pub val FSGNJN_S: MachOp = 81
```

## val FSGNJN_D

```mach
pub val FSGNJN_D: MachOp = 82
```

## val FSGNJX_S

```mach
pub val FSGNJX_S: MachOp = 83
```

## val FSGNJX_D

```mach
pub val FSGNJX_D: MachOp = 84
```

## val FEQ_S

```mach
pub val FEQ_S: MachOp = 85
```

## val FEQ_D

```mach
pub val FEQ_D: MachOp = 86
```

## val FLT_S

```mach
pub val FLT_S: MachOp = 87
```

## val FLT_D

```mach
pub val FLT_D: MachOp = 88
```

## val FLE_S

```mach
pub val FLE_S: MachOp = 89
```

## val FLE_D

```mach
pub val FLE_D: MachOp = 90
```

## val FCVT_S_D

```mach
pub val FCVT_S_D: MachOp = 91
```

## val FCVT_D_S

```mach
pub val FCVT_D_S: MachOp = 92
```

## val FCVT_W_S

```mach
pub val FCVT_W_S:  MachOp = 93
```

## val FCVT_WU_S

```mach
pub val FCVT_WU_S: MachOp = 94
```

## val FCVT_L_S

```mach
pub val FCVT_L_S:  MachOp = 95
```

## val FCVT_LU_S

```mach
pub val FCVT_LU_S: MachOp = 96
```

## val FCVT_W_D

```mach
pub val FCVT_W_D:  MachOp = 97
```

## val FCVT_WU_D

```mach
pub val FCVT_WU_D: MachOp = 98
```

## val FCVT_L_D

```mach
pub val FCVT_L_D:  MachOp = 99
```

## val FCVT_LU_D

```mach
pub val FCVT_LU_D: MachOp = 100
```

## val FCVT_S_W

```mach
pub val FCVT_S_W:  MachOp = 101
```

## val FCVT_S_WU

```mach
pub val FCVT_S_WU: MachOp = 102
```

## val FCVT_S_L

```mach
pub val FCVT_S_L:  MachOp = 103
```

## val FCVT_S_LU

```mach
pub val FCVT_S_LU: MachOp = 104
```

## val FCVT_D_W

```mach
pub val FCVT_D_W:  MachOp = 105
```

## val FCVT_D_WU

```mach
pub val FCVT_D_WU: MachOp = 106
```

## val FCVT_D_L

```mach
pub val FCVT_D_L:  MachOp = 107
```

## val FCVT_D_LU

```mach
pub val FCVT_D_LU: MachOp = 108
```

## val FMV_X_W

```mach
pub val FMV_X_W: MachOp = 109
```

## val FMV_X_D

```mach
pub val FMV_X_D: MachOp = 110
```

## val FMV_W_X

```mach
pub val FMV_W_X: MachOp = 111
```

## val FMV_D_X

```mach
pub val FMV_D_X: MachOp = 112
```

## val LR_W

```mach
pub val LR_W: MachOp = 113
```

## val LR_D

```mach
pub val LR_D: MachOp = 114
```

## val SC_W

```mach
pub val SC_W: MachOp = 115
```

## val SC_D

```mach
pub val SC_D: MachOp = 116
```

## val AMOSWAP_W

```mach
pub val AMOSWAP_W: MachOp = 117
```

## val AMOSWAP_D

```mach
pub val AMOSWAP_D: MachOp = 118
```

## val AMOADD_W

```mach
pub val AMOADD_W:  MachOp = 119
```

## val AMOADD_D

```mach
pub val AMOADD_D:  MachOp = 120
```

## val AMOXOR_W

```mach
pub val AMOXOR_W:  MachOp = 121
```

## val AMOXOR_D

```mach
pub val AMOXOR_D:  MachOp = 122
```

## val AMOAND_W

```mach
pub val AMOAND_W:  MachOp = 123
```

## val AMOAND_D

```mach
pub val AMOAND_D:  MachOp = 124
```

## val AMOOR_W

```mach
pub val AMOOR_W:   MachOp = 125
```

## val AMOOR_D

```mach
pub val AMOOR_D:   MachOp = 126
```

## val AMOMIN_W

```mach
pub val AMOMIN_W:  MachOp = 127
```

## val AMOMIN_D

```mach
pub val AMOMIN_D:  MachOp = 128
```

## val AMOMAX_W

```mach
pub val AMOMAX_W:  MachOp = 129
```

## val AMOMAX_D

```mach
pub val AMOMAX_D:  MachOp = 130
```

## val AMOMINU_W

```mach
pub val AMOMINU_W: MachOp = 131
```

## val AMOMINU_D

```mach
pub val AMOMINU_D: MachOp = 132
```

## val AMOMAXU_W

```mach
pub val AMOMAXU_W: MachOp = 133
```

## val AMOMAXU_D

```mach
pub val AMOMAXU_D: MachOp = 134
```

## val CSRRW

```mach
pub val CSRRW:  MachOp = 135
```

## val CSRRS

```mach
pub val CSRRS:  MachOp = 136
```

## val CSRRC

```mach
pub val CSRRC:  MachOp = 137
```

## val CSRRWI

```mach
pub val CSRRWI: MachOp = 138
```

## val CSRRSI

```mach
pub val CSRRSI: MachOp = 139
```

## val CSRRCI

```mach
pub val CSRRCI: MachOp = 140
```

## val MOP_LAST

```mach
pub val MOP_LAST: MachOp = CSRRCI
```

## val ROW_COUNT

```mach
pub val ROW_COUNT: usize = 141
```

ROW_COUNT is MOP_LAST + 1 spelled as a literal so the array length is
comptime; the catalog test holds the two together

## val XL32

```mach
pub val XL32:   u8 = 0x01
```

## val XL64

```mach
pub val XL64:   u8 = 0x02
```

## val XL_ANY

```mach
pub val XL_ANY: u8 = 0x03
```

## fun xlen_bit

```mach
pub fun xlen_bit(xlen_bytes: u8) u8;
```

## val OPC_LOAD

```mach
pub val OPC_LOAD:      u32 = 0x03
```

base opcodes, bits 6:0 of every word

## val OPC_LOAD_FP

```mach
pub val OPC_LOAD_FP:   u32 = 0x07
```

## val OPC_MISC_MEM

```mach
pub val OPC_MISC_MEM:  u32 = 0x0F
```

## val OPC_OP_IMM

```mach
pub val OPC_OP_IMM:    u32 = 0x13
```

## val OPC_AUIPC

```mach
pub val OPC_AUIPC:     u32 = 0x17
```

## val OPC_OP_IMM_32

```mach
pub val OPC_OP_IMM_32: u32 = 0x1B
```

## val OPC_STORE

```mach
pub val OPC_STORE:     u32 = 0x23
```

## val OPC_STORE_FP

```mach
pub val OPC_STORE_FP:  u32 = 0x27
```

## val OPC_AMO

```mach
pub val OPC_AMO:       u32 = 0x2F
```

## val OPC_OP

```mach
pub val OPC_OP:        u32 = 0x33
```

## val OPC_LUI

```mach
pub val OPC_LUI:       u32 = 0x37
```

## val OPC_OP_32

```mach
pub val OPC_OP_32:     u32 = 0x3B
```

## val OPC_OP_FP

```mach
pub val OPC_OP_FP:     u32 = 0x53
```

## val OPC_BRANCH

```mach
pub val OPC_BRANCH:    u32 = 0x63
```

## val OPC_JALR

```mach
pub val OPC_JALR:      u32 = 0x67
```

## val OPC_JAL

```mach
pub val OPC_JAL:       u32 = 0x6F
```

## val OPC_SYSTEM

```mach
pub val OPC_SYSTEM:    u32 = 0x73
```

## val F3_ADD

```mach
pub val F3_ADD:  u32 = 0x0
```

funct3 of the integer operate forms

## val F3_SLL

```mach
pub val F3_SLL:  u32 = 0x1
```

## val F3_SLT

```mach
pub val F3_SLT:  u32 = 0x2
```

## val F3_SLTU

```mach
pub val F3_SLTU: u32 = 0x3
```

## val F3_XOR

```mach
pub val F3_XOR:  u32 = 0x4
```

## val F3_SRL

```mach
pub val F3_SRL:  u32 = 0x5
```

## val F3_OR

```mach
pub val F3_OR:   u32 = 0x6
```

## val F3_AND

```mach
pub val F3_AND:  u32 = 0x7
```

## val F3_DIV

```mach
pub val F3_DIV:  u32 = 0x4
```

## val F3_DIVU

```mach
pub val F3_DIVU: u32 = 0x5
```

## val F3_REM

```mach
pub val F3_REM:  u32 = 0x6
```

## val F3_REMU

```mach
pub val F3_REMU: u32 = 0x7
```

## val F3_MEM_B

```mach
pub val F3_MEM_B:  u32 = 0x0
```

funct3 of the memory forms: the access width, unsigned loads at bit 2

## val F3_MEM_H

```mach
pub val F3_MEM_H:  u32 = 0x1
```

## val F3_MEM_W

```mach
pub val F3_MEM_W:  u32 = 0x2
```

## val F3_MEM_D

```mach
pub val F3_MEM_D:  u32 = 0x3
```

## val F3_MEM_BU

```mach
pub val F3_MEM_BU: u32 = 0x4
```

## val F3_MEM_HU

```mach
pub val F3_MEM_HU: u32 = 0x5
```

## val F3_MEM_WU

```mach
pub val F3_MEM_WU: u32 = 0x6
```

## val F3_BEQ

```mach
pub val F3_BEQ:  u32 = 0x0
```

## val F3_BNE

```mach
pub val F3_BNE:  u32 = 0x1
```

## val F3_BLT

```mach
pub val F3_BLT:  u32 = 0x4
```

## val F3_BGE

```mach
pub val F3_BGE:  u32 = 0x5
```

## val F3_BLTU

```mach
pub val F3_BLTU: u32 = 0x6
```

## val F3_BGEU

```mach
pub val F3_BGEU: u32 = 0x7
```

## val F3_CSRRW

```mach
pub val F3_CSRRW:  u32 = 0x1
```

## val F3_CSRRS

```mach
pub val F3_CSRRS:  u32 = 0x2
```

## val F3_CSRRC

```mach
pub val F3_CSRRC:  u32 = 0x3
```

## val F3_CSRRWI

```mach
pub val F3_CSRRWI: u32 = 0x5
```

## val F3_CSRRSI

```mach
pub val F3_CSRRSI: u32 = 0x6
```

## val F3_CSRRCI

```mach
pub val F3_CSRRCI: u32 = 0x7
```

## val F3_AMO_W

```mach
pub val F3_AMO_W: u32 = 0x2
```

## val F3_AMO_D

```mach
pub val F3_AMO_D: u32 = 0x3
```

## val F3_FSGNJ

```mach
pub val F3_FSGNJ:  u32 = 0x0
```

funct3 of the sign-injection and compare forms

## val F3_FSGNJN

```mach
pub val F3_FSGNJN: u32 = 0x1
```

## val F3_FSGNJX

```mach
pub val F3_FSGNJX: u32 = 0x2
```

## val F3_FLE

```mach
pub val F3_FLE:    u32 = 0x0
```

## val F3_FLT

```mach
pub val F3_FLT:    u32 = 0x1
```

## val F3_FEQ

```mach
pub val F3_FEQ:    u32 = 0x2
```

## val RM_RNE

```mach
pub val RM_RNE: u32 = 0x0
```

funct3 of the rounding-mode forms: the mode the encoder emits

## val RM_RTZ

```mach
pub val RM_RTZ: u32 = 0x1
```

## val RM_DYN

```mach
pub val RM_DYN: u32 = 0x7
```

## val F7_ZERO

```mach
pub val F7_ZERO:   u32 = 0x00
```

## val F7_MULDIV

```mach
pub val F7_MULDIV: u32 = 0x01
```

## val F7_SUB

```mach
pub val F7_SUB:    u32 = 0x20
```

## val F7_FADD

```mach
pub val F7_FADD:     u32 = 0x00
```

funct7 of the F forms; bit 0 selects the D format

## val F7_FSUB

```mach
pub val F7_FSUB:     u32 = 0x04
```

## val F7_FMUL

```mach
pub val F7_FMUL:     u32 = 0x08
```

## val F7_FDIV

```mach
pub val F7_FDIV:     u32 = 0x0C
```

## val F7_FSGNJ

```mach
pub val F7_FSGNJ:    u32 = 0x10
```

## val F7_FCVT_F2F

```mach
pub val F7_FCVT_F2F: u32 = 0x20
```

## val F7_FCMP

```mach
pub val F7_FCMP:     u32 = 0x50
```

## val F7_FCVT_F2I

```mach
pub val F7_FCVT_F2I: u32 = 0x60
```

## val F7_FCVT_I2F

```mach
pub val F7_FCVT_I2F: u32 = 0x68
```

## val F7_FMV_F2I

```mach
pub val F7_FMV_F2I:  u32 = 0x70
```

## val F7_FMV_I2F

```mach
pub val F7_FMV_I2F:  u32 = 0x78
```

## val FMT_S_BIT

```mach
pub val FMT_S_BIT: u32 = 0x0
```

## val FMT_D_BIT

```mach
pub val FMT_D_BIT: u32 = 0x1
```

## val FCVT_W

```mach
pub val FCVT_W:  u32 = 0x0
```

the rs2 selector of a conversion: the integer width, or the source format

## val FCVT_WU

```mach
pub val FCVT_WU: u32 = 0x1
```

## val FCVT_L

```mach
pub val FCVT_L:  u32 = 0x2
```

## val FCVT_LU

```mach
pub val FCVT_LU: u32 = 0x3
```

## val A5_AMOADD

```mach
pub val A5_AMOADD:  u32 = 0x00
```

funct5 (funct7 bits 6:2) of the A forms; bits 1:0 carry aq and rl

## val A5_AMOSWAP

```mach
pub val A5_AMOSWAP: u32 = 0x01
```

## val A5_LR

```mach
pub val A5_LR:      u32 = 0x02
```

## val A5_SC

```mach
pub val A5_SC:      u32 = 0x03
```

## val A5_AMOXOR

```mach
pub val A5_AMOXOR:  u32 = 0x04
```

## val A5_AMOOR

```mach
pub val A5_AMOOR:   u32 = 0x08
```

## val A5_AMOAND

```mach
pub val A5_AMOAND:  u32 = 0x0C
```

## val A5_AMOMIN

```mach
pub val A5_AMOMIN:  u32 = 0x10
```

## val A5_AMOMAX

```mach
pub val A5_AMOMAX:  u32 = 0x14
```

## val A5_AMOMINU

```mach
pub val A5_AMOMINU: u32 = 0x18
```

## val A5_AMOMAXU

```mach
pub val A5_AMOMAXU: u32 = 0x1C
```

## val IMM_ECALL

```mach
pub val IMM_ECALL:      u32 = 0x000
```

the fixed imm12 of the system and fence forms

## val IMM_EBREAK

```mach
pub val IMM_EBREAK:     u32 = 0x001
```

## val IMM_FENCE_IORW

```mach
pub val IMM_FENCE_IORW: u32 = 0x0FF
```

## val IMM_PAUSE

```mach
pub val IMM_PAUSE:      u32 = 0x010
```

## val FMT_R

```mach
pub val FMT_R: u8 = 0
```

the instruction format: how the fields pack into the word

## val FMT_I

```mach
pub val FMT_I: u8 = 1
```

## val FMT_S

```mach
pub val FMT_S: u8 = 2
```

## val FMT_B

```mach
pub val FMT_B: u8 = 3
```

## val FMT_U

```mach
pub val FMT_U: u8 = 4
```

## val FMT_J

```mach
pub val FMT_J: u8 = 5
```

## val SH_NONE

```mach
pub val SH_NONE:   u8 = 0
```

the operand shape of the notification the encoder builds for an
instruction: which register banks and operand kinds sit in dst/src1/src2

## val SH_RRR

```mach
pub val SH_RRR:    u8 = 1
```

## val SH_RRI

```mach
pub val SH_RRI:    u8 = 2
```

## val SH_RU

```mach
pub val SH_RU:     u8 = 3
```

## val SH_LOAD

```mach
pub val SH_LOAD:   u8 = 4
```

## val SH_FLOAD

```mach
pub val SH_FLOAD:  u8 = 5
```

## val SH_STORE

```mach
pub val SH_STORE:  u8 = 6
```

## val SH_FSTORE

```mach
pub val SH_FSTORE: u8 = 7
```

## val SH_BRANCH

```mach
pub val SH_BRANCH: u8 = 8
```

## val SH_JUMP

```mach
pub val SH_JUMP:   u8 = 9
```

## val SH_FFF

```mach
pub val SH_FFF:    u8 = 10
```

## val SH_FF

```mach
pub val SH_FF:     u8 = 11
```

## val SH_GFF

```mach
pub val SH_GFF:    u8 = 12
```

## val SH_GF

```mach
pub val SH_GF:     u8 = 13
```

## val SH_FG

```mach
pub val SH_FG:     u8 = 14
```

## val SH_AMO

```mach
pub val SH_AMO:    u8 = 15
```

## val SH_AMO_LR

```mach
pub val SH_AMO_LR: u8 = 16
```

## val SH_CSR

```mach
pub val SH_CSR:    u8 = 17
```

## val SH_CSR_I

```mach
pub val SH_CSR_I:  u8 = 18
```

## val MEM_NONE

```mach
pub val MEM_NONE:   u8 = 0
```

the memory role of an instruction

## val MEM_LOAD

```mach
pub val MEM_LOAD:   u8 = 1
```

## val MEM_STORE

```mach
pub val MEM_STORE:  u8 = 2
```

## val MEM_ATOMIC

```mach
pub val MEM_ATOMIC: u8 = 3
```

## val KEY_F3

```mach
pub val KEY_F3: u8 = 0x01
```

which fields beyond the base opcode name the instruction when a word is
classified; a field a row does not key on is free (a rounding mode, a
shift amount, an ordering bit)

## val KEY_F7

```mach
pub val KEY_F7: u8 = 0x02
```

## val KEY_F7_HI6

```mach
pub val KEY_F7_HI6: u8 = 0x04
```

funct7 bits 6:1; bit 0 is shamt[5] on rv64 (the shift immediates)

## val KEY_F5

```mach
pub val KEY_F5: u8 = 0x08
```

funct7 bits 6:2 (the A forms)

## val KEY_RS2

```mach
pub val KEY_RS2: u8 = 0x10
```

rs2 == sel (the conversions)

## val KEY_IMM12

```mach
pub val KEY_IMM12: u8 = 0x20
```

imm12 == sel (the system and fence forms)

## rec Row

```mach
pub rec Row;
```

one row per machine opcode: the encoding fields, the classification key,
the machines and extensions that admit it, its spelling and its shape.
every reader of a per-opcode fact reads this table; an opcode without a
row does not compile, and a row out of order fails the catalog test

## val ROWS

```mach
pub val ROWS: [ROW_COUNT]Row = [ROW_COUNT]Row;
```

## fun in_catalog

```mach
pub fun in_catalog(op: MachOp) bool;
```

## fun row

```mach
pub fun row(op: MachOp) *Row;
```

## fun mnemonic

```mach
pub fun mnemonic(op: MachOp) opt[str];
```

the assembler spelling of an opcode; absent outside the catalog

## fun shape_of

```mach
pub fun shape_of(op: MachOp) u8;
```

## fun mem_role

```mach
pub fun mem_role(op: MachOp) u8;
```

## fun xlens

```mach
pub fun xlens(op: MachOp) u8;
```

## fun admits

```mach
pub fun admits(op: MachOp, xlen_bytes: u8) bool;
```

## fun required_extensions

```mach
pub fun required_extensions(op: MachOp) u32;
```

the selection bits an opcode needs; 0 only for an opcode outside the catalog

## fun int_access

```mach
pub fun int_access(width: u8, is_load: bool) MachOp;
```

the integer memory access of a width: a subword load zero-extends, the
word load sign-extends. the machine that admits the row is the admission
seam's question, not this one's

## fun fp_access

```mach
pub fun fp_access(width: u8, is_load: bool) MachOp;
```

## fun classify

```mach
pub fun classify(word: u32) MachOp;
```

the opcode an emitted word spells, MOP_NONE when no row claims it

## fun key_word

```mach
pub fun key_word(op: MachOp) u32;
```

the word an opcode's row spells with every register field zero: the
keyed fields set, every free field zero

## val FLAG_AQ

```mach
pub val FLAG_AQ: u16 = 0x0001
```

## val FLAG_RL

```mach
pub val FLAG_RL: u16 = 0x0002
```

## val FLAG_LABEL_LOCAL

```mach
pub val FLAG_LABEL_LOCAL: u16 = 0x0004
```

## val FLAG_LABEL_FWD

```mach
pub val FLAG_LABEL_FWD:   u16 = 0x0008
```

## val FLAG_LABEL_SKIP

```mach
pub val FLAG_LABEL_SKIP:  u16 = 0x0010
```

## val FLAG_TARGET_BLOCK

```mach
pub val FLAG_TARGET_BLOCK: u16 = 0x0020
```

