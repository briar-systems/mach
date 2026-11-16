[mach](../README.md) / [base](README.md) / **catalog**

# mach.base.catalog

the refusal of a catalog member that no arm of a dispatch names

a dispatch over a finite catalog never answers an unknown member with a default. it refuses
through one of these, and the class says where the fault lies: a member read from input is
malformed and the user's, a declared member a target, format or phase cannot honor is
unsupported, and a member the compiler produced that its own catalog lacks is internal.

<sub>source: [`src/base/catalog.mach`](../../../src/base/catalog.mach) · imported by 11 modules</sub>

<sub>`Member` is private · `Unsupported` is private · `A` is `std.allocator` · `std_format` is `std.format` · `intern` is [`mach.base.intern`](intern.md) · `fail` is [`mach.base.fail`](fail.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Catalog`](#catalog) | `tag` |  |

| On `Catalog` | |
| :--- | :--- |
| [`malformed_member`](#malformed_member) | a member read from input that the catalog does not declare |
| [`malformed_member_in`](#malformed_member_in) | the same, named with where the input came from |
| [`unsupported_member`](#unsupported_member) | a declared member that `by` (a target, format or phase) does not honor |
| [`unknown_member`](#unknown_member) | a member the compiler produced that its own catalog does not declare |
| [`unknown_member_in`](#unknown_member_in) | the same, named with the site that produced it |
| [`catalog_text`](#catalog_text) | the message, owned by the caller's allocator (extent str_len + 1, released with str_free); the only failure a literal format can meet is the allocator's |
| [`catalog_interned`](#catalog_interned) | the message, owned by the interner so it outlives temporary phase storage, with `a` as scratch |
| [`catalog`](#catalog-1) | the closed-catalog policy as a failure: an input or capability fault is the user's, a compiler-produced member is internal |

| Functions | |
| :--- | :--- |
| [`unknown_catalog`](#unknown_catalog) | |

---

## Catalog

```mach
pub tag Catalog: u8 {
    malformed:   Member;
    unsupported: Unsupported;
    internal:    Member;
}
```

### malformed_member

```mach
pub fun malformed_member(catalog: str, tag: u32) Catalog
```

a member read from input that the catalog does not declare

### malformed_member_in

```mach
pub fun malformed_member_in(catalog: str, tag: u32, where: str) Catalog
```

the same, named with where the input came from

### unsupported_member

```mach
pub fun unsupported_member(catalog: str, member: str, by: str) Catalog
```

a declared member that `by` (a target, format or phase) does not honor

### unknown_member

```mach
pub fun unknown_member(catalog: str, tag: u32) Catalog
```

a member the compiler produced that its own catalog does not declare

### unknown_member_in

```mach
pub fun unknown_member_in(catalog: str, tag: u32, where: str) Catalog
```

the same, named with the site that produced it

### catalog_text

```mach
pub fun catalog_text(a: *A.Allocator, c: Catalog) res[str, std_format.FormatError]
```

the message, owned by the caller's allocator (extent str_len + 1, released
with str_free); the only failure a literal format can meet is the allocator's

### catalog_interned

```mach
pub fun catalog_interned(itn: *intern.Interner, a: *A.Allocator, c: Catalog) res[str, fail.Fail]
```

the message, owned by the interner so it outlives temporary phase storage,
with `a` as scratch

### catalog

```mach
pub fun catalog(a: *A.Allocator, c: Catalog) fail.Fail
```

the closed-catalog policy as a failure: an input or capability fault is the
user's, a compiler-produced member is internal. the failure owns the message
through the caller's allocator

## Functions

### unknown_catalog

```mach
pub fun unknown_catalog(a: *A.Allocator, catalog_name: str, tag: u32) fail.Fail
```

