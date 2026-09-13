# mach.lang.target.isa.arm64.inst

## def MachOp

```mach
pub def MachOp: u16
```

the aarch64 machine-opcode space: one member per instruction the encoder
emits or the inline-asm grammar accepts, named by the assembler spelling the
printer renders. `arm64.Opcode` is the selection space (a MIR row alias);
this is what an instruction notification carries, the same split riscv makes
between `riscv.Opcode` and `riscv.inst.MachOp`. members that share a
spelling are distinct instructions (the NEON `mov` aliases, the three `fmov`
pages); an alias member is a spelling the notification produces for another
member's encoding and is never assembled itself

## val MOP_NONE

```mach
pub val MOP_NONE: MachOp = 0
```

## val NOP

```mach
pub val NOP:   MachOp = 1
```

## val YIELD

```mach
pub val YIELD: MachOp = 2
```

## val WFE

```mach
pub val WFE:   MachOp = 3
```

## val WFI

```mach
pub val WFI:   MachOp = 4
```

## val RET

```mach
pub val RET:   MachOp = 5
```

## val SVC

```mach
pub val SVC:   MachOp = 6
```

## val BRK

```mach
pub val BRK:   MachOp = 7
```

## val HVC

```mach
pub val HVC:   MachOp = 8
```

## val SMC

```mach
pub val SMC:   MachOp = 9
```

## val MOV

```mach
pub val MOV:  MachOp = 10
```

## val MOVZ

```mach
pub val MOVZ: MachOp = 11
```

## val MOVK

```mach
pub val MOVK: MachOp = 12
```

## val MVN

```mach
pub val MVN:  MachOp = 13
```

## val NEG

```mach
pub val NEG:  MachOp = 14
```

## val ADD

```mach
pub val ADD:  MachOp = 15
```

## val ADDS

```mach
pub val ADDS: MachOp = 16
```

## val SUB

```mach
pub val SUB:  MachOp = 17
```

## val SUBS

```mach
pub val SUBS: MachOp = 18
```

## val CMP

```mach
pub val CMP:  MachOp = 19
```

## val CMN

```mach
pub val CMN:  MachOp = 20
```

## val AND

```mach
pub val AND:  MachOp = 21
```

## val ORR

```mach
pub val ORR:  MachOp = 22
```

## val ORN

```mach
pub val ORN:  MachOp = 23
```

## val EOR

```mach
pub val EOR:  MachOp = 24
```

## val MUL

```mach
pub val MUL:  MachOp = 25
```

## val MNEG

```mach
pub val MNEG: MachOp = 26
```

## val MADD

```mach
pub val MADD: MachOp = 27
```

## val MSUB

```mach
pub val MSUB: MachOp = 28
```

## val SDIV

```mach
pub val SDIV: MachOp = 29
```

## val UDIV

```mach
pub val UDIV: MachOp = 30
```

## val LSL

```mach
pub val LSL:  MachOp = 31
```

immediate shifts are bitfield aliases; the register-count forms are the
variable-latency members and keep their own members even though the
printer renders both with the preferred `lsl`/`lsr`/`asr` spelling

## val LSR

```mach
pub val LSR:  MachOp = 32
```

## val ASR

```mach
pub val ASR:  MachOp = 33
```

## val LSLV

```mach
pub val LSLV: MachOp = 34
```

## val LSRV

```mach
pub val LSRV: MachOp = 35
```

## val ASRV

```mach
pub val ASRV: MachOp = 36
```

## val UBFX

```mach
pub val UBFX:  MachOp = 37
```

## val UBFIZ

```mach
pub val UBFIZ: MachOp = 38
```

## val SBFX

```mach
pub val SBFX:  MachOp = 39
```

## val SBFIZ

```mach
pub val SBFIZ: MachOp = 40
```

## val UXTB

```mach
pub val UXTB:  MachOp = 41
```

## val UXTH

```mach
pub val UXTH:  MachOp = 42
```

## val SXTB

```mach
pub val SXTB:  MachOp = 43
```

## val SXTH

```mach
pub val SXTH:  MachOp = 44
```

## val SXTW

```mach
pub val SXTW:  MachOp = 45
```

## val CSET

```mach
pub val CSET: MachOp = 46
```

## val ADRP

```mach
pub val ADRP: MachOp = 47
```

## val B

```mach
pub val B:     MachOp = 48
```

## val BCOND

```mach
pub val BCOND: MachOp = 49
```

## val BL

```mach
pub val BL:    MachOp = 50
```

## val BLR

```mach
pub val BLR:   MachOp = 51
```

## val BR

```mach
pub val BR:    MachOp = 52
```

## val CBZ

```mach
pub val CBZ:   MachOp = 53
```

## val CBNZ

```mach
pub val CBNZ:  MachOp = 54
```

## val LDR

```mach
pub val LDR:   MachOp = 55
```

## val STR

```mach
pub val STR:   MachOp = 56
```

## val LDRB

```mach
pub val LDRB:  MachOp = 57
```

## val STRB

```mach
pub val STRB:  MachOp = 58
```

## val LDRH

```mach
pub val LDRH:  MachOp = 59
```

## val STRH

```mach
pub val STRH:  MachOp = 60
```

## val LDUR

```mach
pub val LDUR:  MachOp = 61
```

## val STUR

```mach
pub val STUR:  MachOp = 62
```

## val LDURB

```mach
pub val LDURB: MachOp = 63
```

## val STURB

```mach
pub val STURB: MachOp = 64
```

## val LDURH

```mach
pub val LDURH: MachOp = 65
```

## val STURH

```mach
pub val STURH: MachOp = 66
```

## val LDP

```mach
pub val LDP:   MachOp = 67
```

## val STP

```mach
pub val STP:   MachOp = 68
```

## val LDAR

```mach
pub val LDAR:  MachOp = 69
```

## val STLR

```mach
pub val STLR:  MachOp = 70
```

## val LDAXR

```mach
pub val LDAXR: MachOp = 71
```

## val STLXR

```mach
pub val STLXR: MachOp = 72
```

## val DMB

```mach
pub val DMB:   MachOp = 73
```

## val MSR

```mach
pub val MSR:   MachOp = 74
```

## val MRS

```mach
pub val MRS:   MachOp = 75
```

## val FADD

```mach
pub val FADD:   MachOp = 76
```

## val FSUB

```mach
pub val FSUB:   MachOp = 77
```

## val FMUL

```mach
pub val FMUL:   MachOp = 78
```

## val FDIV

```mach
pub val FDIV:   MachOp = 79
```

## val FNEG

```mach
pub val FNEG:   MachOp = 80
```

## val FMOV

```mach
pub val FMOV:   MachOp = 81
```

## val FCVT

```mach
pub val FCVT:   MachOp = 82
```

## val FCMP

```mach
pub val FCMP:   MachOp = 83
```

## val SCVTF

```mach
pub val SCVTF:  MachOp = 84
```

## val UCVTF

```mach
pub val UCVTF:  MachOp = 85
```

## val FCVTZS

```mach
pub val FCVTZS: MachOp = 86
```

## val FCVTZU

```mach
pub val FCVTZU: MachOp = 87
```

## val V_FADD

```mach
pub val V_FADD:  MachOp = 88
```

## val V_FSUB

```mach
pub val V_FSUB:  MachOp = 89
```

## val V_FMUL

```mach
pub val V_FMUL:  MachOp = 90
```

## val V_FDIV

```mach
pub val V_FDIV:  MachOp = 91
```

## val V_ADD

```mach
pub val V_ADD:   MachOp = 92
```

## val V_SUB

```mach
pub val V_SUB:   MachOp = 93
```

## val V_MUL

```mach
pub val V_MUL:   MachOp = 94
```

## val V_AND

```mach
pub val V_AND:   MachOp = 95
```

## val V_ORR

```mach
pub val V_ORR:   MachOp = 96
```

## val V_EOR

```mach
pub val V_EOR:   MachOp = 97
```

## val V_MVN

```mach
pub val V_MVN:   MachOp = 98
```

## val V_MOV

```mach
pub val V_MOV:   MachOp = 99
```

## val V_CMEQ

```mach
pub val V_CMEQ:  MachOp = 100
```

## val V_CMGT

```mach
pub val V_CMGT:  MachOp = 101
```

## val V_CMGE

```mach
pub val V_CMGE:  MachOp = 102
```

## val V_CMHI

```mach
pub val V_CMHI:  MachOp = 103
```

## val V_CMHS

```mach
pub val V_CMHS:  MachOp = 104
```

## val V_FCMEQ

```mach
pub val V_FCMEQ: MachOp = 105
```

## val V_FCMGT

```mach
pub val V_FCMGT: MachOp = 106
```

## val V_FCMGE

```mach
pub val V_FCMGE: MachOp = 107
```

## val UMOV

```mach
pub val UMOV:     MachOp = 108
```

lane moves: `umov` for a sub-word lane read, the `mov` alias for a word or
doubleword lane read, `mov` for every lane insert and the scalar dup

## val MOV_LANE

```mach
pub val MOV_LANE: MachOp = 109
```

## val INS

```mach
pub val INS:      MachOp = 110
```

## val DUP

```mach
pub val DUP:      MachOp = 111
```

## val FMOV_GEN

```mach
pub val FMOV_GEN: MachOp = 112
```

the other `fmov` pages: FMOV (general) between the banks, in either
direction, and FMOV (scalar, immediate); INS (element) beside INS (general)

## val FMOV_IMM

```mach
pub val FMOV_IMM: MachOp = 113
```

## val INS_EL

```mach
pub val INS_EL:   MachOp = 114
```

## val MOP_LAST

```mach
pub val MOP_LAST:  MachOp = INS_EL
```

## fun known

```mach
pub fun known(op: MachOp) bool;
```

## def Layout

```mach
pub def Layout: u8
```

the operand layout of a member: what its operands are, how they pack into
the word and how the printer lays the line out. this is the form byte the
printer-private record used to carry, now a column of the member's row

## val L_NONE

```mach
pub val L_NONE: Layout = 0
```

## val L_ALU3

```mach
pub val L_ALU3: Layout = 1
```

dst, src1, src2 registers, an optional src3 shift immediate; a row with an
immediate form takes a src2 immediate instead (add/sub/adds/subs)

## val L_BITFIELD

```mach
pub val L_BITFIELD: Layout = 2
```

the bitfield aliases: dst, src1 and the alias's own immediates

## val L_MOVEWIDE

```mach
pub val L_MOVEWIDE: Layout = 3
```

dst, #imm16, optional src3 `lsl #hw*16`

## val L_MADD

```mach
pub val L_MADD: Layout = 4
```

dst, src1, src2, src3 (the accumulator)

## val L_CSET

```mach
pub val L_CSET: Layout = 5
```

dst and the condition in the flags

## val L_LDST

```mach
pub val L_LDST: Layout = 6
```

a load names dst, [src1]; a store names [dst], src1. a memory operand with
an index is the register-offset form, otherwise the scaled unsigned offset

## val L_LDUR

```mach
pub val L_LDUR: Layout = 7
```

the unscaled signed-offset forms

## val L_LDORD

```mach
pub val L_LDORD: Layout = 8
```

the ordered loads and stores: [base] only

## val L_STXR

```mach
pub val L_STXR: Layout = 9
```

status dst, data src1, [src2]

## val L_LDSTP

```mach
pub val L_LDSTP: Layout = 10
```

a load names dst, src1, [src2]; a store names [dst], src1, src2; the
writeback in the flags

## val L_NEON_3SAME

```mach
pub val L_NEON_3SAME: Layout = 11
```

dst, src1, src2 vectors at the arrangement the element width names

## val L_NEON_2MISC

```mach
pub val L_NEON_2MISC: Layout = 12
```

dst, src1 vectors, always .16b

## val L_NEON_LANE_RD

```mach
pub val L_NEON_LANE_RD: Layout = 13
```

dst, src1.elem[src2]

## val L_NEON_LANE_WR

```mach
pub val L_NEON_LANE_WR: Layout = 14
```

dst.elem[src2], src1 or dst.elem[src2], src1.elem[src3]

## val L_BRANCH

```mach
pub val L_BRANCH: Layout = 15
```

optional src1 register (cbz/cbnz), the target in src2

## val L_BRANCH_REG

```mach
pub val L_BRANCH_REG: Layout = 16
```

src1 register, or none for the plain `ret`

## val L_ADRP

```mach
pub val L_ADRP:    Layout = 17
```

dst, a page symbol in src1

## val L_NULLARY

```mach
pub val L_NULLARY: Layout = 18
```

## val L_IMM16

```mach
pub val L_IMM16: Layout = 19
```

a src1 immediate

## val L_BARRIER

```mach
pub val L_BARRIER: Layout = 20
```

the barrier domain in src1

## val L_SYSREG

```mach
pub val L_SYSREG: Layout = 21
```

mrs dst, src1(sysreg); msr src1(sysreg), src2 register; msr src1(pstate
field), src2 immediate

## val L_FMOV_IMM

```mach
pub val L_FMOV_IMM: Layout = 22
```

dst, the imm8 in src2

## def WidthRule

```mach
pub def WidthRule: u8
```

how the operand widths reach the base word

## val W_NONE

```mach
pub val W_NONE: WidthRule = 0
```

## val W_SF

```mach
pub val W_SF: WidthRule = 1
```

bit 31 set for a 64-bit general-purpose operation

## val W_SF_N

```mach
pub val W_SF_N: WidthRule = 2
```

sf and the bitfield N bit

## val W_FTYPE

```mach
pub val W_FTYPE: WidthRule = 3
```

bit 22 set for a double-precision float operation

## val W_SF_FTYPE

```mach
pub val W_SF_FTYPE: WidthRule = 4
```

sf from the general-purpose operand, ftype from the float operand

## val W_FCVT

```mach
pub val W_FCVT: WidthRule = 5
```

ftype from the source float width, opc from the destination float width

## val W_SIZE

```mach
pub val W_SIZE: WidthRule = 6
```

the load/store size field from the data register, with the vector bank bit
and the quadword opc

## val W_SIZE30

```mach
pub val W_SIZE30: WidthRule = 7
```

bit 30 set for a 64-bit data register

## val W_LDP

```mach
pub val W_LDP: WidthRule = 8
```

the pair opc from the data register's bank and width

## val W_FMOV_GEN

```mach
pub val W_FMOV_GEN: WidthRule = 9
```

FMOV (general): sf and ftype as W_SF_FTYPE, the direction from the banks

## def LaneClass

```mach
pub def LaneClass: u8
```

the lane class of a NEON member: how the element width reaches the size
field and which arrangement the printer renders

## val LANE_NONE

```mach
pub val LANE_NONE:  LaneClass = 0
```

## val LANE_INT

```mach
pub val LANE_INT:   LaneClass = 1
```

## val LANE_FLOAT

```mach
pub val LANE_FLOAT: LaneClass = 2
```

## val LANE_BITS

```mach
pub val LANE_BITS: LaneClass = 3
```

the bitwise members: always the byte arrangement

## rec Form

```mach
pub rec Form;
```

one member's row: the spelling, the operand layout, the base word of its
encoding (and of its immediate form when the instruction has one), and how
the operand widths reach the word. an alias row has no base word: the
encoder assembles the member it aliases and the notification renames it

## fun form

```mach
pub fun form(op: MachOp) *Form;
```

the row of a member; the blank row outside the catalog

## fun mnemonic

```mach
pub fun mnemonic(op: MachOp) opt[str];
```

the assembler spelling the printer renders; absent outside the catalog

## fun layout

```mach
pub fun layout(op: MachOp) Layout;
```

## val FLAG_COND

```mach
pub val FLAG_COND:    u16 = 0x000F
```

`isa.Inst.flags` layout for an aarch64 notification

## val FLAG_WB_PRE

```mach
pub val FLAG_WB_PRE:  u16 = 0x0010
```

## val FLAG_WB_POST

```mach
pub val FLAG_WB_POST: u16 = 0x0020
```

## val FLAG_TARGET_BLOCK

```mach
pub val FLAG_TARGET_BLOCK: u16 = 0x0040
```

the branch target in src2 is a block of the function (a block label);
without it a src2 label is an offset inside the same expansion

## val FLAG_SP

```mach
pub val FLAG_SP:         u16 = 0x0080
```

index 31 in a general-purpose register operand names the stack pointer;
without it index 31 is the zero register

## val FLAG_ELEM

```mach
pub val FLAG_ELEM:       u16 = 0x0F00
```

## val FLAG_LABEL_LOCAL

```mach
pub val FLAG_LABEL_LOCAL: u16 = 0x1000
```

the src2 label is a numbered inline-asm local (`sym_id` holds the number,
FLAG_LABEL_FWD its direction) or a pc-relative skip (`disp` holds the bytes)

## val FLAG_LABEL_FWD

```mach
pub val FLAG_LABEL_FWD:   u16 = 0x2000
```

## val FLAG_LABEL_SKIP

```mach
pub val FLAG_LABEL_SKIP:  u16 = 0x4000
```

## fun cond_of

```mach
pub fun cond_of(flags: u16) u32;
```

## fun with_cond

```mach
pub fun with_cond(flags: u16, cc: u32) u16;
```

## fun elem_bytes

```mach
pub fun elem_bytes(flags: u16) u8;
```

the vector element width in bytes, 0 for a scalar instruction

## fun with_elem_bytes

```mach
pub fun with_elem_bytes(flags: u16, eb: u8) u16;
```

## fun gp_id

```mach
pub fun gp_id(n: u32) i32;
```

## fun vec_id

```mach
pub fun vec_id(n: u32) i32;
```

## fun gp

```mach
pub fun gp(n: u32, bytes: u8) isa.Operand;
```

## fun vec

```mach
pub fun vec(n: u32, bytes: u8) isa.Operand;
```

## fun is_gp

```mach
pub fun is_gp(op: *isa.Operand) bool;
```

## fun is_vec

```mach
pub fun is_vec(op: *isa.Operand) bool;
```

## fun is_zero_reg

```mach
pub fun is_zero_reg(mi: *isa.Inst, op: *isa.Operand) bool;
```

index 31 in a data position with the stack pointer not named

## fun base_word

```mach
pub fun base_word(mi: *isa.Inst) u32;
```

the base word with the operand widths applied: the row's rule names which
operands the size bits come from

## fun assemble

```mach
pub fun assemble(mi: *isa.Inst) u32;
```

the instruction word of a notification-shaped instruction: the row's base
word with the widths applied and the operands packed by the layout. the
operands are the assembler's, so an alias member (never assembled) and a
member whose row cannot hold the operands it was given assemble to none

## fun signed_at_width

```mach
pub fun signed_at_width(v: u64, wide: bool) i64;
```

the value of a move-wide fold as the register holds it: a 32-bit pattern
with its top bit set is negative at that width

## fun spell

```mach
pub fun spell(mi: *isa.Inst);
```

the spelling an assembler reads back for an encoded member: the alias the
preferred disassembly uses, with the operands laid out as the alias names
them. the encoding member is what `assemble` packs; the notification
carries the result of this

