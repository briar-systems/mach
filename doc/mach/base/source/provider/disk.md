[mach](../../../README.md) / [base](../../README.md) / [source](../README.md) / [provider](README.md) / **disk**

# mach.base.source.provider.disk

the source provider that reads modules from package directories on disk

a module's fqn is headed by its package's id and the rest is its path under the package's
source directory. a module exists only when every segment is spelled exactly as its directory
entry, so a case-insensitive filesystem resolves a name as linux does.

<sub>source: [`src/base/source/provider/disk.mach`](../../../../../src/base/source/provider/disk.mach) · imported by 2 modules</sub>

<sub>`intern` is [`mach.base.intern`](../../intern.md) · `collections_vector` is `std.collections.vector` · `A` is `std.allocator` · `fail` is [`mach.base.fail`](../../fail.md) · `provider` is [`mach.base.source.provider`](../provider.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Package`](#package) | `rec` | a package whose modules are files under a directory: the head segment of a module's fqn is the package id, and the rest is the module's path under `src` |
| [`Disk`](#disk) | `rec` | the provider of modules read from the filesystem |

| On `Disk` | |
| :--- | :--- |
| [`init`](#init) · [`dnit`](#dnit) · [`package_add`](#package_add) · [`packages_clear`](#packages_clear) · [`provider_of`](#provider_of) | |

---

## Package

```mach
pub rec Package {
    id:      intern.StrId;
    version: intern.StrId;
    dir:     intern.StrId;
    src:     intern.StrId;
    root:    bool;
}
```

a package whose modules are files under a directory: the head segment of a module's fqn is
the package id, and the rest is the module's path under `src`

| Field | Type | |
| :--- | :--- | :--- |
| `id` | `intern.StrId` | the package's `[project].id` |
| `version` | `intern.StrId` | its `[project].version` |
| `dir` | `intern.StrId` | the package's directory |
| `src` | `intern.StrId` | its source directory, relative to `dir` |
| `root` | `bool` | it is the package being built |

## Disk

```mach
pub rec Disk {
    itn:      *intern.Interner;
    packages: collections_vector.Vector[Package];
}
```

the provider of modules read from the filesystem. a module exists only when every segment of
its path is spelled as its directory entry, so a case-insensitive filesystem resolves a name
as linux does. the root package is matched first, then the dependencies in the order added

### init

```mach
pub fun init(a: *A.Allocator, itn: *intern.Interner) Disk
```

### dnit

```mach
pub fun dnit(d: *Disk)
```

### package_add

```mach
pub fun package_add(d: *Disk, p: Package) err[fail.Fail]
```

### packages_clear

```mach
pub fun packages_clear(d: *Disk)
```

### provider_of

```mach
pub fun provider_of(d: *Disk) provider.Provider
```

