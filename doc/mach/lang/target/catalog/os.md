# mach.lang.target.catalog.os

the operating system catalog: every os id, its spelling, its fingerprint tag
and whether it can host a build, and the filesystem policy an os declares

## val UNKNOWN

```mach
pub val UNKNOWN:      u32 = 0
```

## val LINUX

```mach
pub val LINUX:        u32 = 1
```

## val DARWIN

```mach
pub val DARWIN:       u32 = 2
```

## val WINDOWS

```mach
pub val WINDOWS:      u32 = 3
```

## val FREESTANDING

```mach
pub val FREESTANDING: u32 = 4
```

## val VERSION

```mach
pub val VERSION: u8 = 1
```

## fun id_for

```mach
pub fun id_for(name: str) u32;
```

## fun name_for

```mach
pub fun name_for(id: u32) str;
```

## fun is_hosted

```mach
pub fun is_hosted(id: u32) bool;
```

whether the os can be a build host; false for an unknown id

## fun count

```mach
pub fun count() usize;
```

## fun name_at

```mach
pub fun name_at(index: usize) str;
```

## fun fingerprint_tag

```mach
pub fun fingerprint_tag(id: u32) u8;
```

## val LIBDIR_MAX

```mach
pub val LIBDIR_MAX: u32 = 8
```

## rec FsPolicy

```mach
pub rec FsPolicy;
```

