# mach.lang.build.emit

## fun artifact_product_name

```mach
pub fun artifact_product_name(p: *project.Project) *u8;
```

## rec LoadedImageOptions

```mach
pub rec LoadedImageOptions;
```

the resources an executable declares: its icon and manifest, read, with the
version and names an image's version resource states

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

the resources an executable declares, read where the target's images state a
subsystem, as mink's host of it names; an image of another host carries none

## fun emit_static_archive

```mach
pub fun emit_static_archive(p: *project.Project, destination: str, objects: **Object, inputs: *contract.Input, out_path: **u8) err[fail.Fail];
```

the archive of the emitted modules' objects, read back from `inputs`, as mink's
archive writer lays one out in the variant its registry states for the target's
format, its index naming each member's exported symbols

## fun link_image

```mach
pub fun link_image(p: *project.Project, kind: *mink_output.Kind, objects: **Object, count: u32,
links: *input.Input, link_count: u32, destination: str, image: *LoadedImageOptions) err[fail.Fail];
```

the image of `kind` mink links in memory from the `count` objects at `objects`, in link order,
then the `link_count` inputs at `links`, written to `destination` with the resources
`image` declares, laid out as the artifact and its placed globals state

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

## fun external_inputs

```mach
pub fun external_inputs(p: *project.Project, unit: *build_plan.BuildUnit) res[Vector[input.Input], fail.Fail];
```

the inputs the unit links beyond its objects, in link order: the manifest's link entries,
those its dependencies cascade, then the command line's. an input named twice is linked
once, and each library registers its provider and the symbols it claims

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
pub fun write_objects(p: *project.Project, destinations: *str, inputs_out: **contract.Input) err[fail.Fail];
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
pub fun free_inputs(a: *A.Allocator, inputs: *contract.Input, n: u32, cap: u32);
```

release `n` object inputs and their array of `cap`; a nil array owns nothing

