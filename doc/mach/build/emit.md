[mach](../README.md) / [build](README.md) / **emit**

# mach.build.emit

writing a build's products: objects into `obj/`, the static archive and the linked image

the link's inputs are gathered here too: the build's objects in link order, the
libraries the manifest, its dependencies and the command line name, and the
resources an executable declares.

Source: [`src/build/emit.mach`](../../../src/build/emit.mach)

Uses types from [`mach.base.diagnostic`](../base/diagnostic.md), [`mach.base.fail`](../base/fail.md), [`mach.build.plan`](plan.md), [`mach.compile.backend`](../compile/backend.md), [`mach.compile.project`](../compile/project.md), [`mach.target.input`](../target/input.md).

| Type | | |
| :--- | :--- | :--- |
| [`LoadedImageOptions`](#loadedimageoptions) | `rec` | the resources an executable declares: its icon and manifest, read, with the version and names an image's version resource states |

| On `LoadedImageOptions` | |
| :--- | :--- |
| [`dnit_image_options`](#dnit_image_options) | release loaded image options; nil options own nothing |
| [`load_image_options`](#load_image_options) | the resources an executable declares, read where the target's images state a subsystem, as mink's host of it names; an image of another host carries none |
| [`link_image`](#link_image) | the image of `kind` mink links in memory from the `count` objects at `inputs`, in link order, then the `link_count` inputs at `links`, written to `destination` with the resources `image` declares, laid out as the artifact and its placed globals state, what the link tells of itself reported to `diags`, answering what the link read |

| Functions | |
| :--- | :--- |
| [`artifact_product_name`](#artifact_product_name) | the name the artifact's product goes by, its `bin_name`, empty when that is not interned |
| [`emit_static_archive`](#emit_static_archive) | the archive of the build's objects, each unit's, read back from `inputs` in their order, as mink's archive writer lays one out in the variant its registry states for the target's format, its index naming each member's exported symbols |
| [`framework_dirs`](#framework_dirs) | the directories a framework is searched in into `out`, in order and each once: each `-F` directory, then the `dir` of each framework entry of the link that states one |
| [`object_count`](#object_count) | the objects every program of the build links: each unit's |
| [`ordered_inputs`](#ordered_inputs) | the build's objects as the link's inputs, in the link's order: the units' in the topological order of their modules |
| [`ordered_inputs_free`](#ordered_inputs_free) | release the input array `ordered_inputs` returned |
| [`objects_into`](#objects_into) | the build's objects in the link's order into `into`, which holds object_count of them |
| [`input_name`](#input_name) | what the link calls the objects of module `m`: its source file, which outlives the link |
| [`test_input_name`](#test_input_name) | the name module m's test object goes to the link under, set apart from its object's |
| [`external_inputs`](#external_inputs) | the inputs the unit links beyond its objects, in link order: the manifest's link entries, those its dependencies cascade, then the command line's |
| [`object_inputs`](#object_inputs) | every unit's object as a file input: each reached `obj/` when its unit completed (cache.object_written), and a module the cache restored was there already |
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

| Field | Type | |
| :--- | :--- | :--- |
| `declared` | `bool` | whether the rest were read: the image states a subsystem and declares an icon or a manifest |
| `icon_bytes` | `Vector[u8]` | the icon file's bytes, empty for none |
| `manifest_bytes` | `Vector[u8]` | the application manifest's bytes, empty for none |
| `icon_path` | `str` | the icon file's path, owned, nil for none |
| `manifest_path` | `str` | the application manifest's path, owned, nil for none |
| `version` | `str` | the project's version, borrowed |
| `internal_name` | `str` | the artifact's `bin_name`, borrowed |
| `original_filename` | `str` | the file name of the executable, borrowed from its path |
| `product_name` | `str` | the artifact's `bin_name`, borrowed |

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

## Functions

### artifact_product_name

```mach
pub fun artifact_product_name(p: *mach.compile.project.Project) *u8
```

the name the artifact's product goes by, its `bin_name`, empty when that is not interned

| Parameter | |
| :--- | :--- |
| `p` | the project |
| **returns** | the name, held by the session's names |

### emit_static_archive

```mach
pub fun emit_static_archive(p: *mach.compile.project.Project, destination: str, inputs: *mink.format.contract.Input, out_path: **u8) err[mach.base.fail.Fail]
```

the archive of the build's objects, each unit's, read back from `inputs` in their
order, as mink's archive writer lays one out in the variant its
registry states for the target's format, its index naming each member's exported symbols

### framework_dirs

```mach
pub fun framework_dirs(p: *mach.compile.project.Project, out: *Vector[str]) err[mach.base.fail.Fail]
```

the directories a framework is searched in into `out`, in order and each once: each `-F`
directory, then the `dir` of each framework entry of the link that states one

### object_count

```mach
pub fun object_count(p: *mach.compile.project.Project) u32
```

the objects every program of the build links: each unit's

### ordered_inputs

```mach
pub fun ordered_inputs(p: *mach.compile.project.Project) res[*Given, mach.base.fail.Fail]
```

the build's objects as the link's inputs, in the link's order: the units' in the topological
order of their modules

### ordered_inputs_free

```mach
pub fun ordered_inputs_free(p: *mach.compile.project.Project, inputs: *Given)
```

release the input array `ordered_inputs` returned

| Parameter | |
| :--- | :--- |
| `p` | the project the array was made for, whose allocator owns it |
| `inputs` | the array |

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

### object_inputs

```mach
pub fun object_inputs(p: *mach.compile.project.Project, destinations: *str, inputs_out: **mink.format.contract.Input) err[mach.base.fail.Fail]
```

every unit's object as a file input: each reached `obj/` when its unit completed
(cache.object_written), and a module the cache restored was there already

### inputs_dnit

```mach
pub fun inputs_dnit(a: *std.allocator.Allocator, inputs: *mink.format.contract.Input, n: u32, cap: u32)
```

release `n` object inputs and their array of `cap`; a nil array owns nothing

