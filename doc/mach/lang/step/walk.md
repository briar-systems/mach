# mach.lang.step.walk

a contained path walk: each component of a path beneath a root is read
without following a symlink, refused when it is not what the caller expects,
and made when the caller's policy creates it

## val PHYSICAL_REGULAR

```mach
pub val PHYSICAL_REGULAR:   u8 = 1
```

## val PHYSICAL_DIRECTORY

```mach
pub val PHYSICAL_DIRECTORY: u8 = 2
```

## val PHYSICAL_ENTRY

```mach
pub val PHYSICAL_ENTRY:     u8 = 3
```

## fun path_check

```mach
pub fun path_check(alloc: *A.Allocator, root: str, rel: str, expected: u8, label: str) err[fail.Fail];
```

## fun ancestors_check

```mach
pub fun ancestors_check(alloc: *A.Allocator, root: str, rel: str, label: str) err[fail.Fail];
```

## fun directory_ensure

```mach
pub fun directory_ensure(alloc: *A.Allocator, root: str, rel: str, label: str) err[fail.Fail];
```

## fun owned_directory_ensure

```mach
pub fun owned_directory_ensure(alloc: *A.Allocator, root: str, rel: str, label: str) err[fail.Fail];
```

as `directory_ensure`, in output layout the build owns

