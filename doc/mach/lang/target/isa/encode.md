# mach.lang.target.isa.encode

## val ASM_NOTE_INST

```mach
pub val ASM_NOTE_INST:  u8 = 0
```

## val ASM_NOTE_FUNC

```mach
pub val ASM_NOTE_FUNC:  u8 = 1
```

## val ASM_NOTE_BLOCK

```mach
pub val ASM_NOTE_BLOCK: u8 = 2
```

## val ASM_NOTE_BYTES

```mach
pub val ASM_NOTE_BYTES: u8 = 3
```

## val ASM_NOTE_BARRIER

```mach
pub val ASM_NOTE_BARRIER: u8 = 4
```

a declassify barrier: no bytes, the MirInstr names the register it downgrades

## val ASM_NOTE_LABEL

```mach
pub val ASM_NOTE_LABEL: u8 = 5
```

a numbered local label's definition in an asm block: no bytes, id is its number

## val ASM_NOTE_BYTES_WIDTH

```mach
pub val ASM_NOTE_BYTES_WIDTH: usize = 4
```

## rec AsmNote

```mach
pub rec AsmNote;
```

## val NOTE_NO_LANDING

```mach
pub val NOTE_NO_LANDING: u32 = 0xFFFFFFFF
```

## fun note_blank

```mach
pub fun note_blank() AsmNote;
```

## rec NoteSeeds

```mach
pub rec NoteSeeds;
```

the secrecy seeds a notification carries, per operand of the MirInstr it
encodes: the origin vreg each register or slot operand was rewritten from
and that vreg's declared secrecy. per operand, not per register: one
register can carry a dying secret source and a born public destination in
the same instruction, so a set keyed by register cannot say which

## fun note_seeds

```mach
pub fun note_seeds(f: *lang_mir.MirFunction, n: *AsmNote, out: *NoteSeeds);
```

every seed the walk needs at one notification. a note without a MirInstr
(prologue, epilogue, relaxation) carries no seed. operands past
NOTE_SEED_MAX are not described; no MIR instruction has that many.

## rec TextStream

```mach
pub rec TextStream;
```

one output text section's bytes. stream 0 is the module's default text and
stream n its text section n, so a record's stream is its section

## rec EncoderOutput

```mach
pub rec EncoderOutput;
```

## rec PendingReloc

```mach
pub rec PendingReloc;
```

## rec SymbolMark

```mach
pub rec SymbolMark;
```

## val MARK_NO_FUNCTION

```mach
pub val MARK_NO_FUNCTION: u32 = 0xFFFFFFFF
```

## val CONST_F32

```mach
pub val CONST_F32: u8 = 1
```

## val CONST_F64

```mach
pub val CONST_F64: u8 = 2
```

## val CONST_VEC

```mach
pub val CONST_VEC: u8 = 3
```

## rec ConstEntry

```mach
pub rec ConstEntry;
```

## rec ByteBuf

```mach
pub rec ByteBuf;
```

the bytes an encoder emits and what it records beside them: the listing they
reach and the first refusal of the emission

## rec AsmSink

```mach
pub rec AsmSink;
```

## fun sink_init

```mach
pub fun sink_init(out: *io_writer.Writer, interner: *intern.Interner) AsmSink;
```

## fun note_push

```mach
pub fun note_push(buf: *ByteBuf, n: *AsmNote) err[fail.Fail];
```

## fun note_barrier

```mach
pub fun note_barrier(buf: *ByteBuf, mi: *lang_mir.MirInstr) err[fail.Fail];
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

## fun note_set_landing

```mach
pub fun note_set_landing(buf: *ByteBuf, first: u32, pos: u32, target: u32);
```

the instruction note from `first` on whose bytes hold `pos` lands on the
text offset `target`

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
pub fun render_bytes_text(out: *io_writer.Writer, bytes: *u8, n: usize) err[fail.Fail];
```

## fun note_bytes

```mach
pub fun note_bytes(st: *EncodeState, bytes: *u8, n: usize, start: usize) err[fail.Fail];
```

a raw-byte directive's note, holding the directive's first bytes for an
encoder that renders its listing from the notes once the function is final

## fun render_bytes_directive

```mach
pub fun render_bytes_directive(st: *EncodeState, bytes: *u8, n: usize, start: usize) err[fail.Fail];
```

a raw-byte directive's note, rendered at once for an encoder whose listing
streams as it encodes

## fun sink_claims

```mach
pub fun sink_claims(buf: *ByteBuf) u32;
```

## fun sink_unaccounted

```mach
pub fun sink_unaccounted(buf: *ByteBuf) usize;
```

## fun sink_set_mir

```mach
pub fun sink_set_mir(buf: *ByteBuf, mi: *lang_mir.MirInstr);
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

## fun word_read

```mach
pub fun word_read(buf: *ByteBuf, pos: usize) u32;
```

the little-endian 32-bit word already emitted at pos

## fun word_write

```mach
pub fun word_write(buf: *ByteBuf, pos: usize, word: u32);
```

overwrites the 32-bit word at pos, little-endian

## rec BranchFixup

```mach
pub rec BranchFixup;
```

## rec RelocPairSite

```mach
pub rec RelocPairSite;
```

## val STACK_PROBE_INTERVAL

```mach
pub val STACK_PROBE_INTERVAL: u64 = 4096
```

the step a probed frame allocation touches the stack at: the smallest page any
target has, so no guard a target keeps is narrower than it

## rec EncodeState

```mach
pub rec EncodeState;
```

## rec EncodeHooks

```mach
pub rec EncodeHooks;
```

## fun hooks_blank

```mach
pub fun hooks_blank() EncodeHooks;
```

## fun slot_base_reg

```mach
pub fun slot_base_reg(f: *lang_mir.MirFunction, sp: i32, fp: i32) i32;
```

the register a frame slot is addressed from, as the encoder numbers its
stack pointer and frame pointer

## val REGION_PROLOGUE

```mach
pub val REGION_PROLOGUE: u32 = 0xFFFFFFFF
```

the byte ranges an encoder emits outside any one instruction, as sink_check
names them

## val REGION_RELAX

```mach
pub val REGION_RELAX:    u32 = 0xFFFFFFFE
```

## fun sink_check

```mach
pub fun sink_check(st: *EncodeState, isa: str, region: u32) err[fail.Fail];
```

every byte emitted so far reached the listing through a notification. region
is the opcode whose encoding is checked, or a REGION_ value

## fun instr_encode

```mach
pub fun instr_encode(st: *EncodeState, f: *lang_mir.MirFunction, fn_base: u32, mi: *lang_mir.MirInstr,
hooks: *EncodeHooks) err[fail.Fail];
```

one MIR instruction through the encoder: its variable bindings, the
instructions no encoder emits bytes for, the refusal of a phi that survived
allocation, then the encoder's own hook, the declassify barrier and the
listing's accounting of what it emitted

## fun entry_align

```mach
pub fun entry_align(model: *target_model.Machine, explicit: u32) u32;
```

a function's entry alignment: its own `#[align]` or the target's rule,
whichever is larger

## fun pad_entry

```mach
pub fun pad_entry(st: *EncodeState, align: u32) err[fail.Fail];
```

fills the text up to the next multiple of `align` with the target's fill
byte. the padding is no instruction, so it reaches the sink as a claimed
extent and an alignment directive, never as a note an instruction walk reads

## fun classify_refs

```mach
pub fun classify_refs(mi: *lang_mir.MirInstr, target_block: *u32, has_block: *bool,
sym: *intern.StrId, has_sym: *bool, sym_addend: *i64);
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
pub fun push_reloc(st: *EncodeState, offset: u32, kind: target_of.RelocKind, sym: intern.StrId, addend: i64) err[fail.Fail];
```

## fun push_inst_reloc

```mach
pub fun push_inst_reloc(st: *EncodeState, offset: u32, kind: target_of.RelocKind, sym: intern.StrId, addend: i64,
inst_end: u8) err[fail.Fail];
```

a relocation whose field starts inst_end bytes before its instruction ends

## fun bind_reloc_pair

```mach
pub fun bind_reloc_pair(st: *EncodeState, low_index: u32, high_index: u32) err[fail.Fail];
```

## fun bind_reloc_pair_site

```mach
pub fun bind_reloc_pair_site(st: *EncodeState, high_kind: target_of.RelocKind,
sym: intern.StrId, reloc_index: u32) err[fail.Fail];
```

## fun reloc_pair_site

```mach
pub fun reloc_pair_site(st: *EncodeState, high_kind: target_of.RelocKind,
sym: intern.StrId) res[*RelocPairSite, fail.Fail];
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
pub fun set_fallthrough(st: *EncodeState, f: *lang_mir.MirFunction, bi: u32);
```

## fun branch_falls_through

```mach
pub fun branch_falls_through(st: *EncodeState, target_block: u32) bool;
```

## fun cbr_inverts

```mach
pub fun cbr_inverts(st: *EncodeState, then_block: u32, else_block: u32) bool;
```

a conditional branch whose then arm is the next block and whose else arm is
not jumps to the else arm on the inverse condition and falls into the then arm

## fun push_row

```mach
pub fun push_row(st: *EncodeState, text_offset: u32, loc: lang_source.Location) err[fail.Fail];
```

## fun close_cmp_varlocs

```mach
pub fun close_cmp_varlocs(st: *EncodeState, instr_start: u32, instr_end: u32);
```

## fun begin_function_locations

```mach
pub fun begin_function_locations(st: *EncodeState);
```

a function's locations start closed and with no def outstanding

## fun emit_var_bindings

```mach
pub fun emit_var_bindings(st: *EncodeState, fn: *lang_mir.MirFunction, mi: *lang_mir.MirInstr) err[fail.Fail];
```

the bindings of an instruction open at its start, and the ones it publishes
at its end open at the next instruction's start, the same offset, once the
locations its def clobbered are ended

## fun state_blank

```mach
pub fun state_blank(st: *EncodeState, alloc: *A.Allocator, model: *target_model.Machine);
```

## fun state_dnit

```mach
pub fun state_dnit(st: *EncodeState);
```

## fun consts_free

```mach
pub fun consts_free(alloc: *A.Allocator, c: *ConstEntry, count: u32, cap: u32);
```

## fun scratch_release

```mach
pub fun scratch_release(st: *EncodeState);
```

## fun streams_open

```mach
pub fun streams_open(st: *EncodeState, names: *intern.StrId, count: u32) err[fail.Fail];
```

one stream per output text section: the module's default text, then each of
`names` in order. the default text holds the buffer first

## fun stream_select

```mach
pub fun stream_select(st: *EncodeState, index: u32) err[fail.Fail];
```

hands the buffer to stream `index`, parking the one that held it

## fun stream_align

```mach
pub fun stream_align(st: *EncodeState, align: u32);
```

the current stream's alignment covers an entry aligned to `align`

## fun streams_unaccounted

```mach
pub fun streams_unaccounted(st: *EncodeState) usize;
```

the bytes no instruction notification claimed, across every stream

## fun streams_close

```mach
pub fun streams_close(st: *EncodeState) res[*TextStream, fail.Fail];
```

the streams as the encoder hands them off, each holding exactly its bytes;
the buffer and the parked streams are left empty

## fun tables_fit

```mach
pub fun tables_fit(st: *EncodeState) err[A.Error];
```

give each table the encoder hands off exactly its count

## fun patch_branches

```mach
pub fun patch_branches(st: *EncodeState, hooks: *EncodeHooks) err[fail.Fail];
```

patches the branches of the function just encoded, whose blocks all lie in
the current stream, then forgets its blocks for the next function

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
pub fun opcode_failure(st: *EncodeState, f: *lang_mir.MirFunction, architecture: str, opcode: u32) err[fail.Fail];
```

an opcode no encoder arm names is a member the selector produced outside the
native catalog: internal, named with the function it was found in

## fun asm_located_message

```mach
pub fun asm_located_message(st: *EncodeState, loc: lang_source.Location, msg: str) res[str, fail.Fail];
```

`msg` prefixed with the file, line and column of `loc`, interned; `msg` as
it is where the state carries no source map or `loc` names no place

