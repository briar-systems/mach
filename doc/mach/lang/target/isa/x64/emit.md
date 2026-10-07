# mach.lang.target.isa.x64.emit

x64 instructions encoded to bytes in the encode buffer, and the vector forms built straight into it

## fun operand_touches_reg

```mach
pub fun operand_touches_reg(op: *isa_inst.Operand, reg: i32) bool;
```

## fun pick_stage_scratch

```mach
pub fun pick_stage_scratch(a: *isa_inst.Operand, b: *isa_inst.Operand) i32;
```

## fun pick_stage_scratch_besides

```mach
pub fun pick_stage_scratch_besides(a: *isa_inst.Operand, b: *isa_inst.Operand, taken: i32) i32;
```

r11, else r10: a staging scratch neither operand touches and other than
`taken`, REG_NONE when none is free

## fun store_target

```mach
pub fun store_target(op: *isa_inst.Operand) *isa_inst.Operand;
```

a destination written only after the staging: its address registers must
survive it, a register it names need not

## fun encode_fp_mov

```mach
pub fun encode_fp_mov(mi: *isa_inst.Inst, buf: *isa_encode.ByteBuf) res[i32, fail.Fail];
```

## fun staged_source

```mach
pub fun staged_source(mi: *isa_inst.Inst) bool;
```

a memory source beside a memory destination is staged through a scratch
register where the row admits one r/m operand: a shift's count and a swap's
second operand are never staged

## fun staging_load

```mach
pub fun staging_load(mi: *isa_inst.Inst) opt[isa_inst.Inst];
```

the load staging a source into the scratch the instruction then reads, none
when no scratch is free

## fun encode_inst

```mach
pub fun encode_inst(mi: *isa_inst.Inst, buf: *isa_encode.ByteBuf) i32;
```

## fun string_row

```mach
pub fun string_row(op: u16) StringRow;
```

the row of a string opcode; a width of 0 for anything else

## fun is_rip_mem

```mach
pub fun is_rip_mem(op: *isa_inst.Operand) bool;
```

## fun reloc_offset

```mach
pub fun reloc_offset(ctx: ptr, mi: *isa_inst.Inst, inst_size: i32) i32;
```

## val VARIANT_VEX

```mach
pub val VARIANT_VEX: u8 = 1
```

the variant that writes every sse instruction in its vex form: a legacy one
that writes an xmm register while a ymm register's upper half holds state
pays a state merge on each, well past a hundred cycles on Zen 3, so under avx
no xmm instruction the compiler emits is legacy-encoded

## fun vex_encoding

```mach
pub fun vex_encoding(buf: *isa_encode.ByteBuf) bool;
```

## fun float_scratch0

```mach
pub fun float_scratch0() i32;
```

## fun float_reg_or_slot_load

```mach
pub fun float_reg_or_slot_load(op: isa_inst.Operand, xmm: i32, w: u8, buf: *isa_encode.ByteBuf) err[fail.Fail];
```

## fun float_store

```mach
pub fun float_store(dst: isa_inst.Operand, xmm: i32, w: u8, buf: *isa_encode.ByteBuf) err[fail.Fail];
```

## fun half_move

```mach
pub fun half_move(dst: isa_inst.Operand, src: isa_inst.Operand, buf: *isa_encode.ByteBuf) err[fail.Fail];
```

an f16 lives in the low word of an xmm register: a register copy
moves the whole register, memory is reached as a word by pinsrw and pextrw,
and a general register by movd, whose bits above the word are no part of
the value

## fun patch_rel32

```mach
pub fun patch_rel32(buf: *isa_encode.ByteBuf, pos: usize, disp: i32);
```

## fun movd_movq_reg

```mach
pub fun movd_movq_reg(gp: i32, xmm: i32, w: u8, gp_to_xmm: bool, buf: *isa_encode.ByteBuf);
```

## fun packed_shift_count_opbyte

```mach
pub fun packed_shift_count_opbyte(op: u16) opt[u8];
```

the opcode byte of a packed shift whose count is an xmm register's low
quadword; absent for an opcode with no such form

## val VB_XMM

```mach
pub val VB_XMM: u8 = 16
```

the register widths a packed instruction runs at: 16 is the legacy sse
encoding over an xmm register, 32 the vex.256 encoding over a ymm one

## val VB_YMM

```mach
pub val VB_YMM: u8 = 32
```

## fun vec_reg

```mach
pub fun vec_reg(xmm: i32, vb: u8) isa_inst.Operand;
```

## fun vex_l

```mach
pub fun vex_l(vb: u8) u8;
```

the vex prefix's vector length for a register width

## fun note_vec

```mach
pub fun note_vec(buf: *isa_encode.ByteBuf, start: usize, opcode: u16, flags: u16, dst: isa_inst.Operand, src1: isa_inst.Operand,
src2: isa_inst.Operand, src3: isa_inst.Operand, vb: u8);
```

one packed instruction for the listing: its operands in intel order, the
width it runs at, and the encoding flags (FLAG_VEX for the vex form of a
legacy opcode)

## fun note_packed

```mach
pub fun note_packed(buf: *isa_encode.ByteBuf, start: usize, opcode: u16, dst: isa_inst.Operand, src: isa_inst.Operand);
```

## fun note_packed_imm

```mach
pub fun note_packed_imm(buf: *isa_encode.ByteBuf, start: usize, opcode: u16, dst: isa_inst.Operand, src: isa_inst.Operand, imm: u8);
```

## val PACKED_OPCODE_UNKNOWN

```mach
pub val PACKED_OPCODE_UNKNOWN: str = "encode: x64.Opcode outside the packed SSE2 catalog"
```

an x64.Opcode no packed row names is a member the selector produced outside
the packed SSE2 catalog: internal. no diagnostic owner reaches this depth, so
the text names the catalog and the surrounding function names the site

## val PACKED_WIDTH_UNKNOWN

```mach
pub val PACKED_WIDTH_UNKNOWN: str = "encode: a packed vector op at a register width x86-64 has no encoding for"
```

## fun packed_op

```mach
pub fun packed_op(op: u16, vb: u8, dst_xmm: i32, src: isa_inst.Operand, buf: *isa_encode.ByteBuf) err[fail.Fail];
```

## fun packed_as

```mach
pub fun packed_as(op: u16, vb: u8, dst: isa_inst.Operand, src: isa_inst.Operand, buf: *isa_encode.ByteBuf) err[fail.Fail];
```

a packed op whose destination is not the width it runs at: a conversion
that halves or doubles its lanes' width writes, or reads, the xmm half

## fun packed_imm

```mach
pub fun packed_imm(op: u16, vb: u8, dst_xmm: i32, src: isa_inst.Operand, imm: u8, buf: *isa_encode.ByteBuf) err[fail.Fail];
```

## fun packed_rri

```mach
pub fun packed_rri(op: u16, vb: u8, dst_xmm: i32, src_xmm: i32, imm: u8, buf: *isa_encode.ByteBuf) err[fail.Fail];
```

## fun vex_three

```mach
pub fun vex_three(buf: *isa_encode.ByteBuf, vb: u8, dst: isa_inst.Operand, lhs: isa_inst.Operand) bool;
```

the vex form names its first source apart from its destination, so a two-
source op whose destination and first source are registers is one
instruction there, with no copy of the first source into the destination

## fun packed_three

```mach
pub fun packed_three(op: u16, vb: u8, dst: i32, lhs: i32, rhs: isa_inst.Operand, imm: isa_inst.Operand, buf: *isa_encode.ByteBuf) err[fail.Fail];
```

dst = lhs op rhs as one vex instruction, rhs a register or memory, with an
immediate after it when `imm` is one

## fun mapped_opbytes

```mach
pub fun mapped_opbytes(op: u16, prefix: *u8, map: *u8, opbyte: *u8) bool;
```

the mandatory prefix, the 0F 38 or 0F 3A map byte and the opcode byte of an
ssse3, sse41, sha, aes or pclmul row; false for an opcode outside those maps

## fun sse_rm

```mach
pub fun sse_rm(prefix: u8, opbyte: u8, reg: i32, rm: isa_inst.Operand, buf: *isa_encode.ByteBuf);
```

one legacy-prefixed 0F-map instruction with a register reg field and a register or memory rm

## fun sse_map_rm

```mach
pub fun sse_map_rm(prefix: u8, map: u8, opbyte: u8, reg: i32, rm: isa_inst.Operand, buf: *isa_encode.ByteBuf);
```

as emit_sse_rm, with a 0F 38 or 0F 3A escape when `map` is nonzero

## rec OpMap

```mach
pub rec OpMap;
```

where an instruction sits in the opcode space beyond its register operands:
the mandatory prefix (none, 66, f3 or f2), the escape map (0 for 0F, 0x38
for 0F 38, 0x3A for 0F 3A), the opcode byte and the W bit. the legacy
encoding spells the prefix and map as bytes, and the vex and evex prefixes
fold both into fields, so one row serves every encoding of an opcode

## fun vex_rm

```mach
pub fun vex_rm(m: OpMap, l: u8, reg: i32, vvvv: i32, rm: isa_inst.Operand, buf: *isa_encode.ByteBuf);
```

one vex-encoded instruction: `reg` in modrm.reg, `vvvv` the extra source
register (REG_NONE for none), and `rm` a register or memory. `l` is the vector
length, 0 for 128 bits and 1 for 256. the two-byte c5 prefix carries only
R, vvvv, L and pp, so W, the 0F 38 and 0F 3A maps and an extended index or
base need the three-byte c4 prefix. the extension bits and vvvv are stored
inverted

## fun vec3

```mach
pub fun vec3(op: u16, vb: u8, dst_xmm: i32, src1_xmm: i32, src2: isa_inst.Operand, buf: *isa_encode.ByteBuf) err[fail.Fail];
```

a three-operand instruction at `vb`, dst from src1 (vvvv) and src2 (rm),
vex-encoded where the opcode has a vex row and evex-encoded, unmasked,
where it has an evex one

## fun vec3_ops

```mach
pub fun vec3_ops(op: u16, vb: u8, dst: isa_inst.Operand, lhs: isa_inst.Operand, rhs: isa_inst.Operand, buf: *isa_encode.ByteBuf) err[fail.Fail];
```

a three-operand instruction over codegen operands: the first source rides
vvvv and so must be a register, the second is a register or frame slot, and
the result forms in the destination register, or in scratch0 for a frame slot

## fun vec_sized

```mach
pub fun vec_sized(op: isa_inst.Operand, vb: u8) isa_inst.Operand;
```

a register or frame-slot operand read or written at `vb` bytes

## fun vec_move

```mach
pub fun vec_move(op: u16, dst: isa_inst.Operand, src: isa_inst.Operand, buf: *isa_encode.ByteBuf) err[fail.Fail];
```

a packed move between a vector register and a vector register or memory, in either direction

## fun packed_shift_imm

```mach
pub fun packed_shift_imm(op: u16, vb: u8, xmm: i32, imm: u8, buf: *isa_encode.ByteBuf) err[fail.Fail];
```

a packed shift by an immediate: the legacy form shifts its register in
place, and the vex.256 form names the destination in vvvv and the source in
modrm.rm, both the one register here, with the opcode digit in modrm.reg

## fun movaps_rr

```mach
pub fun movaps_rr(dst_xmm: i32, src_xmm: i32, vb: u8, buf: *isa_encode.ByteBuf);
```

a register copy: movaps, or vmovaps over the ymm register, VEX.256.0F 28
with the source in modrm.rm or 29 with it in modrm.reg: an extended source
into a low destination takes 29, whose extension bit R the two-byte prefix
carries, as the assembler encodes it

## fun vec_load

```mach
pub fun vec_load(op: isa_inst.Operand, xmm: i32, vb: u8, buf: *isa_encode.ByteBuf) err[fail.Fail];
```

a vector operand into a register at `vb`: a register copy, or a load from
its frame slot, movaps at 16 bytes and vmovdqu at 32

## fun vec_store

```mach
pub fun vec_store(dst: isa_inst.Operand, xmm: i32, vb: u8, buf: *isa_encode.ByteBuf) err[fail.Fail];
```

## fun vec_all_ones

```mach
pub fun vec_all_ones(xmm: i32, vb: u8, buf: *isa_encode.ByteBuf) err[fail.Fail];
```

## fun vec_zero

```mach
pub fun vec_zero(xmm: i32, vb: u8, buf: *isa_encode.ByteBuf) err[fail.Fail];
```

a cleared register: pxor, and at 32 bytes vxorps, which avx has for a float
ymm value where the integer vpxor is avx2

## fun vextract128

```mach
pub fun vextract128(dst_xmm: i32, src_ymm: i32, hi: u8, buf: *isa_encode.ByteBuf);
```

the 128-bit half `hi` of a ymm register into an xmm one: vextractf128,
VEX.256.66.0F3A.W0 19 /r ib with the source in modrm.reg

## fun vinsert128

```mach
pub fun vinsert128(dst_ymm: i32, lo_ymm: i32, half: isa_inst.Operand, hi: u8, buf: *isa_encode.ByteBuf);
```

a ymm register from `lo_ymm` with its half `hi` replaced by an xmm register
or 16 bytes of memory: vinsertf128, VEX.256.66.0F3A.W0 18 /r ib

## fun vpermq

```mach
pub fun vpermq(dst_ymm: i32, src: isa_inst.Operand, order: u8, buf: *isa_encode.ByteBuf);
```

the quadwords of a ymm register or memory, each result quadword the one its
two-bit field of `order` names: vpermq, VEX.256.66.0F3A.W1 00 /r ib (avx2)

