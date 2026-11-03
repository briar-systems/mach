# mach.lang.driver.cache.compiler

## val UNAVAILABLE

```mach
pub val UNAVAILABLE: u8 = host.FAULT_UNAVAILABLE
```

## val INTERNAL

```mach
pub val INTERNAL:    u8 = host.FAULT_INTERNAL
```

## rec Error

```mach
pub rec Error;
```

## rec Identity

```mach
pub rec Identity;
```

the sha-256 of the running image's file

## fun identity_clear

```mach
pub fun identity_clear(id: *Identity);
```

## fun identity

```mach
pub fun identity(alloc: *A.Allocator, out: *Identity) err[Error];
```

identifies the current executable, never argv[0], once per process, by the
sha-256 of its file
success fills the identity, failure leaves the output and the memo untouched

