[mach](../README.md) / [compile](README.md) / **backend**

# mach.compile.backend

the back half a build hands its lowered modules to: the target mirl readies once for the
build, on which compile/backend/compile has mirl compile each unit to mink's object in memory,
mink laying an object out in the target's format or reading one back, and mink linking objects
into an image. nothing here starts a process or writes a file between the stages

<sub>source: [`src/compile/backend.mach`](../../../src/compile/backend.mach) · imported by 11 modules</sub>

<sub>`assembled` is [`mach.target.assembled`](../target/assembled.md) · `lower_context` is [`mach.lower.context`](../lower/context.md) · `mink_target` is `mink.catalog.target` · `registry` is `mink.format.registry` · `mirl_compile` is `mirl.target.compile` · `extent` is [`mach.type.query.extent`](../type/query/extent.md) · `target_facts` is [`mach.target.facts`](../target/facts.md) · `A` is `std.allocator` · `resolved` is [`mach.target.resolved`](../target/resolved.md) · `fail` is [`mach.base.fail`](../base/fail.md) · `diagnostic` is [`mach.base.diagnostic`](../base/diagnostic.md) · `output` is `mink.catalog.output` · `mink_arch` is `mink.catalog.arch` · `mink_subsystem` is `mink.catalog.subsystem` · `base_source` is [`mach.base.source`](../base/source.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Back`](#back) | `rec` | the back end a build opens once for its target, which sema and codegen both read |
| [`Laying`](#laying-1) | `rec` | how the target's link names the output section each input section is laid into, by mink's own layout rules, so a layout the artifact states is held to the sections the link lays out |
| [`Plan`](#plan) | `rec` | the layout an artifact states its image, which mink lays out as a linker script would |
| [`Image`](#image) | `rec` | what the build states of the image it links, beyond its kind and format |
| [`Stated`](#stated) | `rec` | where the build stated each option of the link it hands mink, a location of the source map its diagnostics read, so mink's refusal of an option points there |
| [`LinkRead`](#linkread) | `rec` | what a link read beyond the objects and the files it was handed by path |
| [`Linked`](#linked) | `rec` | a linked image and what its link read |

| On `Back` | |
| :--- | :--- |
| [`none`](#none) | a back half that compiles nothing, for a project with no target yet |
| [`init`](#init) | the back half of the resolved target `t` |
| [`merges`](#merges) | whether the objects the back half writes hold merged definitions (mink.model.merged), the copies of one a link keeps one of: false for a format masc writes none in |
| [`unlisted`](#unlisted) | whether an image of format `f` exports only what its inputs' export directives name, so a definition states no visibility; false for a format mink links no image of the name of the mirl family that compiles the build when it writes no listing of its objects, none when it writes one |
| [`dnit`](#dnit) | |
| [`machine`](#machine) | the machine lowering builds for, refused for a cell no owner assembles yet |
| [`object_bytes`](#object_bytes) | the bytes of `o` laid out in the target's object format, given back with sink.dnit |
| [`read_object`](#read_object) | the object the bytes of the file `file` hold, which owns the bytes, or mink's refusal to read one |
| [`object_bytes_with`](#object_bytes_with) | the bytes of `o` with the section `name` holding the `len` bytes at `data` added, laid out in the target's format; `o` is left as it was |
| [`stack_object`](#stack_object) | an object holding the image's stack, `size` zero-filled bytes in STACK_SECTION, kept by the link whether referenced or not, so a stated stack is always laid out |
| [`laying`](#laying) | the laying of the target's link, none for a format whose link lays out no sections, as a SPIR-V module's gives each id its index, and refused for a format mink links no image of |
| [`archive_variant`](#archive_variant) | the variant mink writes a static library of the target's objects in, as its registry states it of the target's format |
| [`c_prefix`](#c_prefix) | the prefix the target's format gives a C symbol's name on its architecture, empty for none, as mink's registry states it |
| [`describes_claims`](#describes_claims) | whether the target's link makes a shared system library from the claim that names it, with no file, as a two-level namespace format does |
| [`link`](#link) | the image of `kind` in the target's format mink links from the `count` requests at `requests`, a library named by name found by mink's search, named `output` where its format records a name, its unreferenced sections left out with `gc`, laid out as `image` states, with what the link read |

| On `Laying` | |
| :--- | :--- |
| [`laying_dnit`](#laying_dnit) | |
| [`laid_name`](#laid_name) | the name of the output section the link lays section `i` of `o` into, valid until the laying names another |

| On `LinkRead` | |
| :--- | :--- |
| [`link_read_init`](#link_read_init) · [`link_read_dnit`](#link_read_dnit) | |

| Functions | |
| :--- | :--- |
| [`refused_by_mink`](#refused_by_mink) | mink's failure `f` as mach's, under `lead`: what mink does not do is the input's to change, an allocation it was refused is the machine's, and every other cause is the compiler's |
| [`lays_stack`](#lays_stack) | whether an image of `t` holds its main stack as a section of its own: an ELF or flat image no loader maps, so no loader gives it a stack and no header records one |
| [`refers`](#refers) | whether `o` refers to the symbol `name` and leaves its definition to another object |
| [`section_bytes`](#section_bytes) | the bytes of the first section of `o` named `name`, nil and zero for none |
| [`section_names`](#section_names) | the names of the sections of `o` its producer named, borrowed from the object |
| [`section_count`](#section_count) | how many sections `o` holds |
| [`symbols_in`](#symbols_in) | the names of the symbols `o` places in its sections named `name`, borrowed from the object |
| [`object_dnit`](#object_dnit) | |
| [`symbol_count`](#symbol_count) | how many symbols `o` holds |
| [`exported`](#exported) | whether symbol `i` of `o` is one the object defines and links outside it |
| [`weakened`](#weakened) | a copy of `o` in which its definition of `name`, when it has a global one, yields to another as a weak definition; none when `o` defines no such symbol |
| [`symbol_name`](#symbol_name) | the name of symbol `i` of `o` |

| Constants | |
| :--- | :--- |
| [`OBJECT_EXT`](#constants) | the extension mach names the object files of its work directory by, whatever the format: mink reads an object by its bytes, never its name |
| [`RECORD_SECTION`](#constants) | the section an object carries mach's cache record in |
| [`STACK_SECTION`](#constants) | the section an image no loader maps holds its main stack in, whose top std's entry reads through mink's bound name `__run_stop_.stack` |
| [`STACK_ALIGN`](#constants) | the alignment the laid stack keeps its base and size at, the strictest any target's psABI asks of its stack pointer |

---

## Back

```mach
pub rec Back {
    assembled: *assembled.Assembled;
    machine:   lower_context.Machine;
    target:    mink_target.Target;
    formats:   *registry.Registry;
    prepared:  *mirl_compile.Prepared;
    laying:    *extent.Laying;
    facts:     target_facts.Facts;
}
```

the back end a build opens once for its target, which sema and codegen both read

| Field | Type | |
| :--- | :--- | :--- |
| `assembled` | `*assembled.Assembled` | the target mirl assembled for the resolved target, nil before one is opened |
| `machine` | `lower_context.Machine` | what lowering builds its units for, borrowing `assembled` |
| `target` | `mink_target.Target` | the target mink resolved, which a link is for |
| `formats` | `*registry.Registry` | mink's format registry |
| `prepared` | `*mirl_compile.Prepared` | the target mirl made ready for the build, once, which every module's compile reads; on the heap, since it stays put until it is closed. nil before one is opened |
| `laying` | `*extent.Laying` | what sema lays the build's types out by, over `machine`'s target. nil before one is opened |
| `facts` | `target_facts.Facts` | what a program reads of the target while it compiles |

### none

```mach
pub fun none() Back
```

a back half that compiles nothing, for a project with no target yet

### init

```mach
pub fun init(a: *A.Allocator, t: *resolved.Target) res[Back, fail.Fail]
```

the back half of the resolved target `t`

### merges

```mach
pub fun merges(b: *Back) bool
```

whether the objects the back half writes hold merged definitions (mink.model.merged), the
copies of one a link keeps one of: false for a format masc writes none in

### unlisted

```mach
pub fun unlisted(b: *Back) opt[str]
```

whether an image of format `f` exports only what its inputs' export directives name, so a
definition states no visibility; false for a format mink links no image of
the name of the mirl family that compiles the build when it writes no listing of its objects, none when it writes one

### dnit

```mach
pub fun dnit(a: *A.Allocator, b: *Back)
```

### machine

```mach
pub fun machine(b: *Back) res[*lower_context.Machine, fail.Fail]
```

the machine lowering builds for, refused for a cell no owner assembles yet

### object_bytes

```mach
pub fun object_bytes(b: *Back, a: *A.Allocator, o: *Object) res[Sink, fail.Fail]
```

the bytes of `o` laid out in the target's object format, given back with sink.dnit

### read_object

```mach
pub fun read_object(b: *Back, a: *A.Allocator, file: str, bytes: Vector[u8]) res[Object, fail.Fail]
```

the object the bytes of the file `file` hold, which owns the bytes, or mink's refusal to read one

### object_bytes_with

```mach
pub fun object_bytes_with(b: *Back, a: *A.Allocator, o: *Object, name: str, data: *u8, len: usize) res[Sink, fail.Fail]
```

the bytes of `o` with the section `name` holding the `len` bytes at `data` added, laid out in
the target's format; `o` is left as it was

### stack_object

```mach
pub fun stack_object(b: *Back, a: *A.Allocator, size: u64) res[Object, fail.Fail]
```

an object holding the image's stack, `size` zero-filled bytes in STACK_SECTION, kept by the
link whether referenced or not, so a stated stack is always laid out

### laying

```mach
pub fun laying(b: *Back, a: *A.Allocator) res[opt[Laying], fail.Fail]
```

the laying of the target's link, none for a format whose link lays out no sections, as a
SPIR-V module's gives each id its index, and refused for a format mink links no image of

### archive_variant

```mach
pub fun archive_variant(b: *Back, a: *A.Allocator) res[*Variant, fail.Fail]
```

the variant mink writes a static library of the target's objects in, as its
registry states it of the target's format

### c_prefix

```mach
pub fun c_prefix(b: *Back, a: *A.Allocator) res[str, fail.Fail]
```

the prefix the target's format gives a C symbol's name on its architecture, empty
for none, as mink's registry states it

### describes_claims

```mach
pub fun describes_claims(b: *Back, a: *A.Allocator) res[bool, fail.Fail]
```

whether the target's link makes a shared system library from the claim that names it,
with no file, as a two-level namespace format does

### link

```mach
pub fun link(b: *Back, a: *A.Allocator, diags: *diagnostic.DiagnosticStore, kind: *output.Kind, requests: *Request, count: usize, output_path: str, gc: bool, image: *Image) res[Linked, fail.Fail]
```

the image of `kind` in the target's format mink links from the `count` requests at `requests`, a library
named by name found by mink's search, named `output` where its format records a name, its
unreferenced sections left out with `gc`, laid out as `image` states, with what the link read.
mink's notes go to `diags`: a refused link's are notes of the refusal, which is reported there,
and a finished link's are warnings

## Laying

```mach
pub rec Laying {
    rules: *Rules;
    arch:  *mink_arch.Arch;
    names: Interner;
}
```

how the target's link names the output section each input section is laid into, by mink's
own layout rules, so a layout the artifact states is held to the sections the link lays out

| Field | Type | |
| :--- | :--- | :--- |
| `rules` | `*Rules` | the layout rules of the format the target's images link as |
| `arch` | `*mink_arch.Arch` | the architecture the link is for |
| `names` | `Interner` | the output section names, owned |

### laying_dnit

```mach
pub fun laying_dnit(l: *Laying)
```

### laid_name

```mach
pub fun laid_name(a: *A.Allocator, l: *Laying, o: *Object, i: usize) res[View, fail.Fail]
```

the name of the output section the link lays section `i` of `o` into, valid until the
laying names another

## Plan

```mach
pub rec Plan {
    keep:          *str;
    keep_count:    usize;
    sections:      *str;
    section_count: usize;
    regions:       *Area;
    region_count:  usize;
    runs:          *Within;
    run_count:     usize;
    loads:         *Within;
    load_count:    usize;
    offsets:       *Offset;
    offset_count:  usize;
    size:          opt[u64];
}
```

the layout an artifact states its image, which mink lays out as a linker script would

| Field | Type | |
| :--- | :--- | :--- |
| `sections` | `*str` | the output sections placed ahead of the rest, in order |
| `regions` | `*Area` | the memory regions |
| `runs` | `*Within` | the output sections that run in a region |
| `loads` | `*Within` | the output sections whose bytes load in a region other than the one they run in |
| `offsets` | `*Offset` | the output sections placed at an offset, one per `#[at]` offset |
| `size` | `opt[u64]` | the most bytes the image loads from its base, none for no ceiling |

## Image

```mach
pub rec Image {
    base:          opt[u64];
    subsystem:     opt[*mink_subsystem.Subsystem];
    libdirs:       *str;
    libdir_count:  usize;
    stack_reserve: opt[u64];
    stack_commit:  opt[u64];
    plan:          *Plan;
    stated:        Stated;
}
```

what the build states of the image it links, beyond its kind and format

| Field | Type | |
| :--- | :--- | :--- |
| `base` | `opt[u64]` | the address the image is laid out from, none for the host's |
| `subsystem` | `opt[*mink_subsystem.Subsystem]` | the PE subsystem the image states, none for the host's |
| `libdirs` | `*str` | the directories a library named by name is searched in ahead of the system's |
| `libdir_count` | `usize` | how many |
| `stack_reserve` | `opt[u64]` | the main stack's reserve, none for the format's default |
| `stack_commit` | `opt[u64]` | the main stack's commit, none for the format's default |
| `plan` | `*Plan` | the layout the artifact states, nil where it states none |
| `stated` | `Stated` | where the build stated each option it hands mink |

## Stated

```mach
pub rec Stated {
    kind:          base_source.Location;
    target:        base_source.Location;
    subsystem:     base_source.Location;
    base:          base_source.Location;
    size:          base_source.Location;
    stack_reserve: base_source.Location;
    stack_commit:  base_source.Location;
    sections:      *base_source.Location;
    regions:       *base_source.Location;
}
```

where the build stated each option of the link it hands mink, a location of the source map
its diagnostics read, so mink's refusal of an option points there. a nil location where the
option was not written or names no place

| Field | Type | |
| :--- | :--- | :--- |
| `kind` | `base_source.Location` | the output kind |
| `target` | `base_source.Location` | the target |
| `subsystem` | `base_source.Location` | the PE subsystem |
| `base` | `base_source.Location` | the image base |
| `size` | `base_source.Location` | the size ceiling |
| `stack_reserve` | `base_source.Location` | the stack's reserve |
| `stack_commit` | `base_source.Location` | the stack's commit |
| `sections` | `*base_source.Location` | each section the plan places ahead of the rest, parallel to its sections |
| `regions` | `*base_source.Location` | each memory region, parallel to the plan's regions, which the plan's runs and loads name |

## LinkRead

```mach
pub rec LinkRead {
    files:    Vector[str];
    searched: bool;
}
```

what a link read beyond the objects and the files it was handed by path

| Field | Type | |
| :--- | :--- | :--- |
| `files` | `Vector[str]` | each other file it read and no search found, by its identity, a copy the read owns |
| `searched` | `bool` | it read a file a search found, which no digest of the file's bytes stands for |

### link_read_init

```mach
pub fun link_read_init(a: *A.Allocator) LinkRead
```

### link_read_dnit

```mach
pub fun link_read_dnit(a: *A.Allocator, r: *LinkRead)
```

## Linked

```mach
pub rec Linked {
    image: Sink;
    read:  LinkRead;
}
```

a linked image and what its link read

## Functions

### refused_by_mink

```mach
pub fun refused_by_mink(a: *A.Allocator, lead: str, f: Fail) fail.Fail
```

mink's failure `f` as mach's, under `lead`: what mink does not do is the input's to change, an
allocation it was refused is the machine's, and every other cause is the compiler's

### lays_stack

```mach
pub fun lays_stack(t: *resolved.Target) bool
```

whether an image of `t` holds its main stack as a section of its own: an ELF or flat image
no loader maps, so no loader gives it a stack and no header records one

### refers

```mach
pub fun refers(o: *Object, name: str) bool
```

whether `o` refers to the symbol `name` and leaves its definition to another object

### section_bytes

```mach
pub fun section_bytes(o: *Object, name: str, len: *usize) *u8
```

the bytes of the first section of `o` named `name`, nil and zero for none

### section_names

```mach
pub fun section_names(o: *Object, out: *Vector[View]) err[fail.Fail]
```

the names of the sections of `o` its producer named, borrowed from the object

### section_count

```mach
pub fun section_count(o: *Object) usize
```

how many sections `o` holds

### symbols_in

```mach
pub fun symbols_in(o: *Object, name: View, out: *Vector[View]) err[fail.Fail]
```

the names of the symbols `o` places in its sections named `name`, borrowed from the object

### object_dnit

```mach
pub fun object_dnit(o: *Object)
```

### symbol_count

```mach
pub fun symbol_count(o: *Object) usize
```

how many symbols `o` holds

### exported

```mach
pub fun exported(o: *Object, i: usize) bool
```

whether symbol `i` of `o` is one the object defines and links outside it

### weakened

```mach
pub fun weakened(a: *A.Allocator, o: *Object, name: str) res[opt[Object], fail.Fail]
```

a copy of `o` in which its definition of `name`, when it has a global one, yields to
another as a weak definition; none when `o` defines no such symbol

### symbol_name

```mach
pub fun symbol_name(o: *Object, i: usize) View
```

the name of symbol `i` of `o`

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `OBJECT_EXT` | `str` | `"o"` | the extension mach names the object files of its work directory by, whatever the format: mink reads an object by its bytes, never its name |
| `RECORD_SECTION` | `str` | `".mach.cache"` | the section an object carries mach's cache record in |
| `STACK_SECTION` | `str` | `".stack"` | the section an image no loader maps holds its main stack in, whose top std's entry reads through mink's bound name `__run_stop_.stack` |
| `STACK_ALIGN` | `u64` | `16` | the alignment the laid stack keeps its base and size at, the strictest any target's psABI asks of its stack pointer |

