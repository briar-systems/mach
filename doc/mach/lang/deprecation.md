# mach.lang.deprecation

## rec Deprecation

```mach
pub rec Deprecation;
```

a deprecation notice recorded on a declaration or a tag case: the file that owns the
annotation (so it never warns on itself) and the optional message

## fun none

```mach
pub fun none() Deprecation;
```

## fun equal

```mach
pub fun equal(a: Deprecation, b: Deprecation) bool;
```

an absent notice is one value whatever its other fields hold, so a zero-filled record
and `none()` compare equal

