# mach.lang.target.of.bin

## fun write_u16_le

```mach
pub fun write_u16_le(buf: *u8, offset: usize, value: u16);
```

fixed-width numbers at a byte offset, laid out by std binary

## fun write_u32_le

```mach
pub fun write_u32_le(buf: *u8, offset: usize, value: u32);
```

## fun write_u64_le

```mach
pub fun write_u64_le(buf: *u8, offset: usize, value: u64);
```

## fun write_u32_be

```mach
pub fun write_u32_be(buf: *u8, offset: usize, value: u32);
```

## fun write_u64_be

```mach
pub fun write_u64_be(buf: *u8, offset: usize, value: u64);
```

## fun read_u16_le

```mach
pub fun read_u16_le(buf: *u8, offset: usize) u16;
```

## fun read_u32_le

```mach
pub fun read_u32_le(buf: *u8, offset: usize) u32;
```

## fun read_u32_be

```mach
pub fun read_u32_be(buf: *u8, offset: usize) u32;
```

## fun read_u64_le

```mach
pub fun read_u64_le(buf: *u8, offset: usize) u64;
```

## fun write_str

```mach
pub fun write_str(buf: *u8, offset: usize, s: str) usize;
```

## fun region_ok

```mach
pub fun region_ok(buf_size: usize, offset: usize, len: usize) bool;
```

## fun require_region

```mach
pub fun require_region(buf_size: usize, offset: usize, len: usize, msg: str) err[fail.Fail];
```

an input object's region that its own bytes do not hold: the object is malformed

## fun cstr_at

```mach
pub fun cstr_at(buf: *u8, buf_size: usize, offset: usize) res[str, fail.Fail];
```

## fun resolve_name

```mach
pub fun resolve_name(itn: *intern.Interner, id: intern.StrId) str;
```

the text of a name an image holds, nil for STR_NIL, an entry with no name

## fun strtab_size

```mach
pub fun strtab_size(img: *target_of.ObjectImage) usize;
```

the bytes of a symbol string table that opens with the empty name and holds
every symbol's name after it, each ended by a nul

## fun name_message

```mach
pub fun name_message(itn: *intern.Interner, a: *A.Allocator, prefix: str, name: str) res[str, fail.Fail];
```

`prefix` and `name` joined, interned when there is an interner so the text
outlives the writer's storage, else owned by `a`

## fun reloc_sym_index

```mach
pub fun reloc_sym_index(img: *target_of.ObjectImage, r: *target_of.Relocation, prefix: str, unnamed: str) res[u32, fail.Fail];
```

the symbol a relocation names, refused as `prefix` and the symbol's name, or
as `unnamed` when the symbol has no name, when that symbol did not resolve

## fun validate_reloc_symbols

```mach
pub fun validate_reloc_symbols(img: *target_of.ObjectImage, prefix: str, unnamed: str) err[fail.Fail];
```

## fun number_message

```mach
pub fun number_message(itn: *intern.Interner, a: *A.Allocator, prefix: str, value: u64) res[str, fail.Fail];
```

`prefix` and `value` joined, as name_message joins them

## fun reloc_type_rejection

```mach
pub fun reloc_type_rejection(itn: *intern.Interner, a: *A.Allocator, r_type: u64) res[str, fail.Fail];
```

why a native relocation type a format reads no kind for is refused

## fun reloc_counts

```mach
pub fun reloc_counts(a: *A.Allocator, img: *target_of.ObjectImage) res[*u32, fail.Fail];
```

