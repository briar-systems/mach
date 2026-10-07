# mach.lang.target.isa.x64.inst

the x64 instruction catalog: what each opcode is, the form it encodes through, the
operands it reads and writes and the flags it touches

## val FLAG_REX_W

```mach
pub val FLAG_REX_W: EncFlag = 1
```

## val FLAG_16BIT

```mach
pub val FLAG_16BIT: EncFlag = 2
```

## val FLAG_LOCK

```mach
pub val FLAG_LOCK: EncFlag = 4
```

## val FLAG_REP

```mach
pub val FLAG_REP: EncFlag = 8
```

## val FLAG_INDIRECT

```mach
pub val FLAG_INDIRECT: EncFlag = 16
```

## val FLAG_REPNE

```mach
pub val FLAG_REPNE: EncFlag = 64
```

the f2 prefix of a string instruction: repne, repeated while zf is clear.
FLAG_REP is f3, which a compare reads as repe

## val FLAG_VEX

```mach
pub val FLAG_VEX: EncFlag = 128
```

the vex encoding of a legacy packed opcode: the destination, the sources in
intel order after it, and the mnemonic spelled with its v

## val MEM_BASE_ABS

```mach
pub val MEM_BASE_ABS: i32 = -2
```

## val SEG_NONE

```mach
pub val SEG_NONE: Segment = 0
```

## val SEG_FS

```mach
pub val SEG_FS: Segment = 1
```

## val SEG_GS

```mach
pub val SEG_GS: Segment = 2
```

## rec SegmentRow

```mach
pub rec SegmentRow;
```

## val SEGMENT_COUNT

```mach
pub val SEGMENT_COUNT: usize = 2
```

## val SEGMENTS

```mach
pub val SEGMENTS: [SEGMENT_COUNT]SegmentRow = [SEGMENT_COUNT]SegmentRow;
```

## fun segment_row

```mach
pub fun segment_row(seg: u8) *SegmentRow;
```

the row of a segment override; nil for SEG_NONE and anything unassigned

## def Form

```mach
pub def Form: u8
```

the instruction form of an opcode: the operand shape the encoder admits and
the encoder family that emits it. every opcode the stream can carry has
exactly one; FORM_NONE is the absence of a row and every reader refuses it

## val FORM_NONE

```mach
pub val FORM_NONE: Form = 0
```

## val FORM_BARE

```mach
pub val FORM_BARE: Form = 1
```

fixed bytes, no explicit operand

## val FORM_MOV

```mach
pub val FORM_MOV: Form = 2
```

mov: dst r/m, src r/m, imm or sym

## val FORM_LEA

```mach
pub val FORM_LEA: Form = 3
```

lea: dst r, src m taken as an address

## val FORM_PUSH

```mach
pub val FORM_PUSH: Form = 4
```

push: src r or imm, through the stack

## val FORM_POP

```mach
pub val FORM_POP: Form = 5
```

pop: dst r, through the stack

## val FORM_ALU

```mach
pub val FORM_ALU: Form = 6
```

add sub and or xor cmp: dst r/m, src r/m or imm

## val FORM_TEST

```mach
pub val FORM_TEST: Form = 7
```

test: dst r/m, src r or imm

## val FORM_IMUL

```mach
pub val FORM_IMUL: Form = 8
```

imul: src r/m into rdx:rax, dst r with src r/m, or dst r with src r/m and imm

## val FORM_DIV

```mach
pub val FORM_DIV: Form = 9
```

idiv div: src r/m over rdx:rax

## val FORM_SHIFT

```mach
pub val FORM_SHIFT: Form = 10
```

shl shr sar: dst r/m, count imm or cl

## val FORM_JCC

```mach
pub val FORM_JCC: Form = 11
```

jcc: a label

## val FORM_JMP

```mach
pub val FORM_JMP: Form = 12
```

jmp: a label, a sym, or r/m

## val FORM_CALL

```mach
pub val FORM_CALL: Form = 13
```

call: a sym or r/m, through the stack

## val FORM_RET

```mach
pub val FORM_RET: Form = 14
```

ret iretq: through the stack

## val FORM_SETCC

```mach
pub val FORM_SETCC: Form = 15
```

setcc: dst r8

## val FORM_MOVX

```mach
pub val FORM_MOVX: Form = 16
```

movzx movsx movsxd: dst r, src r/m

## val FORM_MOVABS

```mach
pub val FORM_MOVABS: Form = 17
```

movabs: dst r, imm64 or sym

## val FORM_MOVQ

```mach
pub val FORM_MOVQ: Form = 18
```

movq: r to xmm or xmm to r

## val FORM_SSE_MOV

```mach
pub val FORM_SSE_MOV: Form = 19
```

movss movsd: xmm to xmm or m, m to xmm

## val FORM_SSE_MOVA

```mach
pub val FORM_SSE_MOVA: Form = 20
```

movaps movups: xmm to xmm or m128, m128 to xmm

## val FORM_SSE_ALU

```mach
pub val FORM_SSE_ALU: Form = 21
```

scalar sse arithmetic: dst xmm, src xmm or m

## val FORM_UCOMIS

```mach
pub val FORM_UCOMIS: Form = 22
```

ucomiss ucomisd: xmm, xmm or m

## val FORM_CVT

```mach
pub val FORM_CVT: Form = 23
```

scalar conversions between r and xmm

## val FORM_XORPS

```mach
pub val FORM_XORPS: Form = 24
```

xorps: dst xmm, src xmm or m128

## val FORM_PACKED

```mach
pub val FORM_PACKED: Form = 25
```

packed sse2 arithmetic, logic and compare: dst xmm, src xmm or m128

## val FORM_PSHUF

```mach
pub val FORM_PSHUF: Form = 26
```

pshufd: dst xmm, src xmm, imm8

## val FORM_PSLL

```mach
pub val FORM_PSLL: Form = 27
```

pslld psllw psraw psrad: dst xmm, imm8

## val FORM_GRP3

```mach
pub val FORM_GRP3: Form = 28
```

neg not: r/m

## val FORM_GRP5

```mach
pub val FORM_GRP5: Form = 29
```

inc dec: r/m

## val FORM_XCHG

```mach
pub val FORM_XCHG: Form = 30
```

xchg: r, r/m

## val FORM_XADD

```mach
pub val FORM_XADD: Form = 31
```

xadd cmpxchg: r/m, r

## val FORM_PORTIO

```mach
pub val FORM_PORTIO: Form = 32
```

in out: al ax eax with dx or imm8

## val FORM_LIDT

```mach
pub val FORM_LIDT: Form = 33
```

lidt: m

## val FORM_MOV_CR

```mach
pub val FORM_MOV_CR: Form = 34
```

mov to or from a control register

## val FORM_MOV_SEG

```mach
pub val FORM_MOV_SEG: Form = 35
```

mov to or from a segment register

## val FORM_STRING

```mach
pub val FORM_STRING: Form = 36
```

movs stos lods cmps scas: through rsi, rdi or both, rcx times under a rep prefix

## val FORM_TRAP

```mach
pub val FORM_TRAP: Form = 37
```

ud2 hlt: never continues

## val FORM_DATA

```mach
pub val FORM_DATA: Form = 38
```

a data byte in the stream (raw_byte), not an instruction

## val FORM_FSGSBASE

```mach
pub val FORM_FSGSBASE: Form = 39
```

rdfsbase rdgsbase wrfsbase wrgsbase: r32 or r64

## val FORM_PINSR

```mach
pub val FORM_PINSR: Form = 40
```

pinsrd pextrd: an xmm and an r32 or m32, with imm8

## val FORM_MUL1

```mach
pub val FORM_MUL1: Form = 41
```

mul: src r/m by rax into rdx:rax

## val FORM_BITSCAN

```mach
pub val FORM_BITSCAN: Form = 42
```

bsf bsr popcnt lzcnt tzcnt: a 16-, 32- or 64-bit register from an r/m of the same width

## val FORM_BSWAP

```mach
pub val FORM_BSWAP: Form = 43
```

bswap: one 32- or 64-bit register, reversed in place

## val FORM_DSHIFT

```mach
pub val FORM_DSHIFT: Form = 44
```

shld shrd: an r/m, the register feeding bits, and a count of imm8 or cl

## val FORM_CARRY

```mach
pub val FORM_CARRY: Form = 45
```

sbb: r/m less a register or imm and the carry, which it reads

## val FORM_CMOV

```mach
pub val FORM_CMOV: Form = 46
```

cmovcc: an r/m moved into a register on a flags condition, which it reads

## val FORM_VEX3

```mach
pub val FORM_VEX3: Form = 47
```

a vex or evex three-operand packed instruction: dst xmm, src1 xmm in vvvv, src2 xmm or m128

## def Shape

```mach
pub def Shape: u8
```

the role pattern of the explicit operands, in intel order

## val SHAPE_NONE

```mach
pub val SHAPE_NONE: Shape = 0
```

## val SHAPE_DEF

```mach
pub val SHAPE_DEF: Shape = 1
```

the first operand is written, the second read

## val SHAPE_RW

```mach
pub val SHAPE_RW: Shape = 2
```

the first operand is read and written, the second read

## val SHAPE_USE

```mach
pub val SHAPE_USE: Shape = 3
```

every operand is read

## val SHAPE_SWAP

```mach
pub val SHAPE_SWAP: Shape = 4
```

every operand is read and written

## val SHAPE_XFER

```mach
pub val SHAPE_XFER: Shape = 5
```

the operand is a transfer target

## def MemEffect

```mach
pub def MemEffect: u8
```

how the instruction reaches memory, as bits

## val MEM_NONE

```mach
pub val MEM_NONE: MemEffect = 0
```

## val MEM_OPERAND

```mach
pub val MEM_OPERAND: MemEffect = 1
```

through an r/m operand, read or written as that position's role says

## val MEM_ADDR

```mach
pub val MEM_ADDR: MemEffect = 2
```

a memory operand read as an address only, never accessed

## val MEM_STACK

```mach
pub val MEM_STACK: MemEffect = 4
```

through rsp with no explicit address

## val MEM_STRING

```mach
pub val MEM_STRING: MemEffect = 8
```

through rsi and rdi with no explicit address

## val MEM_BASE

```mach
pub val MEM_BASE: MemEffect = 16
```

the register operand becomes a segment base that later fs- or gs-relative
operands address through, so its value is an address

## rec OpDesc

```mach
pub rec OpDesc;
```

the description of one opcode: its spelling, its form and operand shape,
the flags it reads and writes with its latency class, and how it reaches
memory. the encoder, the printer, the effect walk, the inline-asm scan and
the range predicates all read this row and nothing else describes an opcode

## val OPCODE_COUNT

```mach
pub val OPCODE_COUNT: u32 = 255
```

the opcodes the catalog holds, one past the highest x64.Opcode

## fun describe

```mach
pub fun describe(op: u16) OpDesc;
```

the row of an opcode; the inline-asm block marker and anything outside the
catalog describe as the marker's row, FORM_NONE with an unknown class

