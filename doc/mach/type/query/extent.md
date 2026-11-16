[mach](../../README.md) / [type](../README.md) / [query](README.md) / **extent**

# mach.type.query.extent

the size, alignment and member offsets of a type, as mirl's data layout of the build's
target lays it out: each source type is interned in a type table the laying holds, no
module's, and mirl answers every question of it, so mach computes no layout of its own

<sub>source: [`src/type/query/extent.mach`](../../../../src/type/query/extent.mach) · imported by 8 modules</sub>

<sub>`A` is `std.allocator` · `mirl_record` is `mirl.target.record` · `core_state` is `masc.core.state` · `types` is `mirl.ir.type.table` · `map` is `std.collections.map` · `type_kind` is [`mach.type.kind`](../kind.md) · `collections_vector` is `std.collections.vector` · `shape` is [`mach.type.shape`](../shape.md) · `fail` is [`mach.base.fail`](../../base/fail.md) · `Placed` is private · `session` is [`mach.session`](../../session.md) · `NodeKind` is private · `type_query` is [`mach.type.query`](../query.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Laying`](#laying) | `rec` | what a build lays its types out by: mirl's target and the context its selection opened, borrowed from the build's back end, and the type table each source type is interned in |
| [`Cause`](#cause) | `def` |  |
| [`Extent`](#extent) | `rec` | a type's size and alignment, or why it has none |
| [`Node`](#node) | `rec` | what a source type is made of, as its layout is asked |
| [`Measure`](#measure) | `rec` | a layout walk's view of the store and of the target: the query it answers within, and the laying, nil where the build opened no target |

| On `Laying` | |
| :--- | :--- |
| [`init`](#init) | a laying over `target` under the context `ctx`, both borrowed, placing tags by `place` |
| [`dnit`](#dnit) · [`of`](#of) | |
| [`field_offset`](#field_offset) | the byte offset of a record field or tag payload from the same layout that sizes the type; `size` carries the offset when `ok` |

| On `Cause` | |
| :--- | :--- |
| [`CAUSE_UNKNOWN`](#cause) · [`CAUSE_FIELD_RANGE`](#cause) · [`unknown`](#unknown) | |
| [`cause_name`](#cause_name) | the text of a cause; absent for one outside the catalog |

| On `Measure` | |
| :--- | :--- |
| [`placement`](#placement) | where the tag `tid`, its secrecy stripped, holds each case's payload on the laying's target |
| [`describe`](#describe) · [`field`](#field) · [`size`](#size) | |
| [`offset`](#offset) | the byte offset of a record field, or of a tag case's payload, in `size`; a case with no payload has none |

| Constants | |
| :--- | :--- |
| [`NODE_UNKNOWN`](#constants) · [`NODE_SCALAR`](#constants) · [`NODE_POINTER`](#constants) · [`NODE_VECTOR`](#constants) · [`NODE_ARRAY`](#constants) · [`NODE_AGGREGATE`](#constants) · [`NODE_ZERO`](#constants) · [`NODE_OPAQUE`](#constants) · [`NODE_TAG`](#constants) | |

---

## Laying

```mach
pub rec Laying {
    alloc:      *A.Allocator;
    target:     *mirl_record.Target;
    ctx:        opt[*core_state.Context];
    table:      types.Table;
    mapped:     map.Map[type_kind.TypeId, Type];
    generation: u64;
    place:      fun(*Measure, type_kind.TypeId, *collections_vector.Vector[shape.Carried]) res[u32, fail.Fail];
    placed:     map.Map[type_kind.TypeId, Placed];
    cases:      collections_vector.Vector[shape.Carried];
}
```

what a build lays its types out by: mirl's target and the context its selection opened,
borrowed from the build's back end, and the type table each source type is interned in

| Field | Type | |
| :--- | :--- | :--- |
| `mapped` | `map.Map[type_kind.TypeId, Type]` | the table's type of each source type interned, under `generation` |
| `generation` | `u64` | the projection generation `mapped` and `placed` were made under, since a later one may give a nominal other fields |
| `place` | `fun(*Measure, type_kind.TypeId, *collections_vector.Vector[shape.Carried]) res[u32, fail.Fail]` | how a tag's cases are placed on this target, which mach.type.query.areas decides from the secrecy of their bytes, a query above this one |
| `placed` | `map.Map[type_kind.TypeId, Placed]` | each tag placed, its cases at `start` in `cases`, under `generation` |

### init

```mach
pub fun init(a: *A.Allocator, target: *mirl_record.Target, ctx: opt[*core_state.Context],
    place: fun(*Measure, type_kind.TypeId, *collections_vector.Vector[shape.Carried]) res[u32, fail.Fail]) res[*Laying, fail.Fail]
```

a laying over `target` under the context `ctx`, both borrowed, placing tags by `place`.
given back by close

### dnit

```mach
pub fun dnit(l: *Laying)
```

### of

```mach
pub fun of(s: *session.Session, laying: *Laying, tid: type_kind.TypeId) res[Extent, fail.Fail]
```

### field_offset

```mach
pub fun field_offset(s: *session.Session, laying: *Laying, tid: type_kind.TypeId, field_ix: u32) res[Extent, fail.Fail]
```

the byte offset of a record field or tag payload from the same layout that sizes the type;
`size` carries the offset when `ok`

## Cause

```mach
pub def Cause: u8
```

| Name | Value | |
| :--- | :--- | :--- |
| `CAUSE_UNKNOWN` | `1` |  |
| `CAUSE_FIELD_RANGE` | `4` |  |

### unknown

```mach
pub fun unknown(cause: Cause) Extent
```

### cause_name

```mach
pub fun cause_name(cause: Cause) opt[str]
```

the text of a cause; absent for one outside the catalog

## Extent

```mach
pub rec Extent {
    ok:    bool;
    cause: Cause;
    size:  u32;
    align: u32;
}
```

a type's size and alignment, or why it has none. an offset is carried in `size`

## Node

```mach
pub rec Node {
    kind:        NodeKind;
    bytes:       u32;
    count:       u32;
    child:       u32;
    field_count: u32;
    align:       u32;
    overlays:    bool;
    packed:      bool;
}
```

what a source type is made of, as its layout is asked

| Field | Type | |
| :--- | :--- | :--- |
| `bytes` | `u32` | a scalar's or a vector element's size, a tag's discriminator's |
| `count` | `u32` | an array's or a vector's element count |
| `child` | `u32` | an array's element type |
| `field_count` | `u32` | an aggregate's fields or a tag's cases |
| `align` | `u32` | the alignment a declaration states, 0 for none |
| `overlays` | `bool` | a union, whose fields overlap |
| `packed` | `bool` | its fields placed with no padding |

## Measure

```mach
pub rec Measure {
    q:      *type_query.Query;
    laying: *Laying;
}
```

a layout walk's view of the store and of the target: the query it answers within, and the
laying, nil where the build opened no target

### placement

```mach
pub fun placement(mz: *Measure, tid: type_kind.TypeId) res[shape.Placement, fail.Fail]
```

where the tag `tid`, its secrecy stripped, holds each case's payload on the laying's target.
the cases it gives are the laying's, good until the next tag is placed

### describe

```mach
pub fun describe(mz: *Measure, id: u32) Node
```

### field

```mach
pub fun field(mz: *Measure, id: u32, index: u32) u32
```

### size

```mach
pub fun size(mz: *Measure, tid: type_kind.TypeId) Extent
```

### offset

```mach
pub fun offset(mz: *Measure, tid: type_kind.TypeId, field_ix: u32) Extent
```

the byte offset of a record field, or of a tag case's payload, in `size`; a case with no
payload has none. a union's fields all stand at 0

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `NODE_UNKNOWN` | `NodeKind` | `0` |  |
| `NODE_SCALAR` | `NodeKind` | `1` |  |
| `NODE_POINTER` | `NodeKind` | `2` |  |
| `NODE_VECTOR` | `NodeKind` | `3` |  |
| `NODE_ARRAY` | `NodeKind` | `4` |  |
| `NODE_AGGREGATE` | `NodeKind` | `5` |  |
| `NODE_ZERO` | `NodeKind` | `6` |  |
| `NODE_OPAQUE` | `NodeKind` | `7` |  |
| `NODE_TAG` | `NodeKind` | `8` |  |

