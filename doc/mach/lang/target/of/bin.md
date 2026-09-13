# mach.lang.target.of.bin

## fun write_u16_le

```mach
pub fun write_u16_le(buf: *u8, offset: usize, value: u16);
```

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

## fun cstr_at

```mach
pub fun cstr_at(buf: *u8, buf_size: usize, offset: usize) res[str, fail.Fail];
```

## fun resolve_name

```mach
pub fun resolve_name(itn: *intern.Interner, id: intern.StrId) str;
```

## fun name_message

```mach
pub fun name_message(itn: *intern.Interner, a: *A.Allocator, prefix: str, name: str, generic: str) str;
```

## fun number_message

```mach
pub fun number_message(itn: *intern.Interner, a: *A.Allocator, prefix: str, value: u64, generic: str) str;
```

## fun reloc_counts

```mach
pub fun reloc_counts(a: *A.Allocator, img: *of.ObjectImage) res[*u32, fail.Fail];
```

