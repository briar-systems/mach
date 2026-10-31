# mach.lang.driver.backend

the back half a build hands its lowered modules to: mirl compiles a unit to mink's object
in memory, mink lays an object out in the target's format or reads one back, and mink links
objects into an image. nothing here starts a process or writes a file between the stages

## rec Back

```mach
pub rec Back;
```

the target a build compiles for and the formats it reads and writes

assembled: the target the owners assemble for the cell, nil for a cell none assembles yet
machine: what lowering builds its units for, borrowing `assembled`
format: the object format of the target's system
formats: mink's format registry

## fun none

```mach
pub fun none() Back;
```

a back half that compiles nothing, for a project with no target yet

## fun open

```mach
pub fun open(a: *A.Allocator, t: *resolved.Target) res[Back, fail.Fail];
```

the back half of the resolved cell `t`, none assembled where no owner assembles the cell

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
pub fun compile(b: *Back, a: *A.Allocator, m: *Module, level: u8, checked: bool, relax: bool, floats: relaxation.Relaxations, name: str) res[Object, fail.Fail];
```

the mirl module `m` compiled to mink's object at mirl's level `level`, its schedule's own checks
run with `checked`, the linker let relax its code with `relax` and the float relaxations `floats`
permitted. the module is the schedule's to rewrite, so it is spent here

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

## fun symbol_name

```mach
pub fun symbol_name(o: *Object, i: usize) View;
```

the name of symbol `i` of `o`

## fun link

```mach
pub fun link(b: *Back, a: *A.Allocator, kind: *output.Kind, format: *Format, sources: *Source, count: usize, output_path: str, gc: bool) res[Sink, fail.Fail];
```

the image of `kind` in `format` mink links from the `count` sources at `sources`, named `output` where
its format records a name, its unreferenced sections left out with `gc`

