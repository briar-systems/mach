[mach](../README.md) / [lower](README.md) / **linkage**

# mach.lower.linkage

what a build declares about linkage, owned by the build's project and read
by the lowering that names declarations and the linker that binds imports:
each declaration's export name and library, the name each declaration links
under once the lowering has named it, and the library each imported symbol is
pinned to

Source: [`src/lower/linkage.mach`](../../../src/lower/linkage.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.base.intern`](../base/intern.md), [`mach.session.module`](../session/module.md).

| Type | | |
| :--- | :--- | :--- |
| [`Sort`](#sort) | `def` | what the build knows an imported symbol to be |
| [`Import`](#import) | `rec` | an imported symbol pinned to a library |
| [`Named`](#named) | `rec` | the name each declaration links under once a lowering has named it, keyed as a linkage keys declarations |
| [`Linkage`](#linkage) | `rec` | keyed by declaration: a module's index in the high 32 bits, its declaration index in the low imports holds every pinned import in the order it was first pinned, and import_at its index there by link name |

| On `Sort` | |
| :--- | :--- |
| [`UNSTATED`](#sort) | nothing, as of a name a manifest entry claims for an archive |
| [`CODE`](#sort) | a function, as an `ext fun` declares |
| [`DATA`](#sort) | a variable, as an `ext val` or `ext var` declares |

| On `Named` | |
| :--- | :--- |
| [`named_init`](#named_init) | an empty memo |
| [`named_dnit`](#named_dnit) | give back a memo's map |
| [`register_name`](#register_name) | record in `n` the name declaration `did` of `mid` links under, as the lowering first names it |
| [`name`](#name) | the name `n` holds for declaration `did` of `mid`, nil before a lowering has named it there |

| On `Linkage` | |
| :--- | :--- |
| [`init`](#init) | an empty linkage record whose tables allocate from `alloc` |
| [`dnit`](#dnit) | release every table of `l` |
| [`clear`](#clear) | forget every declaration and import, as a project does when its module registry is reset |
| [`register_export_name`](#register_export_name) | record the name declaration `did` of `mid` is exported under, replacing an earlier one |
| [`forget_names`](#forget_names) | drop every export name and linkage name of one module's declarations, before its syntax tree is replaced and its declaration ids no longer name the same declarations |
| [`export_name`](#export_name) | the name declaration `did` of `mid` is exported under, nil when it names none |
| [`register_export_library`](#register_export_library) | record the library declaration `did` of `mid` is imported from, replacing an earlier one |
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
    name: mach.base.intern.StrId;
    lib:  mach.base.intern.StrId;
    sort: Sort;
}
```

an imported symbol pinned to a library

| Field | Type | |
| :--- | :--- | :--- |
| `name` | `mach.base.intern.StrId` | its link name |
| `lib` | `mach.base.intern.StrId` | the library's key |
| `sort` | `Sort` | what the build knows it to be |

## Named

```mach
pub rec Named {
    names: std.collections.map.Map[u64, mach.base.intern.StrId];
}
```

the name each declaration links under once a lowering has named it, keyed as a linkage keys
declarations. a name is a pure function of its key, so a memo serves every lowering that names
its strings where the memo's names were named: the project's serves each lowering on the owner,
and a lowering that names its strings in an interner of its own keeps one of its own

### named_init

```mach
pub fun named_init(alloc: *std.allocator.Allocator) Named
```

an empty memo

| Parameter | |
| :--- | :--- |
| `alloc` | the allocator the memo's map takes |
| **returns** | the memo, which `named_dnit` gives back |

### named_dnit

```mach
pub fun named_dnit(n: *Named)
```

give back a memo's map

| Parameter | |
| :--- | :--- |
| `n` | the memo, unusable after |

### register_name

```mach
pub fun register_name(n: *Named, mid: mach.session.module.ModuleId, did: u32, name: mach.base.intern.StrId) err[mach.base.fail.Fail]
```

record in `n` the name declaration `did` of `mid` links under, as the lowering
first names it

### name

```mach
pub fun name(n: *Named, mid: mach.session.module.ModuleId, did: u32) mach.base.intern.StrId
```

the name `n` holds for declaration `did` of `mid`, nil before a lowering has named it there

## Linkage

```mach
pub rec Linkage {
    export_names:     std.collections.map.Map[u64, mach.base.intern.StrId];
    named:            Named;
    export_libraries: std.collections.map.Map[u64, mach.base.intern.StrId];
    imports:          Vector[Import];
    import_at:        std.collections.map.Map[mach.base.intern.StrId, usize];
}
```

keyed by declaration: a module's index in the high 32 bits, its declaration
index in the low
imports holds every pinned import in the order it was first pinned, and import_at its index
there by link name

### init

```mach
pub fun init(alloc: *std.allocator.Allocator) Linkage
```

an empty linkage record whose tables allocate from `alloc`

| Parameter | |
| :--- | :--- |
| `alloc` | the allocator every table grows from |
| **returns** | the linkage, which the caller releases with dnit |

### dnit

```mach
pub fun dnit(l: *Linkage)
```

release every table of `l`

| Parameter | |
| :--- | :--- |
| `l` | the linkage to release |

### clear

```mach
pub fun clear(l: *Linkage)
```

forget every declaration and import, as a project does when its
module registry is reset

### register_export_name

```mach
pub fun register_export_name(l: *Linkage, mid: mach.session.module.ModuleId, did: u32, name: mach.base.intern.StrId) err[mach.base.fail.Fail]
```

record the name declaration `did` of `mid` is exported under, replacing an earlier one

| Parameter | |
| :--- | :--- |
| `l` | the linkage |
| `mid` | the declaration's module |
| `did` | the declaration's index in that module |
| `name` | the export name |
| **returns** | ok, or the allocator's refusal |

### forget_names

```mach
pub fun forget_names(l: *Linkage, mid: mach.session.module.ModuleId, decl_count: u32)
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
pub fun export_name(l: *Linkage, mid: mach.session.module.ModuleId, did: u32) mach.base.intern.StrId
```

the name declaration `did` of `mid` is exported under, nil when it names none

### register_export_library

```mach
pub fun register_export_library(l: *Linkage, mid: mach.session.module.ModuleId, did: u32, lib: mach.base.intern.StrId) err[mach.base.fail.Fail]
```

record the library declaration `did` of `mid` is imported from, replacing an earlier one

| Parameter | |
| :--- | :--- |
| `l` | the linkage |
| `mid` | the declaration's module |
| `did` | the declaration's index in that module |
| `lib` | the library's key |
| **returns** | ok, or the allocator's refusal |

### export_library

```mach
pub fun export_library(l: *Linkage, mid: mach.session.module.ModuleId, did: u32) mach.base.intern.StrId
```

the library declaration `did` of `mid` is imported from, nil when it names none

### register_import_library

```mach
pub fun register_import_library(l: *Linkage, name: mach.base.intern.StrId, lib: mach.base.intern.StrId, sort: Sort) err[mach.base.fail.Fail]
```

pin the imported symbol `name`, known as `sort`, to the library `lib`. a symbol pinned
again takes the later pin, a sort the earlier one left unstated included

### import_library

```mach
pub fun import_library(l: *Linkage, name: mach.base.intern.StrId) mach.base.intern.StrId
```

the library the imported symbol `name` is pinned to, nil when it is pinned to none

