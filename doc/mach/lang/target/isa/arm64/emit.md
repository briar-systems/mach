# mach.lang.target.isa.arm64.emit

arm64 instructions built and appended to the encode buffer, with the relocations they carry

## val ZR

```mach
pub val ZR: u32 = arm64.SP::u32
```

## val SP_REG

```mach
pub val SP_REG: u32 = arm64.SP::u32
```

## val SCRATCH0

```mach
pub val SCRATCH0: u32 = arm64.IP0::u32
```

## val SCRATCH1

```mach
pub val SCRATCH1: u32 = arm64.IP1::u32
```

## val FP_REG

```mach
pub val FP_REG: u32 = arm64.FP::u32
```

## val LR_REG

```mach
pub val LR_REG: u32 = arm64.LR::u32
```

## val MAX_CALLEE_SAVE_REACH

```mach
pub val MAX_CALLEE_SAVE_REACH: u32 = 504
```

## val MAX_FRAME_SUB

```mach
pub val MAX_FRAME_SUB: u32 = 0xFFFFF0
```

## val CC_EQ

```mach
pub val CC_EQ: u32 = 0
```

## val CC_NE

```mach
pub val CC_NE: u32 = 1
```

## val CC_HS

```mach
pub val CC_HS: u32 = 2
```

## val CC_CC

```mach
pub val CC_CC: u32 = 3
```

## val CC_MI

```mach
pub val CC_MI: u32 = 4
```

## val CC_PL

```mach
pub val CC_PL: u32 = 5
```

## val CC_VS

```mach
pub val CC_VS: u32 = 6
```

## val CC_VC

```mach
pub val CC_VC: u32 = 7
```

## val CC_HI

```mach
pub val CC_HI: u32 = 8
```

## val CC_LS

```mach
pub val CC_LS: u32 = 9
```

## val CC_GE

```mach
pub val CC_GE: u32 = 10
```

## val CC_LT

```mach
pub val CC_LT: u32 = 11
```

## val CC_GT

```mach
pub val CC_GT: u32 = 12
```

## val CC_LE

```mach
pub val CC_LE: u32 = 13
```

## val CC_AL

```mach
pub val CC_AL: u32 = 14
```

## val FP_SCRATCH0

```mach
pub val FP_SCRATCH0: u32 = arm64.V31::u32
```

## val FP_SCRATCH1

```mach
pub val FP_SCRATCH1: u32 = arm64.V30::u32
```

## val SYSREG_NAME_MAX

```mach
pub val SYSREG_NAME_MAX: usize = 32
```

## fun pstate_field

```mach
pub fun pstate_field(name: str, op1_out: *u32, op2_out: *u32) bool;
```

## fun sysreg

```mach
pub fun sysreg(name: str, op0_out: *u32, op1_out: *u32, crn_out: *u32, crm_out: *u32, op2_out: *u32) bool;
```

## rec Access

```mach
pub rec Access;
```

the access row of an integer or float load/store at one width: the
member for each addressing form, the register the data travels in and the
immediate scale, together with the `:lo12:` relocation kind. a width with no
row is refused

## fun int_access

```mach
pub fun int_access(w: u8, is_load: bool) res[Access, fail.Fail];
```

## fun fp_access

```mach
pub fun fp_access(w: u8, is_load: bool) res[Access, fail.Fail];
```

## fun width_bytes

```mach
pub fun width_bytes(use64: bool) u8;
```

## fun inst_of

```mach
pub fun inst_of(op: arm64_inst.MachOp, dst: isa_inst.Operand, src1: isa_inst.Operand, src2: isa_inst.Operand) isa_inst.Inst;
```

## fun inst

```mach
pub fun inst(st: *isa_encode.EncodeState, mi: *isa_inst.Inst);
```

every instruction word leaves through here: the row assembles the word
from the instruction, the word is emitted, and the same instruction, spelled
as an assembler reads it back, is what the stream is notified with. a member
whose row cannot hold what it was given emits nothing and refuses the buffer

## fun alu3

```mach
pub fun alu3(st: *isa_encode.EncodeState, op: arm64_inst.MachOp, use64: bool, rd: u32, rn: u32, rm: u32);
```

## fun alu3_sh

```mach
pub fun alu3_sh(st: *isa_encode.EncodeState, op: arm64_inst.MachOp, use64: bool, rd: u32, rn: u32, rm: u32, sh: u32);
```

## fun fp3

```mach
pub fun fp3(st: *isa_encode.EncodeState, op: arm64_inst.MachOp, w: u8, rd: u32, rn: u32, rm: u32);
```

the scalar float forms at w bytes: 2 is half precision (FEAT_FP16), 4 single, 8 double

## fun fp2

```mach
pub fun fp2(st: *isa_encode.EncodeState, op: arm64_inst.MachOp, w: u8, rd: u32, rn: u32);
```

## fun fcmp

```mach
pub fun fcmp(st: *isa_encode.EncodeState, w: u8, rn: u32, rm: u32);
```

## fun fcvt

```mach
pub fun fcvt(st: *isa_encode.EncodeState, to: u8, from: u8, rd: u32, rn: u32);
```

fcvt between any two of half, single and double; half on either side is
the base FP conversion, not FEAT_FP16

## fun scalar_float_width

```mach
pub fun scalar_float_width(w: u8) bool;
```

a scalar float width the encoder takes: an f16 at 2 bytes beside binary32 and binary64

## fun cvt

```mach
pub fun cvt(st: *isa_encode.EncodeState, op: arm64_inst.MachOp, dst: isa_inst.Operand, src: isa_inst.Operand);
```

a conversion between the banks: the operands carry the bank and width of
each side, the row's rule places them in the word

## fun addsub_imm

```mach
pub fun addsub_imm(st: *isa_encode.EncodeState, op: arm64_inst.MachOp, use64: bool, rd: u32, rn: u32, imm12: u32, sh: u32);
```

## fun addsub_imm_sym

```mach
pub fun addsub_imm_sym(st: *isa_encode.EncodeState, op: arm64_inst.MachOp, use64: bool, rd: u32, rn: u32, sym: intern.StrId, mod: isa_inst.SymModifier, addend: i64);
```

## fun bitfield

```mach
pub fun bitfield(st: *isa_encode.EncodeState, op: arm64_inst.MachOp, use64: bool, rd: u32, rn: u32, a: u32, b: u32);
```

a bitfield alias with its own immediates: a shift count for lsl/lsr/asr,
a position and a width for the extract and insert forms, none for the
extends (whose source is always the 32-bit register)

## fun movewide

```mach
pub fun movewide(st: *isa_encode.EncodeState, op: arm64_inst.MachOp, use64: bool, rd: u32, hw: u32, imm16: u32);
```

## fun madd

```mach
pub fun madd(st: *isa_encode.EncodeState, op: arm64_inst.MachOp, use64: bool, rd: u32, rn: u32, rm: u32, ra: u32);
```

## fun cset

```mach
pub fun cset(st: *isa_encode.EncodeState, use64: bool, rd: u32, cc: u32);
```

## fun ldst

```mach
pub fun ldst(st: *isa_encode.EncodeState, op: arm64_inst.MachOp, fp: bool, width: u8, rt: u32, rn: u32, imm12: u32);
```

## fun ldst_sym

```mach
pub fun ldst_sym(st: *isa_encode.EncodeState, op: arm64_inst.MachOp, fp: bool, width: u8, rt: u32, rn: u32, sym: intern.StrId, mod: isa_inst.SymModifier, addend: i64);
```

## fun ldur

```mach
pub fun ldur(st: *isa_encode.EncodeState, op: arm64_inst.MachOp, fp: bool, width: u8, rt: u32, rn: u32, imm9: u32);
```

## fun ldst_reg

```mach
pub fun ldst_reg(st: *isa_encode.EncodeState, op: arm64_inst.MachOp, fp: bool, width: u8, rt: u32, rn: u32, rm: u32);
```

## fun ldord

```mach
pub fun ldord(st: *isa_encode.EncodeState, op: arm64_inst.MachOp, use64: bool, rt: u32, rn: u32);
```

## fun ldstp

```mach
pub fun ldstp(st: *isa_encode.EncodeState, op: arm64_inst.MachOp, fp: bool, rt: u32, rt2: u32, rn: u32, imm7: u32, wb: u16);
```

## fun neon_3same

```mach
pub fun neon_3same(st: *isa_encode.EncodeState, op: arm64_inst.MachOp, rd: u32, rn: u32, rm: u32, lane: u32);
```

## fun neon_3diff

```mach
pub fun neon_3diff(st: *isa_encode.EncodeState, op: arm64_inst.MachOp, rd: u32, rn: u32, rm: u32, from_bytes: u8);
```

## fun neon_2misc

```mach
pub fun neon_2misc(st: *isa_encode.EncodeState, op: arm64_inst.MachOp, rd: u32, rn: u32);
```

## fun neon_2misc_lanes

```mach
pub fun neon_2misc_lanes(st: *isa_encode.EncodeState, op: arm64_inst.MachOp, rd: u32, rn: u32, lane: u32);
```

a 2misc member at the lane's arrangement

## fun neon_shift_imm

```mach
pub fun neon_shift_imm(st: *isa_encode.EncodeState, op: arm64_inst.MachOp, rd: u32, rn: u32, sh: u32, lane: u32);
```

## fun neon_dup_gp

```mach
pub fun neon_dup_gp(st: *isa_encode.EncodeState, rd: u32, rn: u32, lane: u32);
```

every lane of `rd` the general register's low lane: w for lanes up to 32 bits, x for 64

## fun neon_conv

```mach
pub fun neon_conv(st: *isa_encode.EncodeState, op: arm64_inst.MachOp, rd: u32, rn: u32, from_bytes: u8);
```

## fun neon_umov

```mach
pub fun neon_umov(st: *isa_encode.EncodeState, index: u32, rd: u32, rn: u32, lane: u32);
```

## fun neon_dup_el

```mach
pub fun neon_dup_el(st: *isa_encode.EncodeState, index: u32, rd: u32, rn: u32, lane: u32);
```

## fun neon_ins_gp

```mach
pub fun neon_ins_gp(st: *isa_encode.EncodeState, index: u32, rd: u32, rn: u32, lane: u32, src_bytes: u8);
```

## fun neon_ins_el

```mach
pub fun neon_ins_el(st: *isa_encode.EncodeState, index: u32, src_index: u32, rd: u32, rn: u32, lane: u32);
```

## fun branch_block

```mach
pub fun branch_block(st: *isa_encode.EncodeState, op: arm64_inst.MachOp, cc: u32, use64: bool, rt: u32, block: u32);
```

## fun branch_sym

```mach
pub fun branch_sym(st: *isa_encode.EncodeState, op: arm64_inst.MachOp, sym: intern.StrId, addend: i64);
```

## fun branch_pcrel

```mach
pub fun branch_pcrel(st: *isa_encode.EncodeState, op: arm64_inst.MachOp, cc: u32, use64: bool, rt: u32, imm19: u32);
```

a pc-relative skip inside the same expansion: the target travels as a
local label whose displacement is the bytes skipped

## fun branch_local

```mach
pub fun branch_local(st: *isa_encode.EncodeState, op: arm64_inst.MachOp, cc: u32, use64: bool, rt: u32, number: u32, fwd: bool);
```

a numbered inline-asm local: the number travels in the label's sym_id

## fun branch_reg

```mach
pub fun branch_reg(st: *isa_encode.EncodeState, op: arm64_inst.MachOp, rn: u32);
```

## fun return_lr

```mach
pub fun return_lr(st: *isa_encode.EncodeState);
```

## fun adrp

```mach
pub fun adrp(st: *isa_encode.EncodeState, rd: u32, sym: intern.StrId, mod: isa_inst.SymModifier, addend: i64);
```

## fun nullary

```mach
pub fun nullary(st: *isa_encode.EncodeState, op: arm64_inst.MachOp);
```

## fun imm16_inst

```mach
pub fun imm16_inst(st: *isa_encode.EncodeState, op: arm64_inst.MachOp, imm16: u32);
```

## fun barrier_op

```mach
pub fun barrier_op(st: *isa_encode.EncodeState, op: arm64_inst.MachOp, crm: u32);
```

dmb, dsb and isb: one word whose CRm is the barrier option

## fun msr_imm

```mach
pub fun msr_imm(st: *isa_encode.EncodeState, op1: u32, op2: u32, imm: u32);
```

## fun sysreg_pack

```mach
pub fun sysreg_pack(op0: u32, op1: u32, crn: u32, crm: u32, op2: u32) u32;
```

## fun mrs

```mach
pub fun mrs(st: *isa_encode.EncodeState, packed: u32, rt: u32);
```

## fun msr_reg

```mach
pub fun msr_reg(st: *isa_encode.EncodeState, packed: u32, rt: u32);
```

## fun stxr

```mach
pub fun stxr(st: *isa_encode.EncodeState, use64: bool, rs: u32, rt: u32, rn: u32);
```

## fun is64

```mach
pub fun is64(width: u8) bool;
```

## fun const_words

```mach
pub fun const_words(value: u64, use64: bool) u32;
```

the instructions a constant takes: one orr from the zero register for a
logical immediate the move-wide plan needs more words for, else that plan

## fun int_imm_fits

```mach
pub fun int_imm_fits(value: u64, bits: u32) bool;
```

whether one instruction builds the integer: one movz or movn, or one orr of
a logical immediate, the rule the middle end hoists a loop's constants by
. a value narrower than 64 bits is built at 32

## fun movewide_seq

```mach
pub fun movewide_seq(st: *isa_encode.EncodeState, rd: u32, value: u64, use64: bool);
```

## fun mem_access_needs_no_scratch

```mach
pub fun mem_access_needs_no_scratch(off: i64, width: u8) res[bool, fail.Fail];
```

## fun mem_access

```mach
pub fun mem_access(st: *isa_encode.EncodeState, rt: u32, base: u32, off: i64, width: u8, is_load: bool) err[fail.Fail];
```

## fun adrp_reloc

```mach
pub fun adrp_reloc(st: *isa_encode.EncodeState, rd: u32, sym: intern.StrId, addend: i64) err[fail.Fail];
```

## fun adrp_got_reloc

```mach
pub fun adrp_got_reloc(st: *isa_encode.EncodeState, rd: u32, sym: intern.StrId) err[fail.Fail];
```

## fun neon_ext

```mach
pub fun neon_ext(st: *isa_encode.EncodeState, rd: u32, rn: u32, rm: u32, first: u32);
```

## fun patch_branch

```mach
pub fun patch_branch(st: *isa_encode.EncodeState, fx: *isa_encode.BranchFixup, target_off: u32) err[fail.Fail];
```

## fun reloc_offset

```mach
pub fun reloc_offset(inst_start: u32) u32;
```

## fun neon_elem

```mach
pub fun neon_elem(st: *isa_encode.EncodeState, op: arm64_inst.MachOp, eb: u8, rt: u32, rn: u32, post: bool, rm: i32);
```

ld1/st1 of one eb-laned register at [rn]; a post-index advances rn by 16, or by
register id `rm` when it names one

