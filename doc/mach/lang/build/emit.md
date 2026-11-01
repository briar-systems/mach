# mach.lang.build.emit

## fun emit_asm

```mach
pub fun emit_asm() err[fail.Fail];
```

assembly listings come from masc's listing of the object mirl compiled, which masc
does not offer yet (masc#484)

## fun artifact_product_name

```mach
pub fun artifact_product_name(p: *project.Project) *u8;
```

## rec LoadedImageOptions

```mach
pub rec LoadedImageOptions;
```

## fun dnit_image_options

```mach
pub fun dnit_image_options(a: *A.Allocator, loaded: *LoadedImageOptions);
```

release loaded image options; nil options own nothing

## fun load_image_options

```mach
pub fun load_image_options(p: *project.Project, out_kind: catalog_artifact.Kind,
exe_path: *u8) res[LoadedImageOptions, fail.Fail];
```

## fun emit_static_archive

```mach
pub fun emit_static_archive(p: *project.Project, destination: str, objects: **Object, inputs: *target_of.ObjectInput, out_path: **u8) err[fail.Fail];
```

## fun link_image

```mach
pub fun link_image(p: *project.Project, kind: *mink_output.Kind, flat: bool, objects: **Object, count: u32,
ext: *target_of.ObjectInput, ext_count: u32, dynlib_len: u32, destination: str, image: *LoadedImageOptions) err[fail.Fail];
```

the image of `kind` mink links in memory from the `count` objects at `objects`, in link order,
and the `ext_count` external inputs at `ext`, written to `destination`. a link against a
library by name, a flat image, a position independent image and an executable carrying
resources are refused

## fun ordered_objects

```mach
pub fun ordered_objects(p: *project.Project) res[**Object, fail.Fail];
```

the emitted modules' objects in topological order, the link's order

## fun release_ordered

```mach
pub fun release_ordered(p: *project.Project, objects: **Object);
```

## fun indexed_objects

```mach
pub fun indexed_objects(p: *project.Project) res[**Object, fail.Fail];
```

every emitted module's object at its module index, for a reader that walks modules by id

## fun release_indexed

```mach
pub fun release_indexed(p: *project.Project, objects: **Object);
```

## fun append_external_inputs

```mach
pub fun append_external_inputs(p: *project.Project, unit: *build_plan.BuildUnit,
inputs: **target_of.ObjectInput, len: *u32,
dynlibs: **target_of.DynLib, dynlib_len: *u32) err[fail.Fail];
```

the unit's static link inputs, each the user's (RELOC_INPUT), appended to
`inputs` after whatever it already holds

## fun free_dynlibs

```mach
pub fun free_dynlibs(a: *A.Allocator, dynlibs: *target_of.DynLib, dynlib_len: u32);
```

## rec ObjectWrite

```mach
pub rec ObjectWrite;
```

## fun write_object

```mach
pub fun write_object(w: *ObjectWrite, destination: str) err[fail.Fail];
```

## fun write_objects

```mach
pub fun write_objects(p: *project.Project, destinations: *str, inputs_out: **target_of.ObjectInput) err[fail.Fail];
```

every module's object reaches `obj/` through a sibling temporary, carrying its cache record,
since the object cache reads it back; a module the cache restored is already there

## fun write_test_objects

```mach
pub fun write_test_objects(p: *project.Project, destinations: *str) err[fail.Fail];
```

each test object reaches `obj/` beside its module's object, the same way

destinations: per emitted module, its test object's path, nil for a module without one

## fun free_inputs

```mach
pub fun free_inputs(a: *A.Allocator, inputs: *target_of.ObjectInput, n: u32, cap: u32);
```

release `n` object inputs and their array of `cap`; a nil array owns nothing

