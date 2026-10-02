# mach.lang.target.of.macho.parse

## fun macho_kind_from

```mach
pub fun macho_kind_from(flags: u32, is_text: bool, is_data_const: bool, is_rodata: bool) target_of.SectionKind;
```

## fun parse_object

```mach
pub fun parse_object(alloc: *std_allocator.Allocator, itn: *intern.Interner, buf: *u8, buf_size: usize, out: *target_of.ObjectImage) err[fail.Fail];
```

