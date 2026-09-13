# mach.lang.be.codegen.encode

## def EncoderOutput

```mach
pub def EncoderOutput: encoding.EncoderOutput
```

## def LineRow

```mach
pub def LineRow:       encoding.LineRow
```

## def InlinePc

```mach
pub def InlinePc:      encoding.InlinePc
```

## def VarLoc

```mach
pub def VarLoc:        encoding.VarLoc
```

## val VARLOC_NONE

```mach
pub val VARLOC_NONE:  u8 = encoding.VARLOC_NONE
```

## val VARLOC_REG

```mach
pub val VARLOC_REG:   u8 = encoding.VARLOC_REG
```

## val VARLOC_FRAME

```mach
pub val VARLOC_FRAME: u8 = encoding.VARLOC_FRAME
```

## val VARLOC_CMP

```mach
pub val VARLOC_CMP: u8 = encoding.VARLOC_CMP
```

## val VARLOC_IMM

```mach
pub val VARLOC_IMM: u8 = encoding.VARLOC_IMM
```

## val CMPREL_EQ

```mach
pub val CMPREL_EQ: u8 = encoding.CMPREL_EQ
```

## val CMPREL_NE

```mach
pub val CMPREL_NE: u8 = encoding.CMPREL_NE
```

## val CMPREL_LT

```mach
pub val CMPREL_LT: u8 = encoding.CMPREL_LT
```

## val CMPREL_LE

```mach
pub val CMPREL_LE: u8 = encoding.CMPREL_LE
```

## def PendingReloc

```mach
pub def PendingReloc: encoding.PendingReloc
```

## def SymbolMark

```mach
pub def SymbolMark:   encoding.SymbolMark
```

## val CONST_F32

```mach
pub val CONST_F32: u8 = encoding.CONST_F32
```

## val CONST_F64

```mach
pub val CONST_F64: u8 = encoding.CONST_F64
```

## val CONST_VEC

```mach
pub val CONST_VEC: u8 = encoding.CONST_VEC
```

## def ConstEntry

```mach
pub def ConstEntry: encoding.ConstEntry
```

## fun run

```mach
pub fun run(tgt: *target.Target, m: *mir.MirModule,
asm_out: *writer.Writer) res[EncoderOutput, fail.Fail];
```

## rec ByteBuf

```mach
pub rec ByteBuf;
```

## rec AsmSink

```mach
pub rec AsmSink;
```

## def AsmNote

```mach
pub def AsmNote:   notes.AsmNote
```

the stream records live in notes.mach so the walk that consumes them can
sit beside the driver instead of inside it; the names stay reachable here

## def NoteSeeds

```mach
pub def NoteSeeds: notes.NoteSeeds
```

## val ASM_NOTE_INST

```mach
pub val ASM_NOTE_INST:    u8 = notes.ASM_NOTE_INST
```

## val ASM_NOTE_FUNC

```mach
pub val ASM_NOTE_FUNC:    u8 = notes.ASM_NOTE_FUNC
```

## val ASM_NOTE_BLOCK

```mach
pub val ASM_NOTE_BLOCK:   u8 = notes.ASM_NOTE_BLOCK
```

## val ASM_NOTE_BYTES

```mach
pub val ASM_NOTE_BYTES:   u8 = notes.ASM_NOTE_BYTES
```

## val ASM_NOTE_BARRIER

```mach
pub val ASM_NOTE_BARRIER: u8 = notes.ASM_NOTE_BARRIER
```

## val ASM_NOTE_BYTES_WIDTH

```mach
pub val ASM_NOTE_BYTES_WIDTH: usize = notes.ASM_NOTE_BYTES_WIDTH
```

## val NOTE_SEED_MAX

```mach
pub val NOTE_SEED_MAX: u32 = notes.NOTE_SEED_MAX
```

## fun sink_init

```mach
pub fun sink_init(out: *writer.Writer, interner: *intern.Interner) AsmSink;
```

## fun note_blank

```mach
pub fun note_blank() AsmNote;
```

## fun note_push

```mach
pub fun note_push(buf: *ByteBuf, n: *AsmNote) err[fail.Fail];
```

## fun note_barrier

```mach
pub fun note_barrier(buf: *ByteBuf, mi: *mir.MirInstr) err[fail.Fail];
```

the barrier reaches the stream whether or not the move it selected to emitted
bytes (a coalesced self-copy emits none); it follows the instruction's bytes

## fun note_insert_after

```mach
pub fun note_insert_after(buf: *ByteBuf, idx: u32, n: *AsmNote, nbytes: u32) err[fail.Fail];
```

## fun note_count

```mach
pub fun note_count(buf: *ByteBuf) u32;
```

## fun note_at

```mach
pub fun note_at(buf: *ByteBuf, idx: u32) *AsmNote;
```

## fun note_index_at

```mach
pub fun note_index_at(buf: *ByteBuf, off: u32) i32;
```

## fun notes_reset

```mach
pub fun notes_reset(buf: *ByteBuf);
```

## fun note_inst_count

```mach
pub fun note_inst_count(buf: *ByteBuf) u32;
```

## fun notes_free

```mach
pub fun notes_free(buf: *ByteBuf);
```

## fun sink_fail

```mach
pub fun sink_fail(buf: *ByteBuf, msg: str);
```

## fun sink_active

```mach
pub fun sink_active(buf: *ByteBuf) bool;
```

## fun sink_renders

```mach
pub fun sink_renders(buf: *ByteBuf) bool;
```

rendering is gated on a writer; notification and byte accounting are not

## fun sink_claim

```mach
pub fun sink_claim(buf: *ByteBuf, start: usize);
```

## fun render_bytes_text

```mach
pub fun render_bytes_text(out: *writer.Writer, bytes: *u8, n: usize) err[fail.Fail];
```

## fun render_bytes_directive

```mach
pub fun render_bytes_directive(buf: *ByteBuf, bytes: *u8, n: usize, start: usize) err[fail.Fail];
```

## fun sink_claims

```mach
pub fun sink_claims(buf: *ByteBuf) u32;
```

## fun sink_unaccounted

```mach
pub fun sink_unaccounted(buf: *ByteBuf) usize;
```

## fun note_seeds_clear

```mach
pub fun note_seeds_clear(out: *NoteSeeds);
```

## fun note_seeds

```mach
pub fun note_seeds(f: *mir.MirFunction, n: *AsmNote, out: *NoteSeeds);
```

## fun sink_set_mir

```mach
pub fun sink_set_mir(buf: *ByteBuf, mi: *mir.MirInstr);
```

## fun buf_init

```mach
pub fun buf_init(alloc: *A.Allocator) ByteBuf;
```

## fun buf_refuse

```mach
pub fun buf_refuse(buf: *ByteBuf, msg: str);
```

an encoder records that an instruction had no encoding for its operands;
the first refusal is kept and the emission is reported failed at the
function boundary, so the bytes a partial expansion left never ship

## fun buf_dnit

```mach
pub fun buf_dnit(buf: *ByteBuf);
```

## fun emit_byte

```mach
pub fun emit_byte(buf: *ByteBuf, byte: u8);
```

## fun emit_u16

```mach
pub fun emit_u16(buf: *ByteBuf, value: u16);
```

## fun emit_u32

```mach
pub fun emit_u32(buf: *ByteBuf, value: u32);
```

## fun emit_u64

```mach
pub fun emit_u64(buf: *ByteBuf, value: u64);
```

## rec BranchFixup

```mach
pub rec BranchFixup;
```

## rec RelocPairSite

```mach
pub rec RelocPairSite;
```

## rec EncodeState

```mach
pub rec EncodeState;
```

## def EncodeFunctionFn

```mach
pub def EncodeFunctionFn: fun(*EncodeState, *mir.MirFunction) err[fail.Fail]
```

## def PatchBranchFn

```mach
pub def PatchBranchFn: fun(*EncodeState, *BranchFixup, u32) err[fail.Fail]
```

## rec EncodeHooks

```mach
pub rec EncodeHooks;
```

## fun hooks_blank

```mach
pub fun hooks_blank() EncodeHooks;
```

## fun module_has_oblivious

```mach
pub fun module_has_oblivious(m: *mir.MirModule) bool;
```

## fun encode_module

```mach
pub fun encode_module(alloc: *A.Allocator, tgt: *isa.BackendTarget, m: *mir.MirModule, hooks: *EncodeHooks) res[EncoderOutput, fail.Fail];
```

## fun encode_module_asm

```mach
pub fun encode_module_asm(alloc: *A.Allocator, tgt: *isa.BackendTarget, m: *mir.MirModule,
hooks: *EncodeHooks, asm_out: *writer.Writer) res[EncoderOutput, fail.Fail];
```

## fun classify_refs

```mach
pub fun classify_refs(mi: *mir.MirInstr, target_block: *u32, has_block: *bool,
sym: *intern.StrId, has_sym: *bool, sym_addend: *i32);
```

## fun push_symbol

```mach
pub fun push_symbol(st: *EncodeState, name: intern.StrId, offset: u32, flags: u32) err[fail.Fail];
```

## fun push_frame

```mach
pub fun push_frame(st: *EncodeState, offset: u32) err[fail.Fail];
```

## fun push_frame_step

```mach
pub fun push_frame_step(st: *EncodeState, kind: u8, reg: u8, end_off: u32, value: u64) err[fail.Fail];
```

## fun push_reloc

```mach
pub fun push_reloc(st: *EncodeState, offset: u32, kind: of.RelocKind, sym: intern.StrId, addend: i32) err[fail.Fail];
```

## fun bind_reloc_pair

```mach
pub fun bind_reloc_pair(st: *EncodeState, low_index: u32, high_index: u32) err[fail.Fail];
```

## fun bind_reloc_pair_site

```mach
pub fun bind_reloc_pair_site(st: *EncodeState, high_kind: of.RelocKind,
sym: intern.StrId, reloc_index: u32) err[fail.Fail];
```

## fun reloc_pair_site

```mach
pub fun reloc_pair_site(st: *EncodeState, high_kind: of.RelocKind,
sym: intern.StrId) res[*RelocPairSite, fail.Fail];
```

## fun const_align

```mach
pub fun const_align(kind: u8, len: u32) u32;
```

## fun intern_const

```mach
pub fun intern_const(st: *EncodeState, bytes: *u8, len: u32, kind: u8) res[intern.StrId, fail.Fail];
```

## fun intern_local_label

```mach
pub fun intern_local_label(st: *EncodeState, prefix: str, ordinal: u32) res[intern.StrId, fail.Fail];
```

## fun intern_float_const

```mach
pub fun intern_float_const(st: *EncodeState, bits: u64, width: u8) res[intern.StrId, fail.Fail];
```

## fun push_fixup

```mach
pub fun push_fixup(st: *EncodeState, patch_pos: u32, next_ip: u32, target_block: u32, fn_text_base: u32) err[fail.Fail];
```

## fun push_block

```mach
pub fun push_block(st: *EncodeState, fn_text_base: u32, block_id: u32, offset: u32) err[fail.Fail];
```

## fun set_fallthrough

```mach
pub fun set_fallthrough(st: *EncodeState, f: *mir.MirFunction, bi: u32);
```

## fun branch_falls_through

```mach
pub fun branch_falls_through(st: *EncodeState, target_block: u32) bool;
```

## fun push_row

```mach
pub fun push_row(st: *EncodeState, text_offset: u32, loc: source.SrcLoc) err[fail.Fail];
```

## fun push_varloc

```mach
pub fun push_varloc(st: *EncodeState, text_offset: u32, fn: *mir.MirFunction, vreg: u32, iid: u32) err[fail.Fail];
```

## fun push_cmp_varloc

```mach
pub fun push_cmp_varloc(st: *EncodeState, text_offset: u32, fn: *mir.MirFunction, bind: *mir.MirDbgBinding) err[fail.Fail];
```

## fun close_cmp_varlocs

```mach
pub fun close_cmp_varlocs(st: *EncodeState, instr_start: u32, instr_end: u32);
```

## fun emit_var_bindings

```mach
pub fun emit_var_bindings(st: *EncodeState, fn: *mir.MirFunction, mi: *mir.MirInstr) err[fail.Fail];
```

## fun state_blank

```mach
pub fun state_blank(st: *EncodeState, alloc: *A.Allocator, model: *isa.MachineModel);
```

## fun consts_free

```mach
pub fun consts_free(alloc: *A.Allocator, c: *ConstEntry, count: u32, cap: u32);
```

## fun block_offset

```mach
pub fun block_offset(st: *EncodeState, fn_base: u32, block_id: u32) res[u32, fail.Fail];
```

## fun insert_text

```mach
pub fun insert_text(st: *EncodeState, pos: u32, nbytes: u32) err[fail.Fail];
```

## fun push_inline_pc

```mach
pub fun push_inline_pc(st: *EncodeState, text_offset: u32, inline_site: u32) err[fail.Fail];
```

## fun opcode_failure

```mach
pub fun opcode_failure(st: *EncodeState, f: *mir.MirFunction, architecture: str, opcode: u32) err[fail.Fail];
```

an opcode no encoder arm names is a member the selector produced outside the
native catalog: internal, named with the function it was found in

## fun asm_located_message

```mach
pub fun asm_located_message(st: *EncodeState, loc: source.SrcLoc, msg: str) str;
```

