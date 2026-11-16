[mach](../README.md) / [build](README.md) / **emit**

# mach.build.emit

writing a build's products: objects into `obj/`, the static archive and the linked image

the link's inputs are gathered here too: the build's objects in link order, the
libraries the manifest, its dependencies and the command line name, and the
resources an executable declares.

Source: [`src/build/emit.mach`](../../../src/build/emit.mach)

Uses types from [`mach.base.diagnostic`](../base/diagnostic.md), [`mach.base.fail`](../base/fail.md), [`mach.build.plan`](plan.md), [`mach.compile.backend`](../compile/backend.md), [`mach.compile.project`](../compile/project.md), [`mach.host.output`](../host/output.md), [`mach.target.input`](../target/input.md).

| Type | | |
| :--- | :--- | :--- |
| [`LoadedImageOptions`](#loadedimageoptions) | `rec` | the resources an executable declares: its icon and manifest, read, with the version and names an image's version resource states |
| [`ObjectWrite`](#objectwrite) | `rec` |  |

| On `LoadedImageOptions` | |
| :--- | :--- |
| [`dnit_image_options`](#dnit_image_options) | release loaded image options; nil options own nothing |
| [`load_image_options`](#load_image_options) | the resources an executable declares, read where the target's images state a subsystem, as mink's host of it names; an image of another host carries none |
| [`link_image`](#link_image) | the image of `kind` mink links in memory from the `count` objects at `inputs`, in link order, then the `link_count` inputs at `links`, written to `destination` with the resources `image` declares, laid out as the artifact and its placed globals state, what the link tells of itself reported to `diags`, answering what the link read |

| On `ObjectWrite` | |
| :--- | :--- |
| [`write_object`](#write_object) | |

| Functions | |
| :--- | :--- |
| [`artifact_product_name`](#artifact_product_name) | |
| [`emit_static_archive`](#emit_static_archive) | the archive of the build's objects, the emitted modules', read back from `inputs` in their order, as mink's archive writer lays one out in the variant its registry states for the target's format, its index naming each member's exported symbols |
| [`object_count`](#object_count) | the objects every program of the build links: each emitted module's |
| [`ordered_inputs`](#ordered_inputs) | the build's objects as the link's inputs, in the link's order: the emitted modules' in topological order |
| [`ordered_inputs_free`](#ordered_inputs_free) | |
| [`objects_into`](#objects_into) | the build's objects in the link's order into `into`, which holds object_count of them |
| [`input_name`](#input_name) | what the link calls the objects of module `m`: its source file, which outlives the link |
| [`test_input_name`](#test_input_name) | the name module m's test object goes to the link under, set apart from its object's |
| [`external_inputs`](#external_inputs) | the inputs the unit links beyond its objects, in link order: the manifest's link entries, those its dependencies cascade, then the command line's |
| [`write_objects`](#write_objects) | every module's object reaches `obj/` through a sibling temporary, carrying its cache record, since the object cache reads it back; a module the cache restored is already there |
| [`write_test_objects`](#write_test_objects) | each test object reaches `obj/` beside its module's object, the same way |
| [`inputs_dnit`](#inputs_dnit) | release `n` object inputs and their array of `cap`; a nil array owns nothing |

---

## LoadedImageOptions

```mach
pub rec LoadedImageOptions {
    declared:          bool;
    icon_bytes:        Vector[u8];
    manifest_bytes:    Vector[u8];
    icon_path:         str;
    manifest_path:     str;
    version:           str;
    internal_name:     str;
    original_filename: str;
    product_name:      str;
}
```

the resources an executable declares: its icon and manifest, read, with the
version and names an image's version resource states

### dnit_image_options

```mach
pub fun dnit_image_options(a: *std.allocator.Allocator, loaded: *LoadedImageOptions)
```

release loaded image options; nil options own nothing

### load_image_options

```mach
pub fun load_image_options(p: *mach.compile.project.Project, out_kind: Spelled,
    exe_path: *u8) res[LoadedImageOptions, mach.base.fail.Fail]
```

the resources an executable declares, read where the target's images state a
subsystem, as mink's host of it names; an image of another host carries none

### link_image

```mach
pub fun link_image(p: *mach.compile.project.Project, diags: *mach.base.diagnostic.DiagnosticStore, kind: *mink.catalog.output.Kind, inputs: *Given, count: u32,
    links: *mach.target.input.Input, link_count: u32, destination: str, image: *LoadedImageOptions) res[mach.compile.backend.LinkRead, mach.base.fail.Fail]
```

the image of `kind` mink links in memory from the `count` objects at `inputs`, in link order,
then the `link_count` inputs at `links`, written to `destination` with the resources
`image` declares, laid out as the artifact and its placed globals state, what the link tells of
itself reported to `diags`, answering what the link read

## ObjectWrite

```mach
pub rec ObjectWrite {
    p:      *mach.compile.project.Project;
    o:      *Object;
    record: *mach.compile.project.Record;
}
```

### write_object

```mach
pub fun write_object(w: *ObjectWrite, t: *mach.host.output.Temporary) err[mach.base.fail.Fail]
```

## Functions

### artifact_product_name

```mach
pub fun artifact_product_name(p: *mach.compile.project.Project) *u8
```

### emit_static_archive

```mach
pub fun emit_static_archive(p: *mach.compile.project.Project, destination: str, inputs: *mink.format.contract.Input, out_path: **u8) err[mach.base.fail.Fail]
```

the archive of the build's objects, the emitted modules', read back from `inputs` in their
order, as mink's archive writer lays one out in the variant its
registry states for the target's format, its index naming each member's exported symbols

### object_count

```mach
pub fun object_count(p: *mach.compile.project.Project) u32
```

the objects every program of the build links: each emitted module's

### ordered_inputs

```mach
pub fun ordered_inputs(p: *mach.compile.project.Project) res[*Given, mach.base.fail.Fail]
```

the build's objects as the link's inputs, in the link's order: the emitted modules' in
topological order

### ordered_inputs_free

```mach
pub fun ordered_inputs_free(p: *mach.compile.project.Project, inputs: *Given)
```

### objects_into

```mach
pub fun objects_into(p: *mach.compile.project.Project, into: *Given) err[mach.base.fail.Fail]
```

the build's objects in the link's order into `into`, which holds object_count of them

### input_name

```mach
pub fun input_name(p: *mach.compile.project.Project, m: *mach.compile.project.ModuleEntry) str
```

what the link calls the objects of module `m`: its source file, which outlives the link

### test_input_name

```mach
pub fun test_input_name(p: *mach.compile.project.Project, m: *mach.compile.project.ModuleEntry) res[str, mach.base.fail.Fail]
```

the name module m's test object goes to the link under, set apart from its object's

### external_inputs

```mach
pub fun external_inputs(p: *mach.compile.project.Project, unit: *mach.build.plan.BuildUnit) res[Vector[mach.target.input.Input], mach.base.fail.Fail]
```

the inputs the unit links beyond its objects, in link order: the manifest's link entries,
those its dependencies cascade, then the command line's. an input named twice is linked
once, and each library registers the symbols it claims. a library the link makes from
the claims the objects carry is no input of its own

### write_objects

```mach
pub fun write_objects(p: *mach.compile.project.Project, destinations: *str, inputs_out: **mink.format.contract.Input) err[mach.base.fail.Fail]
```

every module's object reaches `obj/` through a sibling temporary, carrying its cache record,
since the object cache reads it back; a module the cache restored is already there

### write_test_objects

```mach
pub fun write_test_objects(p: *mach.compile.project.Project, destinations: *str) err[mach.base.fail.Fail]
```

each test object reaches `obj/` beside its module's object, the same way

| Parameter | |
| :--- | :--- |
| `destinations` | per emitted module, its test object's path, nil for a module without one |

### inputs_dnit

```mach
pub fun inputs_dnit(a: *std.allocator.Allocator, inputs: *mink.format.contract.Input, n: u32, cap: u32)
```

release `n` object inputs and their array of `cap`; a nil array owns nothing

