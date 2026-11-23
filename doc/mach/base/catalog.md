[mach](../README.md) / [base](README.md) / **catalog**

# mach.base.catalog

the refusal of a catalog member that no arm of a dispatch names

a dispatch over a finite catalog never answers an unknown member with a default. it refuses
through one of these, and the class says where the fault lies: a declared member a target,
format or phase cannot honor is unsupported and the user's, and a member the compiler produced
that its own catalog lacks is internal.

Source: [`src/base/catalog.mach`](../../../src/base/catalog.mach)

Uses types from [`mach.base.fail`](fail.md), [`mach.base.intern`](intern.md).

| Type | | |
| :--- | :--- | :--- |
| [`Catalog`](#catalog) | `tag` | the refusal of one catalog member, with the class that says where the fault lies |

| On `Catalog` | |
| :--- | :--- |
| [`unsupported_member`](#unsupported_member) | a declared member that `by` (a target, format or phase) does not honor |
| [`unknown_member`](#unknown_member) | a member the compiler produced that its own catalog does not declare |
| [`unknown_member_in`](#unknown_member_in) | the same, named with the site that produced it |
| [`catalog_text`](#catalog_text) | the message, owned by the caller's allocator (extent str_len + 1, released with str_free); the only failure a literal format can meet is the allocator's |
| [`catalog_interned`](#catalog_interned) | the message, owned by the interner so it outlives temporary phase storage, with `a` as scratch |
| [`catalog`](#catalog-1) | the closed-catalog policy as a failure: a capability fault is the user's, a compiler-produced member is internal |

| Functions | |
| :--- | :--- |
| [`unknown_catalog`](#unknown_catalog) | the internal failure for a member tag `catalog_name` does not declare |

---

## Catalog

```mach
pub tag Catalog: u8 {
    unsupported: Unsupported;
    internal:    Member;
}
```

the refusal of one catalog member, with the class that says where the fault lies

| Case | Payload | |
| :--- | :--- | :--- |
| `unsupported` | `Unsupported` | a declared member a target, format or phase does not honor, the user's fault |
| `internal` | `Member` | a member the compiler produced that its own catalog does not declare |

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
pub fun catalog_text(a: *std.allocator.Allocator, c: Catalog) res[str, std.format.FormatError]
```

the message, owned by the caller's allocator (extent str_len + 1, released
with str_free); the only failure a literal format can meet is the allocator's

### catalog_interned

```mach
pub fun catalog_interned(itn: *mach.base.intern.Interner, a: *std.allocator.Allocator, c: Catalog) res[str, mach.base.fail.Fail]
```

the message, owned by the interner so it outlives temporary phase storage,
with `a` as scratch

### catalog

```mach
pub fun catalog(a: *std.allocator.Allocator, c: Catalog) mach.base.fail.Fail
```

the closed-catalog policy as a failure: a capability fault is the user's, a
compiler-produced member is internal. the failure owns the message through
the caller's allocator

## Functions

### unknown_catalog

```mach
pub fun unknown_catalog(a: *std.allocator.Allocator, catalog_name: str, tag: u32) mach.base.fail.Fail
```

the internal failure for a member tag `catalog_name` does not declare

| Parameter | |
| :--- | :--- |
| `a` | the allocator the failure's message is owned through |
| `catalog_name` | the catalog's name as the message spells it |
| `tag` | the member's tag |
| **returns** | the internal failure, or the format refusal when the message could not be made |

