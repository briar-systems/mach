# mach.lang.linkage

what a build declares about linkage, owned by the build's project and read
by the lowering that names declarations and the linker that binds imports:
each declaration's export name and library, the name each declaration links
under once the lowering has named it, the library each imported symbol is
pinned to, and the libraries the link is offered in the order the build
first named them

## rec Provider

```mach
pub rec Provider;
```

a library the link is offered by name

## def Sort

```mach
pub def Sort: u8
```

what the build knows an imported symbol to be

## val UNSTATED

```mach
pub val UNSTATED: Sort = 0
```

nothing, as of a name a manifest entry claims for an archive

## val CODE

```mach
pub val CODE: Sort = 1
```

a function, as an `ext fun` declares

## val DATA

```mach
pub val DATA: Sort = 2
```

a variable, as an `ext val` or `ext var` declares

## rec Import

```mach
pub rec Import;
```

an imported symbol pinned to a library

name: its link name
lib: the library's key
sort: what the build knows it to be

## rec Linkage

```mach
pub rec Linkage;
```

keyed by declaration: a module's index in the high 32 bits, its declaration
index in the low
imports holds every pinned import in the order it was first pinned, and import_at its index
there by link name

## fun init

```mach
pub fun init(alloc: *A.Allocator) Linkage;
```

## fun dnit

```mach
pub fun dnit(l: *Linkage);
```

## fun clear

```mach
pub fun clear(l: *Linkage);
```

forget every declaration, import and provider, as a project does when its
module registry is reset

## fun register_export_name

```mach
pub fun register_export_name(l: *Linkage, mid: module.ModuleId, did: u32, name: intern.StrId) err[fail.Fail];
```

## fun forget_names

```mach
pub fun forget_names(l: *Linkage, mid: module.ModuleId, decl_count: u32);
```

drop every export name and linkage name of one module's declarations, before
its syntax tree is replaced and its declaration ids no longer name the same
declarations

mid: the module whose names go
decl_count: the declaration count of the syntax tree they were registered against

## fun export_name

```mach
pub fun export_name(l: *Linkage, mid: module.ModuleId, did: u32) intern.StrId;
```

the name declaration `did` of `mid` is exported under, nil when it names none

## fun register_name

```mach
pub fun register_name(l: *Linkage, mid: module.ModuleId, did: u32, name: intern.StrId) err[fail.Fail];
```

record the name declaration `did` of `mid` links under, as the lowering
first names it

## fun name

```mach
pub fun name(l: *Linkage, mid: module.ModuleId, did: u32) intern.StrId;
```

the name declaration `did` of `mid` links under, nil before the lowering has named it

## fun register_export_library

```mach
pub fun register_export_library(l: *Linkage, mid: module.ModuleId, did: u32, lib: intern.StrId) err[fail.Fail];
```

## fun export_library

```mach
pub fun export_library(l: *Linkage, mid: module.ModuleId, did: u32) intern.StrId;
```

the library declaration `did` of `mid` is imported from, nil when it names none

## fun register_import_library

```mach
pub fun register_import_library(l: *Linkage, name: intern.StrId, lib: intern.StrId, sort: Sort) err[fail.Fail];
```

pin the imported symbol `name`, known as `sort`, to the library `lib`. a symbol pinned
again takes the later pin, a sort the earlier one left unstated included

## fun import_library

```mach
pub fun import_library(l: *Linkage, name: intern.StrId) intern.StrId;
```

the library the imported symbol `name` is pinned to, nil when it is pinned to none

## fun register_provider

```mach
pub fun register_provider(l: *Linkage, name: intern.StrId) err[fail.Fail];
```

offer the link the library `name`; a library offered again keeps its place

## fun provider

```mach
pub fun provider(l: *Linkage, name: intern.StrId) opt[Provider];
```

the library `name` as the link is offered it, none when it is not

## fun providers_clear

```mach
pub fun providers_clear(l: *Linkage);
```

forget every library offered, as each link names its own

