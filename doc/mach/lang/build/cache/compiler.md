# mach.lang.build.cache.compiler

## val UNAVAILABLE

```mach
pub val UNAVAILABLE: u8 = 1
```

## val INTERNAL

```mach
pub val INTERNAL:    u8 = 2
```

## rec Error

```mach
pub rec Error;
```

## val SOURCE_BUILD_ID

```mach
pub val SOURCE_BUILD_ID: u8 = 1
```

where the identity came from: the build id the linker wrote into the running
image, or the sha-256 of the image file when it carries none

## val SOURCE_DIGEST

```mach
pub val SOURCE_DIGEST:   u8 = 2
```

## rec Identity

```mach
pub rec Identity;
```

## fun identity_clear

```mach
pub fun identity_clear(id: *Identity);
```

## fun identity_equal

```mach
pub fun identity_equal(a: *Identity, b: *Identity) bool;
```

## fun identity

```mach
pub fun identity(alloc: *A.Allocator, out: *Identity) err[Error];
```

identifies the current executable, never argv[0], once per process: the
linker's build id when the image carries one, else its sha-256
success fills the identity, failure leaves the output and the memo untouched

