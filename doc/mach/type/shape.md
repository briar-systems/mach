[mach](../README.md) / [type](README.md) / **shape**

# mach.type.shape

the mirl shape of a source type: the one place mach decides how a type is held in mirl's
type table, which lowering interns into its module's table and sema into the table it lays
types out over, so the size sema answers and the type codegen builds cannot disagree

a record or union is its fields in order at the alignment and packing it declares, a tag
its discriminator then a union of payloads per area its placement gives, an opaque abi
type a structure of its bytes at its alignment. what reaching a member type means,
substitution, a memo, a cycle on the path, and how a tag's cases are placed, is the
caller's, through `Reach`

<sub>source: [`src/type/shape.mach`](../../../src/type/shape.mach) · imported by 7 modules</sub>

<sub>`types` is `mirl.ir.type.table` · `A` is `std.allocator` · `type_prim` is [`mach.type.prim`](prim.md) · `type_kind` is [`mach.type.kind`](kind.md) · `type` is [`mach.type`](../type.md) · `types_format` is `mirl.ir.type.format`</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Unshaped`](#unshaped) | `tag` | why a source type has no shape |
| [`Reach`](#reach) | `rec` | how a caller reaches what the shape of a type is made of |
| [`Carried`](#carried) | `rec` | where a tag holds the payload of a case |
| [`Placement`](#placement) | `rec` | how a tag's cases share bytes: two cases share an area only where they agree on the secrecy of every byte both cover, which mach.type.query.areas decides for the target |

| On `Unshaped` | |
| :--- | :--- |
| [`integer`](#integer) | a signless integer of `bits`, rounded up to a width mirl holds |
| [`scalar`](#scalar) | an integer or float primitive |
| [`opaque`](#opaque) | an opaque type of `size` bytes at the alignment `align` |

| On `Reach` | |
| :--- | :--- |
| [`of`](#of) | the shape of the source type `tid`, its secrecy and its substitution already the caller's |

| On `Placement` | |
| :--- | :--- |
| [`carried`](#carried-1) | where case `i` holds its payload, none for a case with no payload |

| Functions | |
| :--- | :--- |
| [`format`](#format) | the format of a float of `bits`, none for a width mirl has no format for |

| Constants | |
| :--- | :--- |
| [`TAG_DISCRIMINANT`](#constants) | the member of a tag's structure that holds its discriminator |
| [`TAG_AREA`](#constants) · [`NO_AREA`](#constants) | |

---

## Unshaped

```mach
pub tag Unshaped: u8 {
    member;
    unknown;
    empty_tag;
    discriminator;
    table: types.Error;
    alloc: A.Error;
    placement;
}
```

why a source type has no shape

| Case | Payload | |
| :--- | :--- | :--- |
| `member` |  | reaching a member type refused it, the caller's reason held by its context |
| `unknown` |  | a kind that has no shape: a type parameter, a pack, an unresolved nominal |
| `empty_tag` |  | a tag with no cases |
| `discriminator` |  | a tag with no discriminator type |
| `table` | `types.Error` | mirl's type table refused a type |
| `alloc` | `A.Error` | no storage for a member list |
| `placement` |  | placing a tag's cases was refused, the caller's reason held by its context |

### integer

```mach
pub fun integer(t: *types.Table, bits: u32) res[Type, Unshaped]
```

a signless integer of `bits`, rounded up to a width mirl holds

### scalar

```mach
pub fun scalar(t: *types.Table, d: type_prim.PrimDesc) res[Type, Unshaped]
```

an integer or float primitive

### opaque

```mach
pub fun opaque(t: *types.Table, size: u32, align: u32) res[Type, Unshaped]
```

an opaque type of `size` bytes at the alignment `align`

## Reach

```mach
pub rec Reach[T] {
    ctx:     *T;
    member:  fun(*T, type_kind.TypeId) opt[Type];
    handle:  fun(*T, *type.Type) opt[Type];
    place:   fun(*T, type_kind.TypeId) opt[Placement];
    pointer: Type;
}
```

how a caller reaches what the shape of a type is made of

| Field | Type | |
| :--- | :--- | :--- |
| `ctx` | `*T` | the caller's context |
| `member` | `fun(*T, type_kind.TypeId) opt[Type]` | the type of a member, element or payload, interned in the same table, none when the caller refuses it with a reason it holds |
| `handle` | `fun(*T, *type.Type) opt[Type]` | the type of a target handle |
| `place` | `fun(*T, type_kind.TypeId) opt[Placement]` | the placement of a tag's cases, none when the caller refuses it with a reason it holds |
| `pointer` | `Type` | the type a pointer is held in |

### of

```mach
pub fun of[T](t: *types.Table, ti: *type.TypeInterner, tid: type_kind.TypeId, r: *Reach[T]) res[Type, Unshaped]
```

the shape of the source type `tid`, its secrecy and its substitution already the caller's.
a nil type is an empty structure

## Carried

```mach
pub rec Carried {
    area:   u32;
    member: u32;
}
```

where a tag holds the payload of a case

| Field | Type | |
| :--- | :--- | :--- |
| `area` | `u32` | the member of the tag's structure that is the union of the payloads it shares, NO_AREA for a case with no payload |
| `member` | `u32` | the member of that union the payload is laid out as |

## Placement

```mach
pub rec Placement {
    cases: Slice[Carried];
    areas: u32;
}
```

how a tag's cases share bytes: two cases share an area only where they agree on the
secrecy of every byte both cover, which mach.type.query.areas decides for the target

| Field | Type | |
| :--- | :--- | :--- |
| `cases` | `Slice[Carried]` | where each case's payload is held, in case order |
| `areas` | `u32` | the count of areas |

### carried

```mach
pub fun carried(p: Placement, i: u32) opt[Carried]
```

where case `i` holds its payload, none for a case with no payload

## Functions

### format

```mach
pub fun format(bits: u32) opt[types_format.Format]
```

the format of a float of `bits`, none for a width mirl has no format for

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `TAG_DISCRIMINANT` | `u32` | `0` | the member of a tag's structure that holds its discriminator |
| `TAG_AREA` | `u32` | `1` |  |
| `NO_AREA` | `u32` | `0` |  |

