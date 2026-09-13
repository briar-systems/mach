# mach.lang.target.of.buildid

## val DIGEST_LEN

```mach
pub val DIGEST_LEN: usize = 32
```

## val UUID_LEN

```mach
pub val UUID_LEN:   usize = 16
```

## val ELF_NOTE_NAMESZ

```mach
pub val ELF_NOTE_NAMESZ:  u32   = 4
```

elf: `.note.gnu.build-id`, namesz / descsz / type / "GNU\0" / descriptor

## val ELF_NOTE_TYPE

```mach
pub val ELF_NOTE_TYPE:    u32   = 3
```

## val ELF_NOTE_HEADER

```mach
pub val ELF_NOTE_HEADER:  usize = 16
```

## val ELF_NOTE_SIZE

```mach
pub val ELF_NOTE_SIZE:    usize = 48
```

## val ELF_NOTE_ALIGN

```mach
pub val ELF_NOTE_ALIGN:   usize = 4
```

## val ELF_NOTE_SECTION

```mach
pub val ELF_NOTE_SECTION: str   = ".note.gnu.build-id"
```

## fun write_elf_note_header

```mach
pub fun write_elf_note_header(buf: *u8, off: usize);
```

## fun elf_note_header_valid

```mach
pub fun elf_note_header_valid(note: *u8) bool;
```

## fun uuid_from_digest

```mach
pub fun uuid_from_digest(digest: *u8, out: *u8);
```

the 16-byte form: the first 16 digest bytes with the rfc 4122 version 4 and
variant 1 bits forced, as ld64 and lld shape a content-derived uuid

## val PE_DEBUG_DIR_SIZE

```mach
pub val PE_DEBUG_DIR_SIZE:      usize = 28
```

pe: one IMAGE_DEBUG_DIRECTORY entry then the codeview record
"RSDS" / guid / age / empty path, both inside the `.buildid` section

## val PE_DEBUG_TYPE_CODEVIEW

```mach
pub val PE_DEBUG_TYPE_CODEVIEW: u32   = 2
```

## val PE_CODEVIEW_SIZE

```mach
pub val PE_CODEVIEW_SIZE:       usize = 25
```

## val PE_CODEVIEW_GUID

```mach
pub val PE_CODEVIEW_GUID:       usize = 4
```

## val PE_BUILDID_SIZE

```mach
pub val PE_BUILDID_SIZE:        usize = 53
```

## val PE_BUILDID_SECTION

```mach
pub val PE_BUILDID_SECTION:     str   = ".buildid"
```

## fun write_pe_debug_directory

```mach
pub fun write_pe_debug_directory(buf: *u8, off: usize, record_rva: u32, record_file_off: u32);
```

the directory entry names the record that follows it; both offsets are the
entry's own position translated into an rva and a file offset

## fun write_pe_codeview

```mach
pub fun write_pe_codeview(buf: *u8, off: usize, uuid: *u8);
```

## fun pe_codeview_valid

```mach
pub fun pe_codeview_valid(record: *u8) bool;
```

## fun digest_range

```mach
pub fun digest_range(st: *sha256.State, buf: *u8, off: usize, len: usize);
```

sha-256 over a run of image bytes

