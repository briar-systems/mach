[mach](../../../README.md) / [base](../../README.md) / [source](../README.md) / [provider](README.md) / **disk**

# mach.base.source.provider.disk

the source provider that reads modules from package directories on disk

a module's fqn is headed by its package's id and the rest is its path under the package's
source directory. a module exists only when every segment is spelled exactly as its directory
entry, so a case-insensitive filesystem resolves a name as linux does.

Source: [`src/base/source/provider/disk.mach`](../../../../../src/base/source/provider/disk.mach)

Uses types from [`mach.base.fail`](../../fail.md), [`mach.base.intern`](../../intern.md), [`mach.base.source.provider`](../provider.md).

| Type | | |
| :--- | :--- | :--- |
| [`Package`](#package) | `rec` | a package whose modules are files under a directory: the head segment of a module's fqn is the package id, and the rest is the module's path under `src` |
| [`Disk`](#disk) | `rec` | the provider of modules read from the filesystem |

| On `Disk` | |
| :--- | :--- |
| [`init`](#init), [`dnit`](#dnit), [`package_add`](#package_add), [`packages_clear`](#packages_clear), [`provider_of`](#provider_of) | |

---

## Package

```mach
pub rec Package {
    id:      mach.base.intern.StrId;
    version: mach.base.intern.StrId;
    dir:     mach.base.intern.StrId;
    src:     mach.base.intern.StrId;
    root:    bool;
}
```

a package whose modules are files under a directory: the head segment of a module's fqn is
the package id, and the rest is the module's path under `src`

| Field | Type | |
| :--- | :--- | :--- |
| `id` | `mach.base.intern.StrId` | the package's `[project].id` |
| `version` | `mach.base.intern.StrId` | its `[project].version` |
| `dir` | `mach.base.intern.StrId` | the package's directory |
| `src` | `mach.base.intern.StrId` | its source directory, relative to `dir` |
| `root` | `bool` | it is the package being built |

## Disk

```mach
pub rec Disk {
    itn:      *mach.base.intern.Interner;
    packages: std.collections.vector.Vector[Package];
}
```

the provider of modules read from the filesystem. a module exists only when every segment of
its path is spelled as its directory entry, so a case-insensitive filesystem resolves a name
as linux does. the root package is matched first, then the dependencies in the order added

### init

```mach
pub fun init(a: *std.allocator.Allocator, itn: *mach.base.intern.Interner) Disk
```

### dnit

```mach
pub fun dnit(d: *Disk)
```

### package_add

```mach
pub fun package_add(d: *Disk, p: Package) err[mach.base.fail.Fail]
```

### packages_clear

```mach
pub fun packages_clear(d: *Disk)
```

### provider_of

```mach
pub fun provider_of(d: *Disk) mach.base.source.provider.Provider
```

