# mach.lang.target.of

## val OF_UNKNOWN

```mach
pub val OF_UNKNOWN: u32 = 0
```

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

## val OF_FORMAT_CATALOG_VERSION

```mach
pub val OF_FORMAT_CATALOG_VERSION: u8 = 1
```

## fun of_id_for

```mach
pub fun of_id_for(name: str) u32;
```

## fun of_name_for

```mach
pub fun of_name_for(id: u32) str;
```

## fun of_fingerprint_tag

```mach
pub fun of_fingerprint_tag(id: u32) u8;
```

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
pub fun section_kind_rejection(itn: *intern.Interner, alloc: *A.Allocator, kind: SectionKind) str;
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

## val RK_ABS16

```mach
pub val RK_ABS16:              RelocKind = 21
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

## val RK_CATALOG_COUNT

```mach
pub val RK_CATALOG_COUNT: u32 = 28
```

## rec RelocKindDesc

```mach
pub rec RelocKindDesc;
```

one row per relocation kind, indexed by the kind: its display name and, for a
difference kind, the kind the resolved difference is applied as (every other
row applies as itself). adding a kind is adding a row; a kind without a row
is outside the catalog and every lookup below refuses it

## fun reloc_kind_valid

```mach
pub fun reloc_kind_valid(kind: RelocKind) bool;
```

## fun reloc_desc

```mach
pub fun reloc_desc(kind: RelocKind) opt[*RelocKindDesc];
```

the row for a declared kind, absent for a member outside the catalog

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
pub fun reloc_kind_rejection(itn: *intern.Interner, alloc: *A.Allocator, kind: RelocKind, by: str) str;
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

## fun section_defined

```mach
pub fun section_defined(id: SectionId, count: u32) bool;
```

true when the id names a real section below `count`

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

## val SEC_FLAG_IMAGE_MASK

```mach
pub val SEC_FLAG_IMAGE_MASK: u32 = SEC_FLAG_INIT_FUNCS
```

## val NATIVE_GROUP

```mach
pub val NATIVE_GROUP:       u32 = 17
```

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

## val FRAME_STEP_CAP

```mach
pub val FRAME_STEP_CAP: u32 = 24
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

## rec PltFixup

```mach
pub rec PltFixup;
```

## rec ImportAddrFixup

```mach
pub rec ImportAddrFixup;
```

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

## rec SymtabEntry

```mach
pub rec SymtabEntry;
```

## rec ExportSym

```mach
pub rec ExportSym;
```

## def Subsystem

```mach
pub def Subsystem: u8
```

## val SUBSYSTEM_CONSOLE

```mach
pub val SUBSYSTEM_CONSOLE: Subsystem = 0
```

## val SUBSYSTEM_GUI

```mach
pub val SUBSYSTEM_GUI:     Subsystem = 1
```

## val SUBSYSTEM_CATALOG_VERSION

```mach
pub val SUBSYSTEM_CATALOG_VERSION: u8 = 1
```

## fun subsystem_from_name

```mach
pub fun subsystem_from_name(name: str) opt[Subsystem];
```

## fun subsystem_name

```mach
pub fun subsystem_name(s: Subsystem) str;
```

## fun subsystem_fingerprint_tag

```mach
pub fun subsystem_fingerprint_tag(s: Subsystem) u8;
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
pub fun image_options_default(subsystem: Subsystem) ImageOptions;
```

## fun image_options_flagged

```mach
pub fun image_options_flagged(subsystem: Subsystem, flags: u32) ImageOptions;
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
pub def RelocTraitsFn: fun(RelocKind, SectionKind, bool) res[RelocTraits, RelocError]
```

## def ApplyRelocFn

```mach
pub def ApplyRelocFn:  fun(RelocKind, *u8, u32, u32, RelocTarget, i64, u64, u64) res[bool, RelocError]
```

## def ElfRelocTypeFn

```mach
pub def ElfRelocTypeFn:        fun(RelocKind) opt[u32]
```

the ELF relocation type an instruction set gives a kind, absent when the
instruction set declares no ELF encoding for that kind (unsupported)

## def LocalGotKindFn

```mach
pub def LocalGotKindFn:        fun(RelocKind) bool
```

## def MachineFlagsFn

```mach
pub def MachineFlagsFn:        fun(u32, bool) u32
```

## def NormalizeImageFn

```mach
pub def NormalizeImageFn:      fun(*A.Allocator, *ObjectImage) err[fail.Fail]
```

## def ResolveRelocOperandFn

```mach
pub def ResolveRelocOperandFn: fun(*ObjectImage, u32) res[RelocOperand, fail.Fail]
```

## def BuildAttributesFn

```mach
pub def BuildAttributesFn: fun(*A.Allocator, u32, u32, u32, bool, *u32) res[*u8, fail.Fail]
```

build: (alloc, xlen_bits, selected extension bits, float_arg_bits, has_compressed, out_len)

## def MergeAttributesFn

```mach
pub def MergeAttributesFn: fun(*A.Allocator, *u8, u32, *u8, u32, *u32) res[*u8, fail.Fail]
```

## def ValidateAttributesFn

```mach
pub def ValidateAttributesFn: fun(*u8, u32, u32, u32, u32) err[fail.Fail]
```

validate: (bytes, len, xlen_bits, selected extension bits, object machine flags)

## rec ElfAttributes

```mach
pub rec ElfAttributes;
```

## rec ElfRelocationCapabilities

```mach
pub rec ElfRelocationCapabilities;
```

## rec RelocationCapabilities

```mach
pub rec RelocationCapabilities;
```

## rec ObjectTarget

```mach
pub rec ObjectTarget;
```

## def WriterFn

```mach
pub def WriterFn: fun(*ObjectTarget, *ObjectImage, *publication.Destination) err[fail.Fail]
```

## def ParserFn

```mach
pub def ParserFn: fun(*A.Allocator, *intern.Interner, *u8, usize, *ObjectImage) err[fail.Fail]
```

## def ExecFn

```mach
pub def ExecFn: fun(*A.Allocator, *intern.Interner, *ObjectTarget, *LoadSegment, u32, u64, u64, u64,
*ExecFunction, u32, *Section, u32, *Section, u32,
*SymtabEntry, u32,
*publication.Destination, *u8, ImageOptions) err[fail.Fail]
```

## def DynExecFn

```mach
pub def DynExecFn: fun(*A.Allocator, *intern.Interner, *ObjectTarget, *LoadSegment, u32, u64, u64, u64,
*ExecFunction, u32, *DynamicInfo, *PltFixup, u32,
*Section, u32, *Section, u32,
*SymtabEntry, u32,
*publication.Destination, *u8, ImageOptions) err[fail.Fail]
```

## def SharedFn

```mach
pub def SharedFn: fun(*A.Allocator, *intern.Interner, *ObjectTarget, *LoadSegment, u32, u64,
*ExportSym, u32, *DynamicInfo, *Section, u32,
*SymtabEntry, u32, *publication.Destination, ImageOptions) err[fail.Fail]
```

## rec ExecutableSectionLocation

```mach
pub rec ExecutableSectionLocation;
```

## def ExecutableSectionLocationFn

```mach
pub def ExecutableSectionLocationFn: fun(u64, u64, u32) ExecutableSectionLocation
```

## rec HeaderShape

```mach
pub rec HeaderShape;
```

## def HeaderSpanFn

```mach
pub def HeaderSpanFn: fun(*HeaderShape, u64) u64
```

## def ExecImageFn

```mach
pub def ExecImageFn: fun(*A.Allocator, *LoadSegment, u32, u64, *u64, *usize) res[*u8, fail.Fail]
```

## rec OfVTable

```mach
pub rec OfVTable;
```

## fun artifact_vtable

```mach
pub fun artifact_vtable(id: u32, name: str, object_ext: str,
emit_object: WriterFn) OfVTable;
```

## fun linker_vtable

```mach
pub fun linker_vtable(id: u32, name: str, object_ext: str,
emit_object: WriterFn, parse_object: ParserFn,
emit_exec: ExecFn) OfVTable;
```

## fun direct_image_vtable

```mach
pub fun direct_image_vtable(id: u32, name: str, object_ext: str,
emit_object: WriterFn, parse_object: ParserFn,
emit_exec: ExecFn, emit_exec_image: ExecImageFn) OfVTable;
```

## fun with_dynamic_exec

```mach
pub fun with_dynamic_exec(vt: *OfVTable, emit: DynExecFn);
```

## fun with_shared_output

```mach
pub fun with_shared_output(vt: *OfVTable, emit: SharedFn);
```

## def ArtifactOutputKind

```mach
pub def ArtifactOutputKind: u8
```

## val ARTIFACT_EXECUTABLE

```mach
pub val ARTIFACT_EXECUTABLE: ArtifactOutputKind = 0
```

## val ARTIFACT_STATIC_LIB

```mach
pub val ARTIFACT_STATIC_LIB: ArtifactOutputKind = 1
```

## val ARTIFACT_SHARED_LIB

```mach
pub val ARTIFACT_SHARED_LIB: ArtifactOutputKind = 2
```

## val ARTIFACT_OBJECT

```mach
pub val ARTIFACT_OBJECT:     ArtifactOutputKind = 3
```

## rec ArtifactName

```mach
pub rec ArtifactName;
```

## fun artifact_naming

```mach
pub fun artifact_naming(vt: *OfVTable, os_name: str, kind: ArtifactOutputKind) res[ArtifactName, fail.Fail];
```

## rec OfRegistry

```mach
pub rec OfRegistry;
```

## fun registry_init_with_allocator

```mach
pub fun registry_init_with_allocator(alloc: *A.Allocator) OfRegistry;
```

## fun registry_init

```mach
pub fun registry_init() OfRegistry;
```

## fun registry_dnit

```mach
pub fun registry_dnit(reg: *OfRegistry);
```

## fun registry_validate

```mach
pub fun registry_validate(reg: *OfRegistry) err[fail.Fail];
```

## fun register

```mach
pub fun register(reg: *OfRegistry, vt: *OfVTable) err[fail.Fail];
```

## fun lookup

```mach
pub fun lookup(reg: *OfRegistry, name: str) opt[*OfVTable];
```

## fun registered_count

```mach
pub fun registered_count(reg: *OfRegistry) u32;
```

## fun registered

```mach
pub fun registered(reg: *OfRegistry, idx: u32) opt[*OfVTable];
```

## val DBG_UNKNOWN

```mach
pub val DBG_UNKNOWN:  u32 = 0
```

## val DBG_DWARF

```mach
pub val DBG_DWARF:    u32 = 1
```

## val DBG_CODEVIEW

```mach
pub val DBG_CODEVIEW: u32 = 2
```

## rec DebugProduceRequest

```mach
pub rec DebugProduceRequest;
```

## def DebugProduceFn

```mach
pub def DebugProduceFn:  fun(*DebugProduceRequest) err[fail.Fail]
```

## def DebugSupportsFn

```mach
pub def DebugSupportsFn: fun(*debug_input.DebugTarget) bool
```

## rec DebugVTable

```mach
pub rec DebugVTable;
```

## rec DebugRegistry

```mach
pub rec DebugRegistry;
```

## fun debug_registry_init_with_allocator

```mach
pub fun debug_registry_init_with_allocator(alloc: *A.Allocator) DebugRegistry;
```

## fun debug_registry_init

```mach
pub fun debug_registry_init() DebugRegistry;
```

## fun debug_registry_dnit

```mach
pub fun debug_registry_dnit(reg: *DebugRegistry);
```

## fun debug_register

```mach
pub fun debug_register(reg: *DebugRegistry, vt: *DebugVTable) err[fail.Fail];
```

## fun debug_registry_validate

```mach
pub fun debug_registry_validate(reg: *DebugRegistry) err[fail.Fail];
```

## fun debug_lookup

```mach
pub fun debug_lookup(reg: *DebugRegistry, name: str) opt[*DebugVTable];
```

## fun debug_registered_count

```mach
pub fun debug_registered_count(reg: *DebugRegistry) u32;
```

## fun debug_hooks_complete

```mach
pub fun debug_hooks_complete(reg: *DebugRegistry) err[fail.Fail];
```

## fun debug_registered

```mach
pub fun debug_registered(reg: *DebugRegistry, idx: u32) opt[*DebugVTable];
```

## fun cover_isa

```mach
pub fun cover_isa(vt: *OfVTable, arch_id: u32);
```

## fun covers_isa

```mach
pub fun covers_isa(vt: *OfVTable, arch_id: u32) bool;
```

## fun abs_kind_for_pointer_width

```mach
pub fun abs_kind_for_pointer_width(w: u32) res[RelocKind, fail.Fail];
```

## fun is_pointer_abs_kind

```mach
pub fun is_pointer_abs_kind(kind: RelocKind, w: u32) bool;
```

## fun reloc_symbol_name

```mach
pub fun reloc_symbol_name(img: *ObjectImage, r: *Relocation) intern.StrId;
```

