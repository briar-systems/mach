# mach.lang.driver.backend

the back half a build hands its lowered modules to: mirl compiles a unit to mink's object
in memory, mink lays an object out in the target's format or reads one back, and mink links
objects into an image. nothing here starts a process or writes a file between the stages

## val OBJECT_EXT

```mach
pub val OBJECT_EXT: str = "o"
```

the extension mach names the object files of its work directory by, whatever the
format: mink reads an object by its bytes, never its name

## rec Back

```mach
pub rec Back;
```

the target a build compiles for and the formats it reads and writes

assembled: the target mirl assembled for the resolved target, nil before one is opened
machine: what lowering builds its units for, borrowing `assembled`
format: the object format the target writes
formats: mink's format registry
prepared: the target mirl made ready for the build, once, which every module's compile reads;
           on the heap, since it stays put until it is closed. nil before one is opened

## fun none

```mach
pub fun none() Back;
```

a back half that compiles nothing, for a project with no target yet

## fun open

```mach
pub fun open(a: *A.Allocator, t: *resolved.Target) res[Back, fail.Fail];
```

the back half of the resolved target `t`

## fun close

```mach
pub fun close(a: *A.Allocator, b: *Back);
```

## fun machine

```mach
pub fun machine(b: *Back) res[*lower_context.Machine, fail.Fail];
```

the machine lowering builds for, refused for a cell no owner assembles yet

## fun refused_by_mink

```mach
pub fun refused_by_mink(a: *A.Allocator, lead: str, f: Fail) fail.Fail;
```

mink's failure `f` as mach's, under `lead`

## fun compile

```mach
pub fun compile(b: *Back, a: *A.Allocator, m: *Module, level: u8, checked: bool, relax: bool, floats: relaxation.Relaxations,
listing: opt[*std_writer.Writer], stats: opt[*mirl_stats.Stats], name: str) res[Object, fail.Fail];
```

the mirl module `m` compiled to mink's object at mirl's level `level`, its schedule's own checks
run with `checked`, the linker let relax its code with `relax` and the float relaxations `floats`
permitted, masc's listing of the object written to `listing` when there is one, and its stages
timed and counted into `stats` when there is one. the module is the schedule's to rewrite, so
it is spent here

## fun object_bytes

```mach
pub fun object_bytes(b: *Back, a: *A.Allocator, o: *Object) res[Sink, fail.Fail];
```

the bytes of `o` laid out in the target's object format, given back with sink.release

## fun read_object

```mach
pub fun read_object(b: *Back, a: *A.Allocator, file: str, data: *u8, len: usize) opt[Object];
```

the object the bytes of the file `file` hold, none when mink reads no object from them

## val RECORD_SECTION

```mach
pub val RECORD_SECTION: str = ".mach.cache"
```

the section an object carries mach's cache record in. mink states no flag a link drops a
section by (mink#538), so the section is unreferenced and the link's collection leaves it out

## fun object_bytes_with

```mach
pub fun object_bytes_with(b: *Back, a: *A.Allocator, o: *Object, name: str, data: *u8, len: usize) res[Sink, fail.Fail];
```

the bytes of `o` with the section `name` holding the `len` bytes at `data` added, laid out in
the target's format; `o` is left as it was

## fun section_bytes

```mach
pub fun section_bytes(o: *Object, name: str, len: *usize) *u8;
```

the bytes of the first section of `o` named `name`, nil and zero for none

## fun release_object

```mach
pub fun release_object(o: *Object);
```

## fun symbol_count

```mach
pub fun symbol_count(o: *Object) usize;
```

how many symbols `o` holds

## fun exported

```mach
pub fun exported(o: *Object, i: usize) bool;
```

whether symbol `i` of `o` is one the object defines and links outside it

## fun weakened

```mach
pub fun weakened(a: *A.Allocator, o: *Object, name: str) res[opt[Object], fail.Fail];
```

a copy of `o` in which its definition of `name`, when it has a global one, yields to
another as a weak definition; none when `o` defines no such symbol

## fun archive_variant

```mach
pub fun archive_variant(b: *Back, a: *A.Allocator) res[*Variant, fail.Fail];
```

the variant mink writes a static library of the target's objects in, as its
registry states it of the target's format

## fun c_prefix

```mach
pub fun c_prefix(b: *Back, a: *A.Allocator) res[str, fail.Fail];
```

the prefix the target's format gives a C symbol's name on its architecture, empty
for none, as mink's registry states it

## fun symbol_name

```mach
pub fun symbol_name(o: *Object, i: usize) View;
```

the name of symbol `i` of `o`

## rec Image

```mach
pub rec Image;
```

what the build states of the image it links, beyond its kind and format

base: the address the image is laid out from, none for the host's
subsystem: the PE subsystem the image states, none for the host's
libdirs: the directories a library named by name is searched in ahead of the system's
libdir_count: how many

## fun link

```mach
pub fun link(b: *Back, a: *A.Allocator, kind: *output.Kind, format: *Format, requests: *Request, count: usize, output_path: str, gc: bool, image: *Image) res[Sink, fail.Fail];
```

the image of `kind` in `format` mink links from the `count` requests at `requests`, a library
named by name found by mink's search, named `output` where its format records a name, its
unreferenced sections left out with `gc`, laid out as `image` states

