# mach.lang.be.linker.archive

## fun validate_link_inputs

```mach
pub fun validate_link_inputs(images: *of.ObjectImage, count: u32) err[fail.Fail];
```

## fun read_objects

```mach
pub fun read_objects(s: *session.Session, tgt: *target.Target, obj_paths: **u8,
obj_count: u32, seed: *of.ObjectImage, seed_count: u32,
required: intern.StrId,
module_count: *u32) res[*of.ObjectImage, fail.Fail];
```

## fun free_modules

```mach
pub fun free_modules(alloc: *A.Allocator, modules: *of.ObjectImage, module_count: u32);
```

