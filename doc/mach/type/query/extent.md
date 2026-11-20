[mach](../../README.md) / [type](../README.md) / [query](README.md) / **extent**

# mach.type.query.extent

the size, alignment and member offsets of a type, as mirl's data layout of the build's
target lays it out: each source type is interned in a type table the laying holds, no
module's, and mirl answers every question of it, so mach computes no layout of its own

Source: [`src/type/query/extent.mach`](../../../../src/type/query/extent.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.session`](../../session.md), [`mach.type.kind`](../kind.md), [`mach.type.query`](../query.md), [`mach.type.shape`](../shape.md).

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
| [`dnit`](#dnit) | release a laying and its type table, a nil laying holding nothing |
| [`of`](#of) | the size and alignment of a type, asked as a query of its own |
| [`field_offset`](#field_offset) | the byte offset of a record field or tag payload from the same layout that sizes the type; `size` carries the offset when `ok` |

| On `Cause` | |
| :--- | :--- |
| [`CAUSE_UNKNOWN`](#cause), [`CAUSE_FIELD_RANGE`](#cause) | |
| [`unknown`](#unknown) | the extent of a type that has none |
| [`cause_name`](#cause_name) | the text of a cause; absent for one outside the catalog |

| On `Measure` | |
| :--- | :--- |
| [`placement`](#placement) | where the tag `tid`, its secrecy stripped, holds each case's payload on the laying's target |
| [`describe`](#describe) | what a source type is made of, its secrecy stripped |
| [`field`](#field) | the type of one field or case of a type, its secrecy stripped |
| [`size`](#size) | the size and alignment of a type on the laying's target |
| [`offset`](#offset) | the byte offset of a record field, or of a tag case's payload, in `size`; a case with no payload has none |

| Constants | |
| :--- | :--- |
| [`NODE_UNKNOWN`](#constants), [`NODE_SCALAR`](#constants), [`NODE_POINTER`](#constants), [`NODE_VECTOR`](#constants), [`NODE_ARRAY`](#constants), [`NODE_AGGREGATE`](#constants), [`NODE_ZERO`](#constants), [`NODE_OPAQUE`](#constants), [`NODE_TAG`](#constants) | |

---

## Laying

```mach
pub rec Laying {
    alloc:      *std.allocator.Allocator;
    target:     *mirl.target.record.Target;
    ctx:        opt[*masc.core.state.Context];
    table:      mirl.ir.type.table.Table;
    mapped:     std.collections.map.Map[mach.type.kind.TypeId, Type];
    generation: u64;
    place:      fun(*Measure, mach.type.kind.TypeId, *std.collections.vector.Vector[mach.type.shape.Carried]) res[u32, mach.base.fail.Fail];
    placed:     std.collections.map.Map[mach.type.kind.TypeId, Placed];
    cases:      std.collections.vector.Vector[mach.type.shape.Carried];
}
```

what a build lays its types out by: mirl's target and the context its selection opened,
borrowed from the build's back end, and the type table each source type is interned in

| Field | Type | |
| :--- | :--- | :--- |
| `mapped` | `std.collections.map.Map[mach.type.kind.TypeId, Type]` | the table's type of each source type interned, under `generation` |
| `generation` | `u64` | the projection generation `mapped` and `placed` were made under, since a later one may give a nominal other fields |
| `place` | `fun(*Measure, mach.type.kind.TypeId, *std.collections.vector.Vector[mach.type.shape.Carried]) res[u32, mach.base.fail.Fail]` | how a tag's cases are placed on this target, which mach.type.query.areas decides from the secrecy of their bytes, a query above this one |
| `placed` | `std.collections.map.Map[mach.type.kind.TypeId, Placed]` | each tag placed, its cases at `start` in `cases`, under `generation` |

### init

```mach
pub fun init(a: *std.allocator.Allocator, target: *mirl.target.record.Target, ctx: opt[*masc.core.state.Context],
    place: fun(*Measure, mach.type.kind.TypeId, *std.collections.vector.Vector[mach.type.shape.Carried]) res[u32, mach.base.fail.Fail]) res[*Laying, mach.base.fail.Fail]
```

a laying over `target` under the context `ctx`, both borrowed, placing tags by `place`.
given back by dnit

### dnit

```mach
pub fun dnit(l: *Laying)
```

release a laying and its type table, a nil laying holding nothing

| Parameter | |
| :--- | :--- |
| `l` | the laying |

### of

```mach
pub fun of(s: *mach.session.Session, laying: *Laying, tid: mach.type.kind.TypeId) res[Extent, mach.base.fail.Fail]
```

the size and alignment of a type, asked as a query of its own

| Parameter | |
| :--- | :--- |
| `s` | the session holding the store and fact cache the query uses |
| `laying` | the laying, nil where the build opened no target |
| `tid` | the type |
| **returns** | its extent, or the internal failure or refusal the query met |

### field_offset

```mach
pub fun field_offset(s: *mach.session.Session, laying: *Laying, tid: mach.type.kind.TypeId, field_ix: u32) res[Extent, mach.base.fail.Fail]
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

the extent of a type that has none

| Parameter | |
| :--- | :--- |
| `cause` | why it has none |
| **returns** | the extent, not ok |

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
    q:      *mach.type.query.Query;
    laying: *Laying;
}
```

a layout walk's view of the store and of the target: the query it answers within, and the
laying, nil where the build opened no target

### placement

```mach
pub fun placement(mz: *Measure, tid: mach.type.kind.TypeId) res[mach.type.shape.Placement, mach.base.fail.Fail]
```

where the tag `tid`, its secrecy stripped, holds each case's payload on the laying's target.
the cases it gives are the laying's, good until the next tag is placed

### describe

```mach
pub fun describe(mz: *Measure, id: u32) Node
```

what a source type is made of, its secrecy stripped

| Parameter | |
| :--- | :--- |
| `mz` | the layout walk |
| `id` | the type's index |
| **returns** | its node, NODE_ZERO for the nil type and NODE_UNKNOWN for a type with no layout |

### field

```mach
pub fun field(mz: *Measure, id: u32, index: u32) u32
```

the type of one field or case of a type, its secrecy stripped

| Parameter | |
| :--- | :--- |
| `mz` | the layout walk |
| `id` | the type's index |
| `index` | the field's position |
| **returns** | the field type's index, the error type's for no such field |

### size

```mach
pub fun size(mz: *Measure, tid: mach.type.kind.TypeId) Extent
```

the size and alignment of a type on the laying's target

| Parameter | |
| :--- | :--- |
| `mz` | the layout walk |
| `tid` | the type |
| **returns** | its extent, or why it has none, CAUSE_UNAVAILABLE without a laying |

### offset

```mach
pub fun offset(mz: *Measure, tid: mach.type.kind.TypeId, field_ix: u32) Extent
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

