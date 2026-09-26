# mach.lang.target.of.record

## val SECTION_NAME

```mach
pub val SECTION_NAME: str = ".mach.cache"
```

## fun with_record

```mach
pub fun with_record(alloc: *A.Allocator, img: *of.ObjectImage, name: str, template: of.Section,
out: *of.ObjectImage) err[fail.Fail];
```

a view of `img` whose sections carry the record in a section named `name`
shaped by `template`; the caller releases it with `exports.release`. the
record bytes stay the image's

## fun take

```mach
pub fun take(img: *of.ObjectImage, index: u32) err[fail.Fail];
```

moves the record out of section `index` into the image and leaves the husk.
a husk reads as no record

## fun consume

```mach
pub fun consume(itn: *intern.Interner, img: *of.ObjectImage, name: str) err[fail.Fail];
```

the section named `name`, when the object has one, moved into the image

