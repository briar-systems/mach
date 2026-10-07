# mach.lang.target.of

## val OF_ELF

```mach
pub val OF_ELF:     u32 = 1
```

## val OF_MACHO

```mach
pub val OF_MACHO:   u32 = 2
```

## val OF_COFF

```mach
pub val OF_COFF:    u32 = 3
```

## val OF_RAW

```mach
pub val OF_RAW:     u32 = 4
```

## val OF_SPV

```mach
pub val OF_SPV:     u32 = 5
```

## val FINGERPRINT_VERSION

```mach
pub val FINGERPRINT_VERSION: u8 = 1
```

the version of the fingerprint tags formats declare; a tag is never reused

## def SectionKind

```mach
pub def SectionKind: u8
```

## val SK_TEXT

```mach
pub val SK_TEXT:      SectionKind = 0
```

## val SK_DATA

```mach
pub val SK_DATA:      SectionKind = 1
```

## val SK_RODATA

```mach
pub val SK_RODATA:    SectionKind = 2
```

## val SK_RELRO

```mach
pub val SK_RELRO:     SectionKind = 3
```

## val SK_BSS

```mach
pub val SK_BSS:       SectionKind = 4
```

## val SK_DEBUG

```mach
pub val SK_DEBUG:     SectionKind = 5
```

## val SK_EXCEPTION

```mach
pub val SK_EXCEPTION: SectionKind = 6
```

## val SK_COUNT

```mach
pub val SK_COUNT: u32 = 7
```

## rec SectionKindDesc

```mach
pub rec SectionKindDesc;
```

one row per section kind, indexed by the kind: its canonical name and the
properties every format and the linker derive their placement from. adding
a kind is adding a row; a kind without a row is outside the catalog and
every lookup below refuses it

## fun section_kind_valid

```mach
pub fun section_kind_valid(kind: SectionKind) bool;
```

## fun section_desc

```mach
pub fun section_desc(kind: SectionKind) opt[*SectionKindDesc];
```

the row for a declared kind, absent for a member outside the catalog

## fun canonical_section_name

```mach
pub fun canonical_section_name(kind: SectionKind) opt[str];
```

the canonical name of a declared kind, absent for a member outside the catalog

## fun section_kind_rejection

```mach
pub fun section_kind_rejection(itn: *intern.Interner, alloc: *A.Allocator, kind: SectionKind) res[str, fail.Fail];
```

a section kind outside the catalog reached a consumer that validated its
image at entry: internal, naming the catalog and the tag

## def RelocKind

```mach
pub def RelocKind: u8
```

## val RK_ABS64

```mach
pub val RK_ABS64:              RelocKind = 0
```

## val RK_PC32

```mach
pub val RK_PC32:               RelocKind = 1
```

## val RK_PLT32

```mach
pub val RK_PLT32:              RelocKind = 2
```

## val RK_GOTPCREL

```mach
pub val RK_GOTPCREL:           RelocKind = 3
```

## val RK_ABS32S

```mach
pub val RK_ABS32S:             RelocKind = 4
```

## val RK_ADDR32NB

```mach
pub val RK_ADDR32NB:           RelocKind = 5
```

## val RK_PAGE21

```mach
pub val RK_PAGE21:             RelocKind = 6
```

## val RK_ADD_LO12

```mach
pub val RK_ADD_LO12:           RelocKind = 7
```

## val RK_LDST8_LO12

```mach
pub val RK_LDST8_LO12:         RelocKind = 8
```

## val RK_LDST16_LO12

```mach
pub val RK_LDST16_LO12:        RelocKind = 9
```

## val RK_LDST32_LO12

```mach
pub val RK_LDST32_LO12:        RelocKind = 10
```

## val RK_LDST64_LO12

```mach
pub val RK_LDST64_LO12:        RelocKind = 11
```

## val RK_LDST128_LO12

```mach
pub val RK_LDST128_LO12:       RelocKind = 12
```

## val RK_PCREL_HI20

```mach
pub val RK_PCREL_HI20:         RelocKind = 13
```

## val RK_PCREL_LO12_I

```mach
pub val RK_PCREL_LO12_I:       RelocKind = 14
```

## val RK_PCREL_LO12_S

```mach
pub val RK_PCREL_LO12_S:       RelocKind = 15
```

## val RK_ABS32

```mach
pub val RK_ABS32:              RelocKind = 16
```

## val RK_SECREL32

```mach
pub val RK_SECREL32:           RelocKind = 17
```

## val RK_SECTION

```mach
pub val RK_SECTION:            RelocKind = 18
```

## val RK_GOT_PAGE21

```mach
pub val RK_GOT_PAGE21:         RelocKind = 19
```

## val RK_GOT_LO12

```mach
pub val RK_GOT_LO12:           RelocKind = 20
```

## val RK_GOTPCREL_NORELAX

```mach
pub val RK_GOTPCREL_NORELAX:   RelocKind = 22
```

## val RK_GOTPCREL64_NORELAX

```mach
pub val RK_GOTPCREL64_NORELAX: RelocKind = 23
```

## val RK_DIFF32

```mach
pub val RK_DIFF32: RelocKind = 24
```

## val RK_DIFF64

```mach
pub val RK_DIFF64: RelocKind = 25
```

## val RK_JUMP26

```mach
pub val RK_JUMP26: RelocKind = 26
```

## val RK_GOT_PCREL_HI20

```mach
pub val RK_GOT_PCREL_HI20: RelocKind = 27
```

## val RK_JAL20

```mach
pub val RK_JAL20: RelocKind = 28
```

the riscv jal 20-bit j-type field, +-1 MiB, even displacement

## val RK_PC64

```mach
pub val RK_PC64: RelocKind = 29
```

a 64-bit field holding the target's distance from the field

## val RK_CATALOG_COUNT

```mach
pub val RK_CATALOG_COUNT: u32 = 30
```

## fun reloc_kind_valid

```mach
pub fun reloc_kind_valid(kind: RelocKind) bool;
```

## fun is_difference

```mach
pub fun is_difference(kind: RelocKind) bool;
```

## fun difference_apply_kind

```mach
pub fun difference_apply_kind(kind: RelocKind) opt[RelocKind];
```

the kind a difference relocation is applied as once its difference is
resolved, absent for a member outside the catalog

## fun reloc_kind_name

```mach
pub fun reloc_kind_name(kind: RelocKind) opt[str];
```

the display name of a declared kind, absent for a member outside the catalog

## fun reloc_kind_rejection

```mach
pub fun reloc_kind_rejection(itn: *intern.Interner, alloc: *A.Allocator, kind: RelocKind, by: str) res[str, fail.Fail];
```

a declared kind a format or target does not honor is unsupported by `by`; a
member outside the catalog is internal. both name the catalog and the member

## val SYM_OBJ_FLAG_GLOBAL

```mach
pub val SYM_OBJ_FLAG_GLOBAL:       u32 = 0x01
```

## val SYM_OBJ_FLAG_WEAK

```mach
pub val SYM_OBJ_FLAG_WEAK:         u32 = 0x02
```

## val SYM_OBJ_FLAG_EXTERN

```mach
pub val SYM_OBJ_FLAG_EXTERN:       u32 = 0x04
```

## val SYM_OBJ_FLAG_FUNCTION

```mach
pub val SYM_OBJ_FLAG_FUNCTION:     u32 = 0x08
```

## val SYM_OBJ_FLAG_OBJECT

```mach
pub val SYM_OBJ_FLAG_OBJECT:       u32 = 0x10
```

## val SYM_OBJ_FLAG_FALLBACK

```mach
pub val SYM_OBJ_FLAG_FALLBACK:     u32 = 0x20
```

## val SYM_OBJ_FLAG_SEARCH

```mach
pub val SYM_OBJ_FLAG_SEARCH:       u32 = 0x40
```

## val SYM_OBJ_FLAG_ALIAS

```mach
pub val SYM_OBJ_FLAG_ALIAS:        u32 = 0x80
```

## val SYM_OBJ_FLAG_COMMON

```mach
pub val SYM_OBJ_FLAG_COMMON:       u32 = 0x100
```

## val SYM_OBJ_FLAG_SIZE_UNKNOWN

```mach
pub val SYM_OBJ_FLAG_SIZE_UNKNOWN: u32 = 0x200
```

## val SYM_OBJ_FLAG_HIDDEN

```mach
pub val SYM_OBJ_FLAG_HIDDEN: u32 = 0x400
```

a definition every module in the link may reference and no linked image
exports: ELF STV_HIDDEN, Mach-O N_PEXT, absent from a PE export table

## val SYM_OBJ_FLAG_RETAIN

```mach
pub val SYM_OBJ_FLAG_RETAIN: u32 = 0x800
```

a final link keeps the definition even when nothing references it: Mach-O
N_NO_DEAD_STRIP, which `__attribute__((used))` sets

## val SECTION_EXTERNAL

```mach
pub val SECTION_EXTERNAL: u32 = 0xFFFFFFFF
```

## rec SectionId

```mach
pub rec SectionId;
```

the nominal identity of a section within one object image

a record, not a `def` alias: a `def` interchanges freely with its base type,
and the point of the identity is that a section id cannot be handed where a
symbol, segment, module or placement index is meant. the sentinels
SECTION_EXTERNAL and SECTION_ABSOLUTE live inside the domain and are reached
only through the constructors below.

## fun section_id

```mach
pub fun section_id(index: u32) SectionId;
```

## fun no_section

```mach
pub fun no_section() SectionId;
```

## fun external_section

```mach
pub fun external_section() SectionId;
```

## fun absolute_section

```mach
pub fun absolute_section() SectionId;
```

## fun section_index

```mach
pub fun section_index(id: SectionId) u32;
```

## fun section_is_external

```mach
pub fun section_is_external(id: SectionId) bool;
```

## fun section_is_absolute

```mach
pub fun section_is_absolute(id: SectionId) bool;
```

## fun section_same

```mach
pub fun section_same(left: SectionId, right: SectionId) bool;
```

## val SYMBOL_NONE

```mach
pub val SYMBOL_NONE: u32 = 0xFFFFFFFF
```

## val SEG_FLAG_READ

```mach
pub val SEG_FLAG_READ:    u32 = 0x4
```

## val SEG_FLAG_WRITE

```mach
pub val SEG_FLAG_WRITE:   u32 = 0x2
```

## val SEG_FLAG_EXECUTE

```mach
pub val SEG_FLAG_EXECUTE: u32 = 0x1
```

## val SEC_FLAG_MERGEABLE

```mach
pub val SEC_FLAG_MERGEABLE:  u32 = 0x1
```

## val SEC_FLAG_COALESCE

```mach
pub val SEC_FLAG_COALESCE:   u32 = 0x4
```

## val SEC_FLAG_INIT_FUNCS

```mach
pub val SEC_FLAG_INIT_FUNCS: u32 = 0x2
```

## val SEC_FLAG_RETAIN

```mach
pub val SEC_FLAG_RETAIN: u32 = 0x8
```

a final link keeps the section even when nothing references it

## val SEC_FLAG_IMAGE_MASK

```mach
pub val SEC_FLAG_IMAGE_MASK: u32 = SEC_FLAG_INIT_FUNCS
```

## val SEC_FLAG_UNWIND_INDEX

```mach
pub val SEC_FLAG_UNWIND_INDEX:  u32 = 0x10
```

the unwind tables: the index an unwinder searches first and the frame
descriptions it leads to. on a load section, the room the linker reserved for
the format's table. on an input section, the object's own contribution to
that table, which a link building the tables takes into them: frame
descriptions (`.eh_frame`, mach-o `__eh_frame`) are carried into the frames
table, and an index (mach-o `__compact_unwind`) is read, never placed

## val SEC_FLAG_UNWIND_FRAMES

```mach
pub val SEC_FLAG_UNWIND_FRAMES: u32 = 0x20
```

## val SEC_FLAG_ASSOCIATED

```mach
pub val SEC_FLAG_ASSOCIATED: u32 = 0x40
```

the section lives and dies with the one `native.link_section` names: a coff
associative comdat member, or an elf section ordered after its owner
(SHF_LINK_ORDER). a final link keeps it exactly while it keeps that owner

## val SEC_FLAG_GROUP

```mach
pub val SEC_FLAG_GROUP: u32 = 0x80
```

the section lists sections of its object that a link keeps or drops
together: a flags word, then one word per member holding the member's index
plus one, NATIVE_GROUP_RELOC set on a member that is a section's relocations

## val NATIVE_GROUP_RELOC

```mach
pub val NATIVE_GROUP_RELOC: u32 = 0x80000000
```

## rec NativeSection

```mach
pub rec NativeSection;
```

## rec Section

```mach
pub rec Section;
```

## fun associated_owner

```mach
pub fun associated_owner(sec: *Section) opt[u32];
```

the section an associated section lives and dies with, as an index into its
object's sections; none for a section that is not associated

## fun unwind_entry_size

```mach
pub fun unwind_entry_size(sec: *Section) u64;
```

the bytes of one entry of an unwind index section, 0 when it declares none

## rec GroupMember

```mach
pub rec GroupMember;
```

a member of a section group: a section of the group's object, or that
section's relocations

## fun group_member_count

```mach
pub fun group_member_count(sec: *Section) u32;
```

the members a group section lists

## fun group_member_at

```mach
pub fun group_member_at(sec: *Section, i: u32) opt[GroupMember];
```

the group's member `i`, none for an entry that names no section

## fun group_rebase

```mach
pub fun group_rebase(sec: *Section, by: u32);
```

moves every member of a group section `by` sections along, as its object's
sections move when objects are combined

## fun validate_native_sections

```mach
pub fun validate_native_sections(image: *ObjectImage, format: u32) err[fail.Fail];
```

## val SECTION_ABSOLUTE

```mach
pub val SECTION_ABSOLUTE: u32 = 0xFFFFFFFE
```

## rec NativeSymbol

```mach
pub rec NativeSymbol;
```

## rec Symbol

```mach
pub rec Symbol;
```

## val FRAME_STEP_PUSH

```mach
pub val FRAME_STEP_PUSH:     u8 = 1
```

a frame step records what one prologue instruction did to the frame, in
effect once the code reaches `end_off` bytes into the function. a step is
isa-neutral: `reg` is the isa's own register number, which each unwind format
translates, and "the stack pointer" is its value once the step has happened.
  PUSH reg          the stack pointer drops by a pointer and reg is stored where it points
  ALLOC value       the stack pointer drops by value bytes
  SET_FP reg value  reg becomes the stack pointer plus value, and holds for the whole body
  SAVE reg value    reg is stored at the stack pointer plus value, a signed offset
  SAVE_VEC reg value  vector register reg is stored at the stack pointer plus value
  REALIGN value     the stack pointer is rounded down to a multiple of value, so its
                    distance from the caller's frame is no longer known
a save after REALIGN is addressed from the stack pointer the body runs with.
a frameless function's record has no steps: its call's frame holds throughout

## val FRAME_STEP_ALLOC

```mach
pub val FRAME_STEP_ALLOC:    u8 = 2
```

## val FRAME_STEP_SET_FP

```mach
pub val FRAME_STEP_SET_FP:   u8 = 3
```

## val FRAME_STEP_SAVE

```mach
pub val FRAME_STEP_SAVE:     u8 = 4
```

## val FRAME_STEP_SAVE_VEC

```mach
pub val FRAME_STEP_SAVE_VEC: u8 = 5
```

## val FRAME_STEP_REALIGN

```mach
pub val FRAME_STEP_REALIGN:  u8 = 6
```

## val FRAME_STEP_LAST

```mach
pub val FRAME_STEP_LAST: u8 = FRAME_STEP_REALIGN
```

## val FRAME_STEP_CAP

```mach
pub val FRAME_STEP_CAP: u32 = 32
```

the most steps a frame record holds. a prologue records one step per
callee-saved register it saves and at most FRAME_FIXED_STEPS more (the two
allocations around a realignment, the frame pointer and return address
saves, setting the frame pointer and the realignment itself), so the cap
holds every convention whose callee-saved file fits beside them, which
registration checks. the record stays fixed-size because the linker and
every object writer copy frame records by value

## val FRAME_FIXED_STEPS

```mach
pub val FRAME_FIXED_STEPS: u32 = 6
```

## rec FrameStep

```mach
pub rec FrameStep;
```

## rec FrameUnwind

```mach
pub rec FrameUnwind;
```

## rec Relocation

```mach
pub rec Relocation;
```

## def RelocOrigin

```mach
pub def RelocOrigin: u8
```

who produced a relocation: mach's codegen (the zero value), the user's
inline asm, or an input object mach did not make

## val RELOC_CODEGEN

```mach
pub val RELOC_CODEGEN: RelocOrigin = 0
```

## val RELOC_ASM

```mach
pub val RELOC_ASM:     RelocOrigin = 1
```

## val RELOC_INPUT

```mach
pub val RELOC_INPUT:   RelocOrigin = 2
```

## fun reloc_refusal

```mach
pub fun reloc_refusal(origin: RelocOrigin, k: diagnostic_kind.Kind, text: str) fail.Fail;
```

a relocation the link or the object writer refuses is the user's when their
inline asm or an input object asked for it, and a compiler defect when
codegen made it

## fun reloc_refusal_made

```mach
pub fun reloc_refusal_made(origin: RelocOrigin, k: diagnostic_kind.Kind, made: res[str, fail.Fail]) fail.Fail;
```

the refusal carrying a made text, or the failure that refused to make it

## rec ObjectInput

```mach
pub rec ObjectInput;
```

an object or archive the link reads from a path, and who produced it: an
object mach wrote (RELOC_CODEGEN) or an input mach did not make
(RELOC_INPUT). the origin travels with the path from where the list is built

## fun set_relocation_origin

```mach
pub fun set_relocation_origin(img: *ObjectImage, origin: RelocOrigin);
```

every relocation of an image read from an input takes that input's origin

## val INDIRECT_LOCAL

```mach
pub val INDIRECT_LOCAL:    u32 = 0x80000000
```

## val INDIRECT_ABSOLUTE

```mach
pub val INDIRECT_ABSOLUTE: u32 = 0x40000000
```

## rec NativeIndirect

```mach
pub rec NativeIndirect;
```

## rec ObjectImage

```mach
pub rec ObjectImage;
```

## fun object_image_init

```mach
pub fun object_image_init(alloc: *A.Allocator, itn: *intern.Interner,
name: intern.StrId) ObjectImage;
```

## fun object_image_dnit

```mach
pub fun object_image_dnit(image: *ObjectImage);
```

## fun section_install

```mach
pub fun section_install(o: *ObjectImage, s: *Section) res[u32, fail.Fail];
```

## fun symbol_add

```mach
pub fun symbol_add(o: *ObjectImage, sym: Symbol) res[u32, fail.Fail];
```

## fun export_request_add

```mach
pub fun export_request_add(o: *ObjectImage, name: intern.StrId) err[fail.Fail];
```

## fun add_frame

```mach
pub fun add_frame(o: *ObjectImage, fr: *FrameUnwind) err[fail.Fail];
```

## fun add_relocation

```mach
pub fun add_relocation(o: *ObjectImage, rel: Relocation) err[fail.Fail];
```

## fun validate_object_view

```mach
pub fun validate_object_view(image: *ObjectImage) err[fail.Fail];
```

## fun validate_owned_object

```mach
pub fun validate_owned_object(image: *ObjectImage) err[fail.Fail];
```

## fun validate_owned_object_or_dnit

```mach
pub fun validate_owned_object_or_dnit(image: *ObjectImage) err[fail.Fail];
```

## rec ImportDecl

```mach
pub rec ImportDecl;
```

## rec DynLib

```mach
pub rec DynLib;
```

## rec DynImport

```mach
pub rec DynImport;
```

## val DYNLIB_ANY

```mach
pub val DYNLIB_ANY: u32 = 0xFFFFFFFF
```

## rec BaseReloc

```mach
pub rec BaseReloc;
```

## rec DynamicInfo

```mach
pub rec DynamicInfo;
```

## fun func_import_count

```mach
pub fun func_import_count(dyn: *DynamicInfo) u32;
```

the imported functions of a dynamic part, nil for an image the loader binds
nothing in

## fun func_ordinal

```mach
pub fun func_ordinal(dyn: *DynamicInfo, import_index: u32) u32;
```

an imported function's place among the imported functions in import order,
0xFFFFFFFF for an import that is not a function

## rec SpanLocation

```mach
pub rec SpanLocation;
```

the load segment that holds a reserved table, and where in it the table starts

## fun span_locate

```mach
pub fun span_locate(a: *A.Allocator, span: TableSpan, segs: *LoadSegment, seg_count: u32, executable: bool,
table: str) res[SpanLocation, fail.Fail];
```

the load segment that holds `span` whole, refused when none does or when the
one that does is executable and `executable` says it must not be, or the
other way around

table: the table as a refusal names it, as "the PLT"

## rec TableShape

```mach
pub rec TableShape;
```

what a table a format asks the linker to reserve takes: the linker lays it out
under this section name before it gives anything an address, so the code
reaches it whatever data the image carries

## rec UnwindShape

```mach
pub rec UnwindShape;
```

the unwind tables of an image: an index an unwinder searches by address
and the frame descriptions it reaches, each empty when the format needs none

## rec ForeignUnwind

```mach
pub rec ForeignUnwind;
```

what the link's foreign objects bring to its unwind tables, measured before
layout: the bytes of their own frame descriptions, which the frames table
carries first, the entries among them that describe a function, and the
functions their unwind index describes

## rec ImportFixup

```mach
pub rec ImportFixup;
```

a site in the image the loader's binding of an import completes: a call
through the import's stub, or a load of its address

## rec LoadSection

```mach
pub rec LoadSection;
```

## rec LoadSegment

```mach
pub rec LoadSegment;
```

## rec ExecFunction

```mach
pub rec ExecFunction;
```

## rec NativeUnwind

```mach
pub rec NativeUnwind;
```

a foreign function its object's unwind index describes, at its final
address, and the personality and lsda (zero for none) its entry names

## rec UnwindPersonality

```mach
pub rec UnwindPersonality;
```

the pointer slot an unwind entry's personality is read through: one the link
filled, at `slot`, or the import GOT's slot for `import_index`. neither when
the entry names no personality

## val PERSONALITY_NO_IMPORT

```mach
pub val PERSONALITY_NO_IMPORT: u32 = 0xFFFFFFFF
```

## fun no_personality

```mach
pub fun no_personality() UnwindPersonality;
```

## fun has_personality

```mach
pub fun has_personality(p: *UnwindPersonality) bool;
```

## def SymbolType

```mach
pub def SymbolType: u8
```

what an image symbol names: a callable body, a data object, or neither. the
format writer maps it onto its own symbol type field

## val SYMT_NOTYPE

```mach
pub val SYMT_NOTYPE: SymbolType = 0
```

## val SYMT_FUNC

```mach
pub val SYMT_FUNC:   SymbolType = 1
```

## val SYMT_OBJECT

```mach
pub val SYMT_OBJECT: SymbolType = 2
```

## rec SymtabEntry

```mach
pub rec SymtabEntry;
```

## rec ExportSym

```mach
pub rec ExportSym;
```

## rec ResourceInfo

```mach
pub rec ResourceInfo;
```

## rec ImageOptions

```mach
pub rec ImageOptions;
```

## fun image_options_default

```mach
pub fun image_options_default(subsystem: catalog_subsystem.Subsystem) ImageOptions;
```

## fun effective_stack_reserve

```mach
pub fun effective_stack_reserve(tgt_of: *OfVTable, stated: u64) u64;
```

## def RelocError

```mach
pub def RelocError: u8
```

## val RELOC_OVERFLOW

```mach
pub val RELOC_OVERFLOW:            RelocError = 0
```

## val RELOC_UNSUPPORTED

```mach
pub val RELOC_UNSUPPORTED:         RelocError = 1
```

## val RELOC_INVALID_INSTRUCTION

```mach
pub val RELOC_INVALID_INSTRUCTION: RelocError = 2
```

## def RelocAddendMode

```mach
pub def RelocAddendMode: u8
```

## val RELOC_ADDEND_SYMBOL

```mach
pub val RELOC_ADDEND_SYMBOL:     RelocAddendMode = 0
```

## val RELOC_ADDEND_FIELD_BIAS

```mach
pub val RELOC_ADDEND_FIELD_BIAS: RelocAddendMode = 1
```

## val RELOC_ADDEND_IGNORED

```mach
pub val RELOC_ADDEND_IGNORED:    RelocAddendMode = 2
```

## rec RelocTraits

```mach
pub rec RelocTraits;
```

## rec RelocTarget

```mach
pub rec RelocTarget;
```

## rec RelocOperand

```mach
pub rec RelocOperand;
```

## def RelocTraitsFn

```mach
pub def RelocTraitsFn:         fun(RelocKind, SectionKind, bool) res[RelocTraits, RelocError]
```

## def ApplyRelocFn

```mach
pub def ApplyRelocFn:          fun(RelocKind, *u8, u32, u32, RelocTarget, i64, u64, u64) res[bool, RelocError]
```

## def LocalGotKindFn

```mach
pub def LocalGotKindFn:        fun(RelocKind) bool
```

## def NormalizeImageFn

```mach
pub def NormalizeImageFn:      fun(*A.Allocator, *ObjectImage) err[fail.Fail]
```

## def ResolveRelocOperandFn

```mach
pub def ResolveRelocOperandFn: fun(*ObjectImage, u32) res[RelocOperand, fail.Fail]
```

## def MachineFlagsFn

```mach
pub def MachineFlagsFn: fun(u32, bool) u32
```

(float_arg_bits, has_compressed): the processor flags word a header records

## def BuildAttributesFn

```mach
pub def BuildAttributesFn: fun(*A.Allocator, u32, u64, u32, bool, *u32) res[*u8, fail.Fail]
```

build: (alloc, xlen_bits, selected extension bits, float_arg_bits, has_compressed, out_len)

## def MergeAttributesFn

```mach
pub def MergeAttributesFn: fun(*A.Allocator, *u8, u32, *u8, u32, *u32) res[*u8, fail.Fail]
```

## def ValidateAttributesFn

```mach
pub def ValidateAttributesFn: fun(*u8, u32, u32, u32) err[fail.Fail]
```

validate: (bytes, len, xlen_bits, object machine flags), an input's section and flags
against what the target can link at all, never against the extensions it selects

## rec IsaRecord

```mach
pub rec IsaRecord;
```

what a format records of one instruction set it covers beyond numbering its
relocations: the processor flags word its headers carry and the attribute
section describing what an object needs of the processor. a hook is nil when
the format records nothing of that kind for the instruction set, and the three
attribute hooks are declared together

## def IsaRecordFn

```mach
pub def IsaRecordFn: fun(u32) *IsaRecord
```

the record a format keeps for an instruction set, nil when it keeps none

## rec BranchReach

```mach
pub rec BranchReach;
```

how far a direct branch reaches: the least and greatest displacement from the
branch to its target the instruction encodes, and the granule it counts in

## rec ThunkField

```mach
pub rec ThunkField;
```

one field of a range-extension thunk, resolved against the thunk's destination
as a relocation of `kind` at `offset` into the thunk would be

## rec BranchThunk

```mach
pub rec BranchThunk;
```

a range-extension thunk (a veneer): the code the linker places within reach of
a branch whose target lies beyond it. once each field is resolved against the
destination the bytes transfer there, touching only what the ABI lets a call
veneer clobber

## def BranchReachFn

```mach
pub def BranchReachFn: fun(RelocKind) opt[BranchReach]
```

the reach of a relocation kind that is a direct branch a thunk can extend, none
for every other kind

## def BranchThunkFn

```mach
pub def BranchThunkFn: fun() BranchThunk
```

## rec RelocationCapabilities

```mach
pub rec RelocationCapabilities;
```

## rec ObjectTarget

```mach
pub rec ObjectTarget;
```

## def ObjectEmitFn

```mach
pub def ObjectEmitFn: fun(*ObjectTarget, *ObjectImage, *of_destination.Destination) err[fail.Fail]
```

## rec LinkedImage

```mach
pub rec LinkedImage;
```

what the linker hands an image writer: the laid-out image and everything the
format records about it, the same record for every product and format

product: an executable or a shared library
dynamic: what the loader binds, nil for an image it binds nothing in
exports: what a shared library offers its users
name: the product's name, which a format may record in the image
library_name: the file name a loader finds a shared library by, empty for
              an executable

## def ImageEmitFn

```mach
pub def ImageEmitFn: fun(*LinkedImage, *of_destination.Destination) err[fail.Fail]
```

writes a linked image's files into the destination

## rec ExecutableSectionLocation

```mach
pub rec ExecutableSectionLocation;
```

## rec HeaderShape

```mach
pub rec HeaderShape;
```

## def ObjectMatchesFn

```mach
pub def ObjectMatchesFn: fun(*u8, usize, u32) bool
```

whether `bytes` are a relocatable object of the format for the instruction set

## def PathNamesFn

```mach
pub def PathNamesFn: fun(*u8) bool
```

whether a path names a file of the kind

## def SharedCandidateFn

```mach
pub def SharedCandidateFn: fun(*u8, *u8, u32, *u8, usize) bool
```

writes the file a bare library name is searched as in a directory, `index`
from 0 in search order, into the buffer; false past the last

## def SharedIdentityFn

```mach
pub def SharedIdentityFn: fun(*u8, usize, u32, *u8, *u8, usize) bool
```

whether `bytes` read from `path` are a loadable library for the instruction
set, its loader name written to the buffer when they are

## def AbsentShared

```mach
pub def AbsentShared: u8
```

how a path to a shared library that does not exist is linked

## val ABSENT_REFUSED

```mach
pub val ABSENT_REFUSED: AbsentShared = 0
```

refused as a link input that cannot be found

## val ABSENT_BY_NAME

```mach
pub val ABSENT_BY_NAME: AbsentShared = 1
```

linked by its file name, which the loader searches for

## val ABSENT_BY_PATH

```mach
pub val ABSENT_BY_PATH: AbsentShared = 2
```

linked by the path as written, which the loader opens

## rec SharedNaming

```mach
pub rec SharedNaming;
```

how a format's shared libraries are named, found and identified

## rec InputNaming

```mach
pub rec InputNaming;
```

how a link token names one of the format's inputs and how the bytes of one
are recognized. an archive is a static input of every format that names one

library_prefix: the prefix a library file carries, tried before the bare name
static_suffixes: the suffixes, without their dot, a bare name is searched
                 with for a static input, in search order
folds_case: file names compare without case
object_matches: nil when the format reads no relocatable object
shared: nil when the format takes no shared library

## rec SelectorStubReloc

```mach
pub rec SelectorStubReloc;
```

a relocation one selector stub carries, at `offset` from the stub's start;
against the stub's selector reference or the send function

## rec SelectorStubShape

```mach
pub rec SelectorStubShape;
```

the objective-c selector stubs a format's linker makes for one instruction
set: a call of `<send_symbol>$<selector>` reaches a stub that loads the
selector's reference and jumps to the send function

code: the `size` bytes every stub starts from, its relocated fields zero
relocs: the relocations of one stub

## def SelectorStubsFn

```mach
pub def SelectorStubsFn: fun(u32) *SelectorStubShape
```

the selector stubs of the instruction set, nil when the format makes none for it

## fun path_write

```mach
pub fun path_write(buf: *u8, cap: usize, dir: *u8, parts: *str, count: u32) bool;
```

writes `dir` and the concatenated `parts` as one path into `buf`, a separator
between them when `dir` is not empty and does not end in one; false when it
does not fit

## fun path_has_suffix

```mach
pub fun path_has_suffix(path: *u8, suffix: str, folds_case: bool) bool;
```

whether `path` ends in `suffix`, compared without case when `folds_case`

## fun digit_text

```mach
pub fun digit_text(v: u32) str;
```

the decimal digit `v` as text, empty past 9

## rec OfVTable

```mach
pub rec OfVTable;
```

## rec ArtifactName

```mach
pub rec ArtifactName;
```

## rec SystemNaming

```mach
pub rec SystemNaming;
```

how an operating system names the files it runs and links: a static library
and an executable, each suffix with its dot or empty

## fun artifact_naming

```mach
pub fun artifact_naming(vt: *OfVTable, system: *SystemNaming, kind: catalog_artifact.Kind) res[ArtifactName, fail.Fail];
```

the name an artifact of `kind` takes from its format and its system

## fun object_ext

```mach
pub fun object_ext(vt: *OfVTable) str;
```

the object suffix without its dot, as the build names the object files of
its object directory

## fun validate

```mach
pub fun validate(a: *A.Allocator, vt: *OfVTable) err[fail.Fail];
```

why a descriptor is malformed, read once when a registry adds it

a: formats the refusal
vt: the descriptor

## fun name_of

```mach
pub fun name_of(vt: *OfVTable) str;
```

## fun id_of

```mach
pub fun id_of(vt: *OfVTable) u32;
```

## val DBG_UNKNOWN

```mach
pub val DBG_UNKNOWN:  u32 = 0
```

## val DBG_DWARF

```mach
pub val DBG_DWARF:    u32 = 1
```

## val DBG_SPIRV

```mach
pub val DBG_SPIRV:    u32 = 3
```

## def DebugShape

```mach
pub def DebugShape: u8
```

how a debug model reaches its format

## val DEBUG_SHAPE_SECTIONS

```mach
pub val DEBUG_SHAPE_SECTIONS: DebugShape = 0
```

appends its own sections to a finished object image from address-keyed rows, through produce

## val DEBUG_SHAPE_MODULE

```mach
pub val DEBUG_SHAPE_MODULE: DebugShape = 1
```

written by a whole-module emitter as it builds the module, so it declares no producer

## rec DebugProduceRequest

```mach
pub rec DebugProduceRequest;
```

## rec DebugVTable

```mach
pub rec DebugVTable;
```

## fun debug_shape_for

```mach
pub fun debug_shape_for(vt: *OfVTable) DebugShape;
```

a format whose object is the finished artifact takes its debug model from the emitter that
builds it, and a format of linkable objects takes one that appends sections

## fun debug_validate

```mach
pub fun debug_validate(a: *A.Allocator, vt: *DebugVTable) err[fail.Fail];
```

why a debug descriptor is malformed, read once when a registry adds it

a: formats the refusal
vt: the descriptor

## fun debug_name_of

```mach
pub fun debug_name_of(vt: *DebugVTable) str;
```

## fun debug_id_of

```mach
pub fun debug_id_of(vt: *DebugVTable) u32;
```

## fun covers_isa

```mach
pub fun covers_isa(vt: *OfVTable, arch_id: u32) bool;
```

## fun isa_record_for

```mach
pub fun isa_record_for(vt: *OfVTable, arch_id: u32) *IsaRecord;
```

the record `vt` keeps for the instruction set `arch_id`, nil when it keeps none

## fun declares_machine_flags

```mach
pub fun declares_machine_flags(vt: *OfVTable, arch_id: u32) bool;
```

## fun machine_flags

```mach
pub fun machine_flags(vt: *OfVTable, arch_id: u32, float_arg_bits: u32, has_compressed: bool) u32;
```

the processor flags word `vt` records for `arch_id` under an abi passing floats
in `float_arg_bits`-wide registers; 0 when it records none

## fun declares_attributes

```mach
pub fun declares_attributes(vt: *OfVTable, arch_id: u32) bool;
```

## fun attributes_build

```mach
pub fun attributes_build(vt: *OfVTable, arch_id: u32, alloc: *A.Allocator, xlen_bits: u32, extensions: u64,
float_arg_bits: u32, has_compressed: bool, out_len: *u32) res[*u8, fail.Fail];
```

the attribute section body an object for `arch_id` carries, nil when the
format carries none

## fun attributes_validate

```mach
pub fun attributes_validate(vt: *OfVTable, arch_id: u32, xlen_bits: u32,
bytes: *u8, len: u32, flags: u32) err[fail.Fail];
```

## fun attributes_merge

```mach
pub fun attributes_merge(vt: *OfVTable, arch_id: u32, alloc: *A.Allocator, acc: *u8, acc_len: u32,
add: *u8, add_len: u32, out_len: *u32) res[*u8, fail.Fail];
```

## fun abs_kind_for_pointer_width

```mach
pub fun abs_kind_for_pointer_width(w: u32) res[RelocKind, fail.Fail];
```

## fun addend_fits_field

```mach
pub fun addend_fits_field(addend: i64, width: usize, pc_relative: bool) bool;
```

an addend a format stores in the relocated field itself fits that field of
`width` bytes: signed for a pc-relative field, signed or unsigned for an
absolute one. a writer refuses one that does not rather than keep its low
bits

## fun is_pointer_abs_kind

```mach
pub fun is_pointer_abs_kind(kind: RelocKind, w: u32) bool;
```

## fun is_pcrel_address_kind

```mach
pub fun is_pcrel_address_kind(kind: RelocKind) bool;
```

a pc-relative materialization of a symbol's address in two parts, a page or
high part and a low part, as aarch64 and riscv64 take it (x86-64 takes it
whole with RK_PC32). against a function import each part resolves to the
import's call stub, as a call does

## fun reloc_symbol_name

```mach
pub fun reloc_symbol_name(img: *ObjectImage, r: *Relocation) intern.StrId;
```

## rec DeferredReloc

```mach
pub rec DeferredReloc;
```

## rec DeferredRelocs

```mach
pub rec DeferredRelocs;
```

## fun deferred_init

```mach
pub fun deferred_init(a: *A.Allocator) DeferredRelocs;
```

## fun deferred_dnit

```mach
pub fun deferred_dnit(d: *DeferredRelocs);
```

## fun defer_relocation

```mach
pub fun defer_relocation(d: *DeferredRelocs, rec: DeferredReloc, caps: *RelocationCapabilities,
section_kind: SectionKind, codegen_image: bool) err[fail.Fail];
```

queues a relocation the target's seam can encode in a section of
`section_kind`, to bind once the image's symbols are known

## fun flush_deferred

```mach
pub fun flush_deferred(o: *ObjectImage, d: *DeferredRelocs) err[fail.Fail];
```

## fun rehome

```mach
pub fun rehome(dst_alloc: *A.Allocator, dst_interner: *intern.Interner,
src: *ObjectImage, remap: intern.Remap) res[ObjectImage, fail.Fail];
```

