# mach.lang.target.of.macho.format

## val MH_MAGIC_64

```mach
pub val MH_MAGIC_64: u32 = 0xFEEDFACF
```

## val CPU_TYPE_X86_64

```mach
pub val CPU_TYPE_X86_64: u32 = 0x01000007
```

## val CPU_TYPE_ARM64

```mach
pub val CPU_TYPE_ARM64:  u32 = 0x0100000C
```

## val MH_OBJECT

```mach
pub val MH_OBJECT:  u32 = 1
```

## val MH_EXECUTE

```mach
pub val MH_EXECUTE: u32 = 2
```

## val MH_NOUNDEFS

```mach
pub val MH_NOUNDEFS: u32 = 0x1
```

## val MH_DYLDLINK

```mach
pub val MH_DYLDLINK: u32 = 0x4
```

## val MH_TWOLEVEL

```mach
pub val MH_TWOLEVEL: u32 = 0x80
```

## val LC_SEGMENT_64

```mach
pub val LC_SEGMENT_64:     u32 = 0x19
```

## val LC_SYMTAB

```mach
pub val LC_SYMTAB:         u32 = 0x2
```

## val LC_DYSYMTAB

```mach
pub val LC_DYSYMTAB:       u32 = 0xB
```

## val LC_UNIXTHREAD

```mach
pub val LC_UNIXTHREAD:     u32 = 0x5
```

## val LC_LOAD_DYLIB

```mach
pub val LC_LOAD_DYLIB:     u32 = 0xC
```

## val LC_LOAD_DYLINKER

```mach
pub val LC_LOAD_DYLINKER:  u32 = 0xE
```

## val LC_RPATH

```mach
pub val LC_RPATH:          u32 = 0x8000001C
```

## val LC_DYLD_INFO_ONLY

```mach
pub val LC_DYLD_INFO_ONLY: u32 = 0x80000022
```

## val LC_CODE_SIGNATURE

```mach
pub val LC_CODE_SIGNATURE: u32 = 0x1D
```

## val LC_MAIN

```mach
pub val LC_MAIN:           u32 = 0x80000028
```

## val LC_BUILD_VERSION

```mach
pub val LC_BUILD_VERSION:  u32 = 0x32
```

## val LC_UUID

```mach
pub val LC_UUID:           u32 = 0x1B
```

## val ENTRY_POINT_CMD_SIZE

```mach
pub val ENTRY_POINT_CMD_SIZE:   usize = 24
```

## val BUILD_VERSION_CMD_SIZE

```mach
pub val BUILD_VERSION_CMD_SIZE: usize = 24
```

## val UUID_CMD_SIZE

```mach
pub val UUID_CMD_SIZE:          usize = 24
```

## val PLATFORM_MACOS

```mach
pub val PLATFORM_MACOS:    u32 = 1
```

## val MACOS_MIN_VERSION

```mach
pub val MACOS_MIN_VERSION: u32 = 0x000B0000
```

## val MH_PIE

```mach
pub val MH_PIE: u32 = 0x200000
```

## val SG_NORELOC

```mach
pub val SG_NORELOC:   u32 = 0x4
```

## val SG_READ_ONLY

```mach
pub val SG_READ_ONLY: u32 = 0x10
```

## val VM_PROT_READ

```mach
pub val VM_PROT_READ:    u32 = 1
```

## val VM_PROT_WRITE

```mach
pub val VM_PROT_WRITE:   u32 = 2
```

## val VM_PROT_EXECUTE

```mach
pub val VM_PROT_EXECUTE: u32 = 4
```

## val SECTION_TYPE

```mach
pub val SECTION_TYPE:             u32 = 0x000000FF
```

## val S_ZEROFILL

```mach
pub val S_ZEROFILL:               u32 = 0x1
```

## val S_MOD_INIT_FUNC_POINTERS

```mach
pub val S_MOD_INIT_FUNC_POINTERS: u32 = 0x9
```

## val S_ATTR_NO_DEAD_STRIP

```mach
pub val S_ATTR_NO_DEAD_STRIP:     u32 = 0x10000000
```

## val S_ATTR_DEBUG

```mach
pub val S_ATTR_DEBUG:             u32 = 0x02000000
```

## val S_ATTR_SOME_INSTRUCTIONS

```mach
pub val S_ATTR_SOME_INSTRUCTIONS: u32 = 0x00000400
```

## val S_ATTR_PURE_INSTRUCTIONS

```mach
pub val S_ATTR_PURE_INSTRUCTIONS: u32 = 0x80000000
```

## val N_EXT

```mach
pub val N_EXT:  u8 = 0x01
```

## val N_TYPE

```mach
pub val N_TYPE: u8 = 0x0e
```

## val N_PEXT

```mach
pub val N_PEXT: u8 = 0x10
```

## val N_UNDF

```mach
pub val N_UNDF: u8 = 0x00
```

## val N_SECT

```mach
pub val N_SECT: u8 = 0x0e
```

## val N_ABS

```mach
pub val N_ABS:  u8 = 0x02
```

## val N_WEAK_DEF

```mach
pub val N_WEAK_DEF: u16 = 0x0080
```

## val N_WEAK_REF

```mach
pub val N_WEAK_REF: u16 = 0x0040
```

## val REFERENCE_TYPE_MASK

```mach
pub val REFERENCE_TYPE_MASK:           u16 = 0x7
```

## val REFERENCE_FLAG_UNDEFINED_LAZY

```mach
pub val REFERENCE_FLAG_UNDEFINED_LAZY: u16 = 0x1
```

## val X86_64_RELOC_UNSIGNED

```mach
pub val X86_64_RELOC_UNSIGNED:   u32 = 0
```

## val X86_64_RELOC_SIGNED

```mach
pub val X86_64_RELOC_SIGNED:     u32 = 1
```

## val X86_64_RELOC_BRANCH

```mach
pub val X86_64_RELOC_BRANCH:     u32 = 2
```

## val X86_64_RELOC_GOT_LOAD

```mach
pub val X86_64_RELOC_GOT_LOAD:   u32 = 3
```

## val X86_64_RELOC_GOT

```mach
pub val X86_64_RELOC_GOT:        u32 = 4
```

## val X86_64_RELOC_SUBTRACTOR

```mach
pub val X86_64_RELOC_SUBTRACTOR: u32 = 5
```

## val X86_64_RELOC_SIGNED_1

```mach
pub val X86_64_RELOC_SIGNED_1:   u32 = 6
```

## val X86_64_RELOC_SIGNED_2

```mach
pub val X86_64_RELOC_SIGNED_2:   u32 = 7
```

## val X86_64_RELOC_SIGNED_4

```mach
pub val X86_64_RELOC_SIGNED_4:   u32 = 8
```

## val ARM64_RELOC_UNSIGNED

```mach
pub val ARM64_RELOC_UNSIGNED:   u32 = 0
```

## val ARM64_RELOC_SUBTRACTOR

```mach
pub val ARM64_RELOC_SUBTRACTOR: u32 = 1
```

## val ARM64_RELOC_BRANCH26

```mach
pub val ARM64_RELOC_BRANCH26:   u32 = 2
```

## val ARM64_RELOC_PAGE21

```mach
pub val ARM64_RELOC_PAGE21:     u32 = 3
```

## val ARM64_RELOC_PAGEOFF12

```mach
pub val ARM64_RELOC_PAGEOFF12:  u32 = 4
```

## val ARM64_RELOC_ADDEND

```mach
pub val ARM64_RELOC_ADDEND:     u32 = 10
```

## val MACH_HEADER_64_SIZE

```mach
pub val MACH_HEADER_64_SIZE: usize = 32
```

## val SEGMENT_CMD_64_SIZE

```mach
pub val SEGMENT_CMD_64_SIZE: usize = 72
```

## val SECTION_64_SIZE

```mach
pub val SECTION_64_SIZE:     usize = 80
```

## val SYMTAB_CMD_SIZE

```mach
pub val SYMTAB_CMD_SIZE:     usize = 24
```

## val DYSYMTAB_CMD_SIZE

```mach
pub val DYSYMTAB_CMD_SIZE:   usize = 80
```

## val DYLD_INFO_CMD_SIZE

```mach
pub val DYLD_INFO_CMD_SIZE:  usize = 48
```

## val DYLINKER_CMD_BASE

```mach
pub val DYLINKER_CMD_BASE:   usize = 12
```

## val DYLIB_CMD_BASE

```mach
pub val DYLIB_CMD_BASE:      usize = 24
```

## val RPATH_CMD_BASE

```mach
pub val RPATH_CMD_BASE:      usize = 12
```

## val NLIST_64_SIZE

```mach
pub val NLIST_64_SIZE:       usize = 16
```

## val RELOC_INFO_SIZE

```mach
pub val RELOC_INFO_SIZE:     usize = 8
```

## val ANCHOR_NONE

```mach
pub val ANCHOR_NONE: u32 = 0xFFFFFFFF
```

## val SECTION_IDENTITY_SIZE

```mach
pub val SECTION_IDENTITY_SIZE: usize = 34
```

## val BIND_OPCODE_DONE

```mach
pub val BIND_OPCODE_DONE:                          u8 = 0x00
```

## val BIND_OPCODE_SET_DYLIB_ORDINAL_IMM

```mach
pub val BIND_OPCODE_SET_DYLIB_ORDINAL_IMM:         u8 = 0x10
```

## val BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB

```mach
pub val BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB:        u8 = 0x20
```

## val BIND_OPCODE_SET_DYLIB_SPECIAL_IMM

```mach
pub val BIND_OPCODE_SET_DYLIB_SPECIAL_IMM:         u8 = 0x30
```

## val BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM

```mach
pub val BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM: u8 = 0x40
```

## val BIND_OPCODE_SET_TYPE_IMM

```mach
pub val BIND_OPCODE_SET_TYPE_IMM:                  u8 = 0x50
```

## val BIND_OPCODE_SET_ADDEND_SLIB_ULEB

```mach
pub val BIND_OPCODE_SET_ADDEND_SLIB_ULEB:          u8 = 0x60
```

## val BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB

```mach
pub val BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:   u8 = 0x70
```

## val BIND_OPCODE_DO_BIND

```mach
pub val BIND_OPCODE_DO_BIND:                       u8 = 0x90
```

## val BIND_TYPE_POINTER

```mach
pub val BIND_TYPE_POINTER:              u8  = 0x1
```

## val BIND_SPECIAL_DYLIB_FLAT_LOOKUP

```mach
pub val BIND_SPECIAL_DYLIB_FLAT_LOOKUP: u8  = 0xE
```

## val DYNAMIC_LOOKUP_ORDINAL

```mach
pub val DYNAMIC_LOOKUP_ORDINAL:         u16 = 0xFE
```

## val REBASE_OPCODE_DONE

```mach
pub val REBASE_OPCODE_DONE:                        u8 = 0x00
```

## val REBASE_OPCODE_SET_TYPE_IMM

```mach
pub val REBASE_OPCODE_SET_TYPE_IMM:                u8 = 0x10
```

## val REBASE_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB

```mach
pub val REBASE_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB: u8 = 0x20
```

## val REBASE_OPCODE_DO_REBASE_IMM_TIMES

```mach
pub val REBASE_OPCODE_DO_REBASE_IMM_TIMES:         u8 = 0x50
```

## val REBASE_TYPE_POINTER

```mach
pub val REBASE_TYPE_POINTER:                       u8 = 0x1
```

## val DARWIN_DYLD_PATH

```mach
pub val DARWIN_DYLD_PATH: str = "/usr/lib/dyld"
```

## val MACHO_X86_STUB_SIZE

```mach
pub val MACHO_X86_STUB_SIZE:   usize = 6
```

## val MACHO_ARM64_STUB_SIZE

```mach
pub val MACHO_ARM64_STUB_SIZE: usize = 12
```

## val X86_THREAD_STATE64

```mach
pub val X86_THREAD_STATE64:       u32 = 4
```

## val X86_THREAD_STATE64_COUNT

```mach
pub val X86_THREAD_STATE64_COUNT: u32 = 42
```

## val ARM_THREAD_STATE64

```mach
pub val ARM_THREAD_STATE64:       u32 = 6
```

## val ARM_THREAD_STATE64_COUNT

```mach
pub val ARM_THREAD_STATE64_COUNT: u32 = 68
```

## val X86_RIP_STATE_OFFSET

```mach
pub val X86_RIP_STATE_OFFSET: usize = 128
```

## val ARM_PC_STATE_OFFSET

```mach
pub val ARM_PC_STATE_OFFSET:  usize = 256
```

## val EXEC_PAGE_SIZE

```mach
pub val EXEC_PAGE_SIZE: u64 = 4096
```

## val CSMAGIC_EMBEDDED_SIGNATURE

```mach
pub val CSMAGIC_EMBEDDED_SIGNATURE: u32 = 0xFADE0CC0
```

## val CSMAGIC_CODEDIRECTORY

```mach
pub val CSMAGIC_CODEDIRECTORY:      u32 = 0xFADE0C02
```

## val CSSLOT_CODEDIRECTORY

```mach
pub val CSSLOT_CODEDIRECTORY:       u32 = 0
```

## val CS_CODEDIRECTORY_VERSION

```mach
pub val CS_CODEDIRECTORY_VERSION: u32 = 0x00020400
```

## val CS_ADHOC_FLAG

```mach
pub val CS_ADHOC_FLAG:            u32 = 0x0002
```

## val CS_LINKER_SIGNED_FLAG

```mach
pub val CS_LINKER_SIGNED_FLAG:    u32 = 0x20000
```

## val CS_HASHTYPE_SHA256

```mach
pub val CS_HASHTYPE_SHA256:       u8  = 2
```

## val CS_HASH_SIZE_SHA256

```mach
pub val CS_HASH_SIZE_SHA256:      u8  = 32
```

## val CS_PAGE_SHIFT

```mach
pub val CS_PAGE_SHIFT:            u8  = 12
```

## val CS_EXECSEG_MAIN_BINARY

```mach
pub val CS_EXECSEG_MAIN_BINARY:   u64 = 0x1
```

## val LINKEDIT_DATA_CMD_SIZE

```mach
pub val LINKEDIT_DATA_CMD_SIZE: usize = 16
```

## val SUPERBLOB_HEADER_SIZE

```mach
pub val SUPERBLOB_HEADER_SIZE:  usize = 20
```

## val CODEDIR_HEADER_SIZE

```mach
pub val CODEDIR_HEADER_SIZE:    usize = 88
```

## val CODE_SIG_PAGE_SIZE

```mach
pub val CODE_SIG_PAGE_SIZE:     usize = 4096
```

## val CODE_SIG_ALIGN

```mach
pub val CODE_SIG_ALIGN:         usize = 16
```

## fun arch_is_arm64

```mach
pub fun arch_is_arm64(arch_id: u32) bool;
```

## fun macho_page_size

```mach
pub fun macho_page_size(arch_id: u32) u64;
```

## fun cpu_type_for

```mach
pub fun cpu_type_for(arch_id: u32) res[u32, fail.Fail];
```

## fun cpu_subtype_for

```mach
pub fun cpu_subtype_for(arch_id: u32) u32;
```

## rec MachoSectionDesc

```mach
pub rec MachoSectionDesc;
```

the Mach-O segment, section name and section flags of every section kind,
indexed by the kind. a kind without a row is outside the catalog and the
lookups below refuse it

## fun macho_section

```mach
pub fun macho_section(kind: of.SectionKind) opt[*MachoSectionDesc];
```

## fun macho_sectname

```mach
pub fun macho_sectname(kind: of.SectionKind) opt[str];
```

## fun macho_section_flags

```mach
pub fun macho_section_flags(kind: of.SectionKind) opt[u32];
```

## fun align_log2

```mach
pub fun align_log2(align: u32) u32;
```

## fun write_fixed16

```mach
pub fun write_fixed16(buf: *u8, off: usize, s: str);
```

## fun fixed16_equals

```mach
pub fun fixed16_equals(buf: *u8, off: usize, s: str) bool;
```

## fun write_dwarf_sectname

```mach
pub fun write_dwarf_sectname(buf: *u8, off: usize, name: str);
```

## fun canonical_sectname

```mach
pub fun canonical_sectname(raw: str, out: *u8) str;
```

## fun indirect_slots

```mach
pub fun indirect_slots(section: *of.Section) res[u32, fail.Fail];
```

## fun t_isa

```mach
pub fun t_isa(id: u32) of.ObjectTarget;
```

## fun t_intern

```mach
pub fun t_intern(itn: *intern.Interner, s: str) intern.StrId;
```

## fun t_find_lc

```mach
pub fun t_find_lc(buf: *u8, cmd: u32) usize;
```

## fun t_bytes_contain

```mach
pub fun t_bytes_contain(buf: *u8, start: usize, len: usize, needle: str) bool;
```

## fun t_untouched

```mach
pub fun t_untouched(alloc: *A.Allocator, path: str) bool;
```

