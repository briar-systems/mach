# mach.lang.target.of.exports

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
the section, leaving a zero-length husk the linker skips. the image must
hold no requests yet: a parse fills the list exactly once

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

