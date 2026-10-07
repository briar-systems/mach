# mach.lang.type.query.extent

the size, alignment and field offsets of a type, from the same checked layout the
backend lays it out by

## rec Measure

```mach
pub rec Measure;
```

a layout walk's view of the store: the query it answers within

## fun describe

```mach
pub fun describe(mz: *Measure, id: u32) layout.Node;
```

## fun field

```mach
pub fun field(mz: *Measure, id: u32, index: u32) u32;
```

## fun size

```mach
pub fun size(mz: *Measure, m: layout.Machine, tid: type.TypeId) layout.Extent;
```

## fun of

```mach
pub fun of(s: *session.Session, m: layout.Machine, tid: type.TypeId) res[layout.Extent, fail.Fail];
```

## fun field_offset

```mach
pub fun field_offset(s: *session.Session, m: layout.Machine, tid: type.TypeId, field_ix: u32) res[layout.Extent, fail.Fail];
```

the byte offset of a record field or tag payload from the same checked layout that sizes
the type; `size` carries the offset when `ok`

