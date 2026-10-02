# mach.lang.be.linker.archive

## fun validate_link_inputs

```mach
pub fun validate_link_inputs(images: *target_of.ObjectImage, count: u32) err[fail.Fail];
```

## fun read_objects

```mach
pub fun read_objects(s: *session.Session, tgt: *lang_target.Target, inputs: *target_of.ObjectInput,
input_count: u32, seed: *target_of.ObjectImage, seed_count: u32,
required: intern.StrId,
module_count: *u32) res[*target_of.ObjectImage, fail.Fail];
```

each input carries its origin: an object mach wrote, or the user's input,
whose relocations are theirs and whose defects are the input's

## fun free_modules

```mach
pub fun free_modules(alloc: *A.Allocator, modules: *target_of.ObjectImage, module_count: u32);
```

