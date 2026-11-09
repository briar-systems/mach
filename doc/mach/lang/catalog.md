# mach.lang.catalog

## tag Catalog

```mach
pub tag Catalog: u8 {
    malformed:   Member;
    unsupported: Unsupported;
    internal:    Member;
}
```

## fun malformed_member

```mach
pub fun malformed_member(catalog: str, tag: u32) Catalog;
```

a member read from input that the catalog does not declare

## fun malformed_member_in

```mach
pub fun malformed_member_in(catalog: str, tag: u32, where: str) Catalog;
```

the same, named with where the input came from

## fun unsupported_member

```mach
pub fun unsupported_member(catalog: str, member: str, by: str) Catalog;
```

a declared member that `by` (a target, format or phase) does not honor

## fun unknown_member

```mach
pub fun unknown_member(catalog: str, tag: u32) Catalog;
```

a member the compiler produced that its own catalog does not declare

## fun unknown_member_in

```mach
pub fun unknown_member_in(catalog: str, tag: u32, where: str) Catalog;
```

the same, named with the site that produced it

## fun catalog_text

```mach
pub fun catalog_text(a: *A.Allocator, c: Catalog) res[str, std_format.FormatError];
```

the message, owned by the caller's allocator (extent str_len + 1, released
with str_free); the only failure a literal format can meet is the allocator's

## fun catalog_interned

```mach
pub fun catalog_interned(itn: *intern.Interner, a: *A.Allocator, c: Catalog) res[str, fail.Fail];
```

the message, owned by the interner so it outlives temporary phase storage,
with `a` as scratch

## fun catalog

```mach
pub fun catalog(a: *A.Allocator, c: Catalog) fail.Fail;
```

the closed-catalog policy as a failure: an input or capability fault is the
user's, a compiler-produced member is internal. the failure owns the message
through the caller's allocator

## fun unknown_catalog

```mach
pub fun unknown_catalog(a: *A.Allocator, catalog_name: str, tag: u32) fail.Fail;
```

