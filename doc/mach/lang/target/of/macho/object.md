# mach.lang.target.of.macho.object

## fun validate_indirects

```mach
pub fun validate_indirects(img: *of.ObjectImage) err[fail.Fail];
```

## val EXPORTS_SEGNAME

```mach
pub val EXPORTS_SEGNAME:  str = "__MACH"
```

the request list rides in `__MACH,__mach_exports`, a debug-attributed
section mach-o has no native spelling for, named through the native path so
the dwarf segment rewrite leaves it alone

## val EXPORTS_SECTNAME

```mach
pub val EXPORTS_SECTNAME: str = "__mach_exports"
```

## fun emit_object

```mach
pub fun emit_object(isa_vt: *of.ObjectTarget, img: *of.ObjectImage, destination: str) err[fail.Fail];
```

