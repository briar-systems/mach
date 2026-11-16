[mach](../README.md) / [lower](README.md) / **linkage**

# mach.lower.linkage

what a build declares about linkage, owned by the build's project and read
by the lowering that names declarations and the linker that binds imports:
each declaration's export name and library, the name each declaration links
under once the lowering has named it, and the library each imported symbol is
pinned to

<sub>source: [`src/lower/linkage.mach`](../../../src/lower/linkage.mach) · imported by 7 modules</sub>

<sub>`intern` is [`mach.base.intern`](../base/intern.md) · `map` is `std.collections.map` · `A` is `std.allocator` · `module` is [`mach.session.module`](../session/module.md) · `fail` is [`mach.base.fail`](../base/fail.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Sort`](#sort) | `def` | what the build knows an imported symbol to be |
| [`Import`](#import) | `rec` | an imported symbol pinned to a library |
| [`Linkage`](#linkage) | `rec` | keyed by declaration: a module's index in the high 32 bits, its declaration index in the low imports holds every pinned import in the order it was first pinned, and import_at its index there by link name |

| On `Sort` | |
| :--- | :--- |
| [`UNSTATED`](#sort) | nothing, as of a name a manifest entry claims for an archive |
| [`CODE`](#sort) | a function, as an `ext fun` declares |
| [`DATA`](#sort) | a variable, as an `ext val` or `ext var` declares |

| On `Linkage` | |
| :--- | :--- |
| [`init`](#init) · [`dnit`](#dnit) | |
| [`clear`](#clear) | forget every declaration and import, as a project does when its module registry is reset |
| [`register_export_name`](#register_export_name) | |
| [`forget_names`](#forget_names) | drop every export name and linkage name of one module's declarations, before its syntax tree is replaced and its declaration ids no longer name the same declarations |
| [`export_name`](#export_name) | the name declaration `did` of `mid` is exported under, nil when it names none |
| [`register_name`](#register_name) | record the name declaration `did` of `mid` links under, as the lowering first names it |
| [`name`](#name) | the name declaration `did` of `mid` links under, nil before the lowering has named it |
| [`register_export_library`](#register_export_library) | |
| [`export_library`](#export_library) | the library declaration `did` of `mid` is imported from, nil when it names none |
| [`register_import_library`](#register_import_library) | pin the imported symbol `name`, known as `sort`, to the library `lib` |
| [`import_library`](#import_library) | the library the imported symbol `name` is pinned to, nil when it is pinned to none |

---

## Sort

```mach
pub def Sort: u8
```

what the build knows an imported symbol to be

| Name | Value | |
| :--- | :--- | :--- |
| `UNSTATED` | `0` | nothing, as of a name a manifest entry claims for an archive |
| `CODE` | `1` | a function, as an `ext fun` declares |
| `DATA` | `2` | a variable, as an `ext val` or `ext var` declares |

## Import

```mach
pub rec Import {
    name: intern.StrId;
    lib:  intern.StrId;
    sort: Sort;
}
```

an imported symbol pinned to a library

| Field | Type | |
| :--- | :--- | :--- |
| `name` | `intern.StrId` | its link name |
| `lib` | `intern.StrId` | the library's key |
| `sort` | `Sort` | what the build knows it to be |

## Linkage

```mach
pub rec Linkage {
    export_names:     map.Map[u64, intern.StrId];
    names:            map.Map[u64, intern.StrId];
    export_libraries: map.Map[u64, intern.StrId];
    imports:          Vector[Import];
    import_at:        map.Map[intern.StrId, usize];
}
```

keyed by declaration: a module's index in the high 32 bits, its declaration
index in the low
imports holds every pinned import in the order it was first pinned, and import_at its index
there by link name

### init

```mach
pub fun init(alloc: *A.Allocator) Linkage
```

### dnit

```mach
pub fun dnit(l: *Linkage)
```

### clear

```mach
pub fun clear(l: *Linkage)
```

forget every declaration and import, as a project does when its
module registry is reset

### register_export_name

```mach
pub fun register_export_name(l: *Linkage, mid: module.ModuleId, did: u32, name: intern.StrId) err[fail.Fail]
```

### forget_names

```mach
pub fun forget_names(l: *Linkage, mid: module.ModuleId, decl_count: u32)
```

drop every export name and linkage name of one module's declarations, before
its syntax tree is replaced and its declaration ids no longer name the same
declarations

| Parameter | |
| :--- | :--- |
| `mid` | the module whose names go |
| `decl_count` | the declaration count of the syntax tree they were registered against |

### export_name

```mach
pub fun export_name(l: *Linkage, mid: module.ModuleId, did: u32) intern.StrId
```

the name declaration `did` of `mid` is exported under, nil when it names none

### register_name

```mach
pub fun register_name(l: *Linkage, mid: module.ModuleId, did: u32, name: intern.StrId) err[fail.Fail]
```

record the name declaration `did` of `mid` links under, as the lowering
first names it

### name

```mach
pub fun name(l: *Linkage, mid: module.ModuleId, did: u32) intern.StrId
```

the name declaration `did` of `mid` links under, nil before the lowering has named it

### register_export_library

```mach
pub fun register_export_library(l: *Linkage, mid: module.ModuleId, did: u32, lib: intern.StrId) err[fail.Fail]
```

### export_library

```mach
pub fun export_library(l: *Linkage, mid: module.ModuleId, did: u32) intern.StrId
```

the library declaration `did` of `mid` is imported from, nil when it names none

### register_import_library

```mach
pub fun register_import_library(l: *Linkage, name: intern.StrId, lib: intern.StrId, sort: Sort) err[fail.Fail]
```

pin the imported symbol `name`, known as `sort`, to the library `lib`. a symbol pinned
again takes the later pin, a sort the earlier one left unstated included

### import_library

```mach
pub fun import_library(l: *Linkage, name: intern.StrId) intern.StrId
```

the library the imported symbol `name` is pinned to, nil when it is pinned to none

