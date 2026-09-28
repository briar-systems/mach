# mach.lang.target.of.exports

the export-request list of a relocatable object: the names a module asks the
link to export whoever defines them, one per `fwd` re-export. no format shared
by elf and mach-o has a native per-object export directive, so both carry the
list in a mach-specific section the emitter writes and the parser consumes:
`u32 version`, `u32 count`, then per name `u32 len`, the bytes, padded to 4.
coff spells the same request natively as a `.drectve /EXPORT:` of a name the
object does not define, so it does not carry this section.

## val SECTION_NAME

```mach
pub val SECTION_NAME: str = ".mach.exports"
```

## val VERSION

```mach
pub val VERSION:      u32 = 1
```

## fun encode

```mach
pub fun encode(alloc: *A.Allocator, img: *of.ObjectImage, out_bytes: **u8, out_len: *u32) err[fail.Fail];
```

the section blob for an image's request list, or a nil blob when the list is
empty. the caller owns the bytes

## fun decode

```mach
pub fun decode(alloc: *A.Allocator, itn: *intern.Interner, img: *of.ObjectImage, index: u32) err[fail.Fail];
```

reads the request list out of section `index` into the image and consumes
the section, leaving a zero-length husk the linker skips. a husk written
back out reads as no requests. the image must hold no requests yet: a parse
fills the list exactly once

## fun find

```mach
pub fun find(img: *of.ObjectImage, name: intern.StrId) opt[u32];
```

the index of the section named `name`, or none

## fun with_section

```mach
pub fun with_section(alloc: *A.Allocator, img: *of.ObjectImage, name: intern.StrId,
template: of.Section, bytes: *u8, len: u32, out: *of.ObjectImage) err[fail.Fail];
```

a view of `img` whose sections are the input's plus one carrying `bytes`:
a zero-length section already named `name` (the husk an earlier parse left)
is filled in place, otherwise the section is appended. every other array is
shared with the input, so only the sections array is freed by `release`

## fun release

```mach
pub fun release(alloc: *A.Allocator, view: *of.ObjectImage);
```

