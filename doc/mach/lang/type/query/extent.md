# mach.lang.type.query.extent

the size, alignment and member offsets of a type, as mirl's data layout of the build's
target lays it out: each source type is interned in a type table the laying holds, no
module's, and mirl answers every question of it, so mach computes no layout of its own

## rec Laying

```mach
pub rec Laying;
```

what a build lays its types out by: mirl's target and the context its selection opened,
borrowed from the build's back end, and the type table each source type is interned in

mapped: the table's type of each source type interned, under `generation`
generation: the projection generation `mapped` was made under, since a later one may
            give a nominal other fields

## fun open

```mach
pub fun open(a: *A.Allocator, target: *mirl_record.Target, ctx: opt[*core_state.Context]) res[*Laying, fail.Fail];
```

a laying over `target` under the context `ctx`, both borrowed. given back by close

## fun close

```mach
pub fun close(l: *Laying);
```

## def Cause

```mach
pub def Cause: u8
```

## val CAUSE_UNKNOWN

```mach
pub val CAUSE_UNKNOWN:     Cause = 1
```

## val CAUSE_FIELD_RANGE

```mach
pub val CAUSE_FIELD_RANGE: Cause = 4
```

## rec Extent

```mach
pub rec Extent;
```

a type's size and alignment, or why it has none. an offset is carried in `size`

## fun unknown

```mach
pub fun unknown(cause: Cause) Extent;
```

## fun cause_name

```mach
pub fun cause_name(cause: Cause) opt[str];
```

the text of a cause; absent for one outside the catalog

## val NODE_UNKNOWN

```mach
pub val NODE_UNKNOWN:   NodeKind = 0
```

## val NODE_SCALAR

```mach
pub val NODE_SCALAR:    NodeKind = 1
```

## val NODE_POINTER

```mach
pub val NODE_POINTER:   NodeKind = 2
```

## val NODE_VECTOR

```mach
pub val NODE_VECTOR:    NodeKind = 3
```

## val NODE_ARRAY

```mach
pub val NODE_ARRAY:     NodeKind = 4
```

## val NODE_AGGREGATE

```mach
pub val NODE_AGGREGATE: NodeKind = 5
```

## val NODE_ZERO

```mach
pub val NODE_ZERO:      NodeKind = 6
```

## val NODE_OPAQUE

```mach
pub val NODE_OPAQUE:    NodeKind = 7
```

## val NODE_TAG

```mach
pub val NODE_TAG:       NodeKind = 8
```

## rec Node

```mach
pub rec Node;
```

what a source type is made of, as its layout is asked

bytes: a scalar's or a vector element's size, a tag's discriminator's
count: an array's or a vector's element count
child: an array's element type
field_count: an aggregate's fields or a tag's cases
align: the alignment a declaration states, 0 for none
overlays: a union, whose fields overlap
packed: its fields placed with no padding

## rec Measure

```mach
pub rec Measure;
```

a layout walk's view of the store and of the target: the query it answers within, and the
laying, nil where the build opened no target

## fun describe

```mach
pub fun describe(mz: *Measure, id: u32) Node;
```

## fun field

```mach
pub fun field(mz: *Measure, id: u32, index: u32) u32;
```

## fun size

```mach
pub fun size(mz: *Measure, tid: type_kind.TypeId) Extent;
```

## fun offset

```mach
pub fun offset(mz: *Measure, tid: type_kind.TypeId, field_ix: u32) Extent;
```

the byte offset of a record field, or of a tag's payload for any of its cases, in `size`.
a union's fields all stand at 0

## fun of

```mach
pub fun of(s: *session.Session, laying: *Laying, tid: type_kind.TypeId) res[Extent, fail.Fail];
```

## fun field_offset

```mach
pub fun field_offset(s: *session.Session, laying: *Laying, tid: type_kind.TypeId, field_ix: u32) res[Extent, fail.Fail];
```

the byte offset of a record field or tag payload from the same layout that sizes the type;
`size` carries the offset when `ok`

