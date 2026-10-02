# mach.lang.target.of.record

the object cache's record: opaque bytes an object carries for the build
that wrote it, in a section no link loads. elf and coff name it
`.mach.cache`, mach-o `__MACH,__mach_cache`. the emitter writes the section
when the image carries a record, and the parser moves its bytes back into
the image, leaving the zero-length husk the linker already skips, so an
object with a record links exactly as one without

## val SECTION_NAME

```mach
pub val SECTION_NAME: str = ".mach.cache"
```

## fun with_record

```mach
pub fun with_record(alloc: *std_allocator.Allocator, img: *target_of.ObjectImage, name: str, template: target_of.Section,
out: *target_of.ObjectImage) err[fail.Fail];
```

a view of `img` whose sections carry the record in a section named `name`
shaped by `template`; the caller releases it with `exports.release`. the
record bytes stay the image's

## fun take

```mach
pub fun take(img: *target_of.ObjectImage, index: u32) err[fail.Fail];
```

moves the record out of section `index` into the image and leaves the husk.
a husk reads as no record

## fun consume

```mach
pub fun consume(itn: *intern.Interner, img: *target_of.ObjectImage, name: str) err[fail.Fail];
```

the section named `name`, when the object has one, moved into the image

