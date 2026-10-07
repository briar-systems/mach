# mach.lang.target.isa.riscv.emit

riscv instructions packed and appended to the encode buffer, with the relocations they carry

## val OPC_OP

```mach
pub val OPC_OP: u32 = inst.OPC_OP
```

the encoding vocabulary is the table's (inst.mach); these are the local
spellings the field-passing emit sites use until they emit by opcode

## val OPC_OP_IMM

```mach
pub val OPC_OP_IMM: u32 = inst.OPC_OP_IMM
```

## val OPC_OP_32

```mach
pub val OPC_OP_32: u32 = inst.OPC_OP_32
```

## val OPC_OP_IMM_32

```mach
pub val OPC_OP_IMM_32: u32 = inst.OPC_OP_IMM_32
```

## val OPC_LOAD

```mach
pub val OPC_LOAD: u32 = inst.OPC_LOAD
```

## val OPC_STORE

```mach
pub val OPC_STORE: u32 = inst.OPC_STORE
```

## val OPC_BRANCH

```mach
pub val OPC_BRANCH: u32 = inst.OPC_BRANCH
```

## val OPC_JAL

```mach
pub val OPC_JAL: u32 = inst.OPC_JAL
```

## val OPC_JALR

```mach
pub val OPC_JALR: u32 = inst.OPC_JALR
```

## val OPC_LUI

```mach
pub val OPC_LUI: u32 = inst.OPC_LUI
```

## val OPC_AUIPC

```mach
pub val OPC_AUIPC: u32 = inst.OPC_AUIPC
```

## val OPC_SYSTEM

```mach
pub val OPC_SYSTEM: u32 = inst.OPC_SYSTEM
```

## val OPC_OP_FP

```mach
pub val OPC_OP_FP: u32 = inst.OPC_OP_FP
```

## val OPC_LOAD_FP

```mach
pub val OPC_LOAD_FP: u32 = inst.OPC_LOAD_FP
```

## val OPC_STORE_FP

```mach
pub val OPC_STORE_FP: u32 = inst.OPC_STORE_FP
```

## val F3_ADD

```mach
pub val F3_ADD: u32 = inst.F3_ADD
```

## val F3_SLL

```mach
pub val F3_SLL: u32 = inst.F3_SLL
```

## val F3_SLT

```mach
pub val F3_SLT: u32 = inst.F3_SLT
```

## val F3_SLTU

```mach
pub val F3_SLTU: u32 = inst.F3_SLTU
```

## val F3_XOR

```mach
pub val F3_XOR: u32 = inst.F3_XOR
```

## val F3_SRL

```mach
pub val F3_SRL: u32 = inst.F3_SRL
```

## val F3_OR

```mach
pub val F3_OR: u32 = inst.F3_OR
```

## val F3_AND

```mach
pub val F3_AND: u32 = inst.F3_AND
```

## val F7_ZERO

```mach
pub val F7_ZERO: u32 = inst.F7_ZERO
```

## val F7_SUB

```mach
pub val F7_SUB: u32 = inst.F7_SUB
```

## val F7_MULDIV

```mach
pub val F7_MULDIV: u32 = inst.F7_MULDIV
```

## val F3_BEQ

```mach
pub val F3_BEQ: u32 = inst.F3_BEQ
```

## val F3_BNE

```mach
pub val F3_BNE: u32 = inst.F3_BNE
```

## val F3_BLT

```mach
pub val F3_BLT: u32 = inst.F3_BLT
```

## val F3_BGE

```mach
pub val F3_BGE: u32 = inst.F3_BGE
```

## val F3_BLTU

```mach
pub val F3_BLTU: u32 = inst.F3_BLTU
```

## val F3_BGEU

```mach
pub val F3_BGEU: u32 = inst.F3_BGEU
```

## val F3_DIV

```mach
pub val F3_DIV: u32 = inst.F3_DIV
```

## val F3_DIVU

```mach
pub val F3_DIVU: u32 = inst.F3_DIVU
```

## val F3_REM

```mach
pub val F3_REM: u32 = inst.F3_REM
```

## val F3_REMU

```mach
pub val F3_REMU: u32 = inst.F3_REMU
```

## val F7_FADD

```mach
pub val F7_FADD: u32 = inst.F7_FADD
```

## val F7_FSUB

```mach
pub val F7_FSUB: u32 = inst.F7_FSUB
```

## val F7_FMUL

```mach
pub val F7_FMUL: u32 = inst.F7_FMUL
```

## val F7_FDIV

```mach
pub val F7_FDIV: u32 = inst.F7_FDIV
```

## val F7_FSGNJ

```mach
pub val F7_FSGNJ: u32 = inst.F7_FSGNJ
```

## val F7_FCMP

```mach
pub val F7_FCMP: u32 = inst.F7_FCMP
```

## val F7_FCVT_F2F

```mach
pub val F7_FCVT_F2F: u32 = inst.F7_FCVT_F2F
```

## val F7_FCVT_F2I

```mach
pub val F7_FCVT_F2I: u32 = inst.F7_FCVT_F2I
```

## val F7_FCVT_I2F

```mach
pub val F7_FCVT_I2F: u32 = inst.F7_FCVT_I2F
```

## val F7_FMV_F2I

```mach
pub val F7_FMV_F2I: u32 = inst.F7_FMV_F2I
```

## val F7_FMV_I2F

```mach
pub val F7_FMV_I2F: u32 = inst.F7_FMV_I2F
```

## val F3_FLE

```mach
pub val F3_FLE: u32 = inst.F3_FLE
```

## val F3_FLT

```mach
pub val F3_FLT: u32 = inst.F3_FLT
```

## val F3_FEQ

```mach
pub val F3_FEQ: u32 = inst.F3_FEQ
```

## val RM_RNE

```mach
pub val RM_RNE: u32 = inst.RM_RNE
```

## val RM_RTZ

```mach
pub val RM_RTZ: u32 = inst.RM_RTZ
```

## val RM_DYN

```mach
pub val RM_DYN: u32 = inst.RM_DYN
```

## val FCVT_W

```mach
pub val FCVT_W: u32 = inst.FCVT_W
```

## val FCVT_WU

```mach
pub val FCVT_WU: u32 = inst.FCVT_WU
```

## val FCVT_L

```mach
pub val FCVT_L: u32 = inst.FCVT_L
```

## val FCVT_LU

```mach
pub val FCVT_LU: u32 = inst.FCVT_LU
```

## val FMT_S

```mach
pub val FMT_S: u32 = inst.FMT_S_BIT
```

## val FMT_D

```mach
pub val FMT_D: u32 = inst.FMT_D_BIT
```

## val FP_SCRATCH

```mach
pub val FP_SCRATCH: u32 = isa_riscv.F31::u32
```

## val FP_SCRATCH2

```mach
pub val FP_SCRATCH2: u32 = isa_riscv.F30::u32
```

## val RZERO

```mach
pub val RZERO: u32 = isa_riscv.ZERO::u32
```

## val RRA

```mach
pub val RRA: u32 = isa_riscv.RA::u32
```

## val RSP

```mach
pub val RSP: u32 = isa_riscv.SP::u32
```

## val RFP

```mach
pub val RFP: u32 = isa_riscv.FP::u32
```

## val SCRATCH

```mach
pub val SCRATCH: u32 = isa_riscv.SCRATCH_REG::u32
```

## val SCRATCH2

```mach
pub val SCRATCH2: u32 = isa_riscv.SCRATCH_REG2::u32
```

## val CALLER_SAVED_GP

```mach
pub val CALLER_SAVED_GP: u32 = 0xF003FCE2
```

## val RA_READ

```mach
pub val RA_READ: u32 = 0x2
```

## fun b_imm

```mach
pub fun b_imm(imm: u32) u32;
```

## fun pack_r

```mach
pub fun pack_r(opcode: u32, funct3: u32, funct7: u32, rd: u32, rs1: u32, rs2: u32) u32;
```

## fun pack_i

```mach
pub fun pack_i(opcode: u32, funct3: u32, rd: u32, rs1: u32, imm12: u32) u32;
```

## fun pack_s

```mach
pub fun pack_s(opcode: u32, funct3: u32, rs1: u32, rs2: u32, imm12: u32) u32;
```

## fun pack_b

```mach
pub fun pack_b(opcode: u32, funct3: u32, rs1: u32, rs2: u32, imm: u32) u32;
```

## fun pack_u

```mach
pub fun pack_u(opcode: u32, rd: u32, imm20: u32) u32;
```

## fun pack_j

```mach
pub fun pack_j(opcode: u32, rd: u32, imm: u32) u32;
```

## fun raw_word

```mach
pub fun raw_word(st: *isa_encode.EncodeState, word: u32);
```

## val WORD_SIZE

```mach
pub val WORD_SIZE: u32 = 4
```

## fun gpr

```mach
pub fun gpr(idx: u32) isa_inst.Operand;
```

## fun build_inst

```mach
pub fun build_inst(op: inst.MachOp, rd: u32, rs1: u32, rs2: u32, imm: i64) isa_inst.Inst;
```

## fun r_type

```mach
pub fun r_type(st: *isa_encode.EncodeState, opcode: u32, funct3: u32, funct7: u32, rd: u32, rs1: u32, rs2: u32);
```

## fun i_type

```mach
pub fun i_type(st: *isa_encode.EncodeState, opcode: u32, funct3: u32, rd: u32, rs1: u32, imm12: u32);
```

## fun s_type

```mach
pub fun s_type(st: *isa_encode.EncodeState, opcode: u32, funct3: u32, rs1: u32, rs2: u32, imm12: u32);
```

## fun b_type

```mach
pub fun b_type(st: *isa_encode.EncodeState, opcode: u32, funct3: u32, rs1: u32, rs2: u32, imm: u32);
```

a displacement given here is a skip inside the expansion, and the note says
so; a block, local or symbol target is patched later and its caller notes it

## fun u_type

```mach
pub fun u_type(st: *isa_encode.EncodeState, opcode: u32, rd: u32, imm20: u32);
```

## fun j_type

```mach
pub fun j_type(st: *isa_encode.EncodeState, opcode: u32, rd: u32, imm: u32);
```

## fun note_block_target

```mach
pub fun note_block_target(st: *isa_encode.EncodeState, block: u32);
```

## fun note_local_target

```mach
pub fun note_local_target(st: *isa_encode.EncodeState, number: u32, fwd: bool);
```

## fun note_sym_target

```mach
pub fun note_sym_target(st: *isa_encode.EncodeState, sym: intern.StrId, mod: isa_inst.SymModifier, addend: i64);
```

## fun xlen

```mach
pub fun xlen(st: *isa_encode.EncodeState) u8;
```

## fun flen

```mach
pub fun flen(st: *isa_encode.EncodeState) u8;
```

## fun alu_opcode

```mach
pub fun alu_opcode(width: u8, xl: u8) u32;
```

## fun alu_imm_opcode

```mach
pub fun alu_imm_opcode(width: u8, xl: u8) u32;
```

## fun int_mem_funct3

```mach
pub fun int_mem_funct3(width: u8, is_load: bool) res[u32, fail.Fail];
```

the funct3 of a memory access row; the machine that admits the row is the
admission seam's question after the word is emitted

## fun sext12

```mach
pub fun sext12(v: i64) i64;
```

## fun li

```mach
pub fun li(st: *isa_encode.EncodeState, rd: u32, value: i64);
```

## fun pcrel_hi

```mach
pub fun pcrel_hi(st: *isa_encode.EncodeState, rd: u32, sym: intern.StrId, addend: i64) err[fail.Fail];
```

## fun push_pcrel_lo

```mach
pub fun push_pcrel_lo(st: *isa_encode.EncodeState, lo_pos: u32, kind: target_of.RelocKind,
sym: intern.StrId) err[fail.Fail];
```

## fun push_pcrel_lo_of

```mach
pub fun push_pcrel_lo_of(st: *isa_encode.EncodeState, lo_pos: u32, kind: target_of.RelocKind,
high_kind: target_of.RelocKind, sym: intern.StrId) err[fail.Fail];
```

a low part paired with the high part of `high_kind` that last named sym

## fun got_hi

```mach
pub fun got_hi(st: *isa_encode.EncodeState, rd: u32, sym: intern.StrId) err[fail.Fail];
```

the high part of the address of sym's GOT slot

## val B_DISP_MIN

```mach
pub val B_DISP_MIN: i64 = -4096
```

## val B_DISP_MAX

```mach
pub val B_DISP_MAX: i64 = 4094
```

## val J_DISP_MIN

```mach
pub val J_DISP_MIN: i64 = -1048576
```

## val J_DISP_MAX

```mach
pub val J_DISP_MAX: i64 = 1048574
```

## fun missing_extension

```mach
pub fun missing_extension(bits: u64, op: inst.MachOp) opt[str];
```

the extension a word needs and the selected machine lacks, by name; absent
when every extension the word needs is selected

## fun admission

```mach
pub fun admission(bits: u64, xl: u8, op: inst.MachOp) err[fail.Fail];
```

the one admission point: the table's xlen mask and extension bits against
the selected machine

## fun patch_branch

```mach
pub fun patch_branch(st: *isa_encode.EncodeState, fx: *isa_encode.BranchFixup, target_off: u32) err[fail.Fail];
```

