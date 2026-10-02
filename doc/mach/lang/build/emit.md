# mach.lang.build.emit

## fun emit_asm

```mach
pub fun emit_asm(p: *driver.Project, destination: str, m: *me_ir.Module) err[outcome.Fail];
```

## fun emit_ir

```mach
pub fun emit_ir(p: *driver.Project, destination: str, m: *me_ir.Module) err[outcome.Fail];
```

## fun artifact_product_name

```mach
pub fun artifact_product_name(p: *driver.Project) *u8;
```

## rec LoadedImageOptions

```mach
pub rec LoadedImageOptions;
```

## fun dnit_image_options

```mach
pub fun dnit_image_options(a: *A.Allocator, loaded: *LoadedImageOptions);
```

## fun load_image_options

```mach
pub fun load_image_options(p: *driver.Project, out_kind: linker.LinkMode,
exe_path: *u8) res[LoadedImageOptions, outcome.Fail];
```

## fun emit_static_archive

```mach
pub fun emit_static_archive(p: *driver.Project, destination: str, images: *target_of.ObjectImage, inputs: *target_of.ObjectInput, out_path: **u8) err[outcome.Fail];
```

## fun emit_shared_library

```mach
pub fun emit_shared_library(p: *driver.Project, destination: str, inputs: *target_of.ObjectInput, len: u32,
dynlibs: *target_of.DynLib, dynlib_len: u32, out_path: **u8) err[outcome.Fail];
```

## fun link_shared_images

```mach
pub fun link_shared_images(p: *driver.Project, images: *target_of.ObjectImage,
ext: *target_of.ObjectInput, ext_count: u32,
dynlibs: *target_of.DynLib, dynlib_len: u32, destination: str, out_path: **u8) err[outcome.Fail];
```

## fun link_executable

```mach
pub fun link_executable(p: *driver.Project, inputs: *target_of.ObjectInput, len: u32,
dynlibs: *target_of.DynLib, dynlib_len: u32, destination: str,
image_options: target_of.ImageOptions) err[outcome.Fail];
```

## fun link_flat_images

```mach
pub fun link_flat_images(p: *driver.Project, images: *target_of.ObjectImage, destination: str,
image_options: target_of.ImageOptions) err[outcome.Fail];
```

## fun publish_entry_module

```mach
pub fun publish_entry_module(p: *driver.Project, images: *target_of.ObjectImage, destination: str) err[outcome.Fail];
```

write the artifact entry module's object to `destination`. on a finished-module
format that object is the complete deliverable, so no other module takes part

## fun link_mixed_images

```mach
pub fun link_mixed_images(p: *driver.Project, images: *target_of.ObjectImage,
ext: *target_of.ObjectInput, ext_count: u32,
dynlibs: *target_of.DynLib, dynlib_len: u32, destination: str,
image_options: target_of.ImageOptions) err[outcome.Fail];
```

## fun append_external_inputs

```mach
pub fun append_external_inputs(p: *driver.Project, unit: *build_plan.BuildUnit,
inputs: **target_of.ObjectInput, len: *u32,
dynlibs: **target_of.DynLib, dynlib_len: *u32) err[outcome.Fail];
```

the unit's static link inputs, each the user's (RELOC_INPUT), appended to
`inputs` after whatever it already holds

## fun free_dynlibs

```mach
pub fun free_dynlibs(a: *A.Allocator, dynlibs: *target_of.DynLib, dynlib_len: u32);
```

## fun write_objects

```mach
pub fun write_objects(p: *driver.Project, images: *target_of.ObjectImage,
destinations: *str, inputs_out: **target_of.ObjectInput) err[outcome.Fail];
```

every module's object reaches `obj/` through a sibling temporary, since the
object cache reads it back; a module the cache restored is already there

## fun write_test_objects

```mach
pub fun write_test_objects(p: *driver.Project, destinations: *str) err[outcome.Fail];
```

each test object reaches `obj/` beside its module's object, the same way

destinations: per emitted module, its test object's path, nil for a module without one

## fun free_inputs

```mach
pub fun free_inputs(a: *A.Allocator, inputs: *target_of.ObjectInput, n: u32, cap: u32);
```

