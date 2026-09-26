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

## val RECORD_SECTNAME

```mach
pub val RECORD_SECTNAME: str = "__mach_cache"
```

the object cache's record rides in `__MACH,__mach_cache`, debug-attributed
like the request list so no link loads it

## val CARRIER_SECTNAME

```mach
pub val CARRIER_SECTNAME: str = "__mach_link"
```

a codegen image's carrier rides in `__MACH,__mach_link`, debug-attributed
like the request list so no link loads it

## fun emit_object

```mach
pub fun emit_object(isa_vt: *of.ObjectTarget, img: *of.ObjectImage, destination: str) err[fail.Fail];
```

