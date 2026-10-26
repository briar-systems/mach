# mach.lang.target.catalog.os

the operating system catalog: the systems of mink's catalog mach has a target
for, each with its id, its fingerprint tag and whether it can host a build, and
the filesystem policy an os declares. a row names mink's system row and is
spelled by it, and a system of mink's no row names (uefi, wasi, opencl) is no
os here and no `$mach.os` tag

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

## val VULKAN

```mach
pub val VULKAN:       u32 = 5
```

## val VERSION

```mach
pub val VERSION: u8 = 2
```

version 2: vulkan is a row of its own, which spir-v targets name in place of freestanding

## fun id_for

```mach
pub fun id_for(name: str) u32;
```

## fun name_for

```mach
pub fun name_for(id: u32) str;
```

## fun owner_of

```mach
pub fun owner_of(id: u32) *mink_os.System;
```

mink's system row the os `id` means, nil for an unknown id

## fun is_hosted

```mach
pub fun is_hosted(id: u32) bool;
```

whether the os can be a build host; false for an unknown id

## fun system_library

```mach
pub fun system_library(id: u32) str;
```

the system library a hosted program of the os links for the runtime it calls; empty when it
needs none or the id is unknown

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

