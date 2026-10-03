# mach.lang.source.provider.disk

## rec Package

```mach
pub rec Package;
```

a package whose modules are files under a directory: the head segment of a module's fqn is
the package id, and the rest is the module's path under `src`

id: the package's `[project].id`
version: its `[project].version`
module: the fqn of its public module, or STR_NIL when it has none
dir: the package's directory
src: its source directory, relative to `dir`
artifactless: it declares no artifact
root: it is the package being built

## rec Disk

```mach
pub rec Disk;
```

the provider of modules read from the filesystem. a module exists only when every segment of
its path is spelled as its directory entry, so a case-insensitive filesystem resolves a name
as linux does. the root package is matched first, then the dependencies in the order added

## fun init

```mach
pub fun init(a: *A.Allocator, itn: *intern.Interner) Disk;
```

## fun dnit

```mach
pub fun dnit(d: *Disk);
```

## fun package_add

```mach
pub fun package_add(d: *Disk, p: Package) err[fail.Fail];
```

## fun packages_clear

```mach
pub fun packages_clear(d: *Disk);
```

## fun provider_of

```mach
pub fun provider_of(d: *Disk) provider.Provider;
```

