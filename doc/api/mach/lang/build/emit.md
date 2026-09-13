# mach.lang.build.emit

## fun ensure_parents

```mach
pub fun ensure_parents(a: *A.Allocator, p: str) err[outcome.Fail];
```

## fun emit_asm

```mach
pub fun emit_asm(p: *driver.Project, destination: *publication.Destination, m: *ir.Module) err[outcome.Fail];
```

## fun emit_ir

```mach
pub fun emit_ir(p: *driver.Project, destination: *publication.Destination, m: *ir.Module) err[outcome.Fail];
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
pub fun emit_static_archive(p: *driver.Project, destination: *publication.Destination, images: *of.ObjectImage, paths: **u8, out_path: **u8) err[outcome.Fail];
```

## fun emit_shared_library

```mach
pub fun emit_shared_library(p: *driver.Project, destination: *publication.Destination, paths: **u8, len: u32,
dynlibs: *of.DynLib, dynlib_len: u32, out_path: **u8) err[outcome.Fail];
```

## fun link_executable

```mach
pub fun link_executable(p: *driver.Project, paths: **u8, len: u32,
dynlibs: *of.DynLib, dynlib_len: u32, destination: *publication.Destination,
image_options: of.ImageOptions) err[outcome.Fail];
```

## fun link_flat_images

```mach
pub fun link_flat_images(p: *driver.Project, images: *of.ObjectImage, destination: *publication.Destination,
image_options: of.ImageOptions) err[outcome.Fail];
```

## fun link_mixed_images

```mach
pub fun link_mixed_images(p: *driver.Project, images: *of.ObjectImage,
ext_paths: **u8, ext_count: u32,
dynlibs: *of.DynLib, dynlib_len: u32, destination: *publication.Destination,
image_options: of.ImageOptions) err[outcome.Fail];
```

## fun append_external_inputs

```mach
pub fun append_external_inputs(p: *driver.Project, unit: *plan.BuildUnit,
paths: ***u8, len: *u32,
dynlibs: **of.DynLib, dynlib_len: *u32) err[outcome.Fail];
```

## fun free_dynlibs

```mach
pub fun free_dynlibs(a: *A.Allocator, dynlibs: *of.DynLib, dynlib_len: u32);
```

## fun write_objects

```mach
pub fun write_objects(p: *driver.Project, images: *of.ObjectImage,
destinations: **publication.Destination, paths_out: ***u8) err[outcome.Fail];
```

## fun mirror_fqn

```mach
pub fun mirror_fqn(a: *A.Allocator, base: *u8, fqn: str, suffix: str) res[str, outcome.Fail];
```

## fun free_paths

```mach
pub fun free_paths(a: *A.Allocator, paths: **u8, n: u32, cap: u32);
```

