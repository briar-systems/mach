# mach.lang.target.of.carrier

## val SECTION_NAME

```mach
pub val SECTION_NAME: str = ".mach.link"
```

the section elf and coff carry it in; mach-o spells it `__MACH,__mach_link`

## val TABLE_SECTIONS

```mach
pub val TABLE_SECTIONS: u32 = 0x1
```

the tables a carrier may hold, one bit each

## val TABLE_SYMBOLS

```mach
pub val TABLE_SYMBOLS:  u32 = 0x2
```

## val TABLE_RELOCS

```mach
pub val TABLE_RELOCS:   u32 = 0x4
```

## val TABLE_FRAMES

```mach
pub val TABLE_FRAMES:   u32 = 0x8
```

## rec Host

```mach
pub rec Host;
```

where a described section lies in the image the parser reads back

## fun encode

```mach
pub fun encode(alloc: *A.Allocator, img: *of.ObjectImage, described: u32, hosts: *Host,
tables: u32, out_bytes: **u8, out_len: *u32) err[fail.Fail];
```

the carrier of img's first `described` sections, holding `tables`; `hosts`
places each described section in the parsed image, nil when each is itself.
the caller owns the bytes

## fun apply

```mach
pub fun apply(img: *of.ObjectImage, bytes: *u8, len: usize) err[fail.Fail];
```

applies a carrier to the image its object parsed into, which becomes a
codegen image

## fun take

```mach
pub fun take(img: *of.ObjectImage, index: u32) err[fail.Fail];
```

applies the carrier in section `index` and leaves its zero-length husk, which
reads as no carrier

## fun consume

```mach
pub fun consume(itn: *intern.Interner, img: *of.ObjectImage, name: str) err[fail.Fail];
```

applies the carrier the section named `name` holds, when the object has one

## fun with_carrier

```mach
pub fun with_carrier(alloc: *A.Allocator, img: *of.ObjectImage, name: str, template: of.Section,
bytes: *u8, len: u32, out: *of.ObjectImage) err[fail.Fail];
```

a view of img carrying the carrier bytes in a section named `name` shaped by
`template`; the caller releases it with `exports.release`

