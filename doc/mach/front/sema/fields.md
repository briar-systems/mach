[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **fields**

# mach.front.sema.fields

field graphs: the field tables a set of types reaches, captured for another reader

`capture` walks every type the roots reach, preparing each record, union and tag it meets, and
copies its fields, its tag discriminator and its layout decorations into a graph. `install`
publishes a graph into the type table's current projection and refuses one that disagrees with
a table already there, so a module's typing or published surface brings the field tables it
was read with.

Source: [`src/front/sema/fields.mach`](../../../../src/front/sema/fields.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.session`](../../session.md), [`mach.type`](../../type.md), [`mach.type.field`](../../type/field.md), [`mach.type.kind`](../../type/kind.md).

| Type | | |
| :--- | :--- | :--- |
| [`Node`](#node) | `rec` | one record, union, tag or materialized instance a graph captured |
| [`Roots`](#roots) | `rec` | one group of root types a capture starts from |
| [`Graph`](#graph) | `rec` | a module's field recipes, and the root table they were last installed in, under its projection generation, which every table descending from it answers them from |

| On `Roots` | |
| :--- | :--- |
| [`capture`](#capture) | walks every type the roots reach, preparing each record, union and tag, and copies the fields, discriminator and decorations of each into a new graph |

| On `Graph` | |
| :--- | :--- |
| [`init`](#init) | an empty graph |
| [`dnit`](#dnit) | releases every node of `graph` and the graph's own storage |
| [`install`](#install) | publishes each node of `graph` that the type table has no field table for, and holds every one it already has to the node's fields, discriminator and decorations |

| Constants | |
| :--- | :--- |
| [`RECIPES`](#constants), [`MATERIALIZED`](#constants) | |

---

## Node

```mach
pub rec Node {
    ty:         mach.type.kind.TypeId;
    entries:    std.collections.vector.Vector[mach.type.field.Entry];
    decoration: mach.type.field.Decoration;
    # the declared discriminator of a tag nominal; TYPE_NIL on every other node
    disc: mach.type.kind.TypeId;
}
```

one record, union, tag or materialized instance a graph captured

| Field | Type | |
| :--- | :--- | :--- |
| `ty` | `mach.type.kind.TypeId` | the type the node describes |
| `entries` | `std.collections.vector.Vector[mach.type.field.Entry]` | its fields, in declaration order |
| `decoration` | `mach.type.field.Decoration` | its layout decorations |

## Roots

```mach
pub rec Roots {
    data: *mach.type.kind.TypeId;
    len:  usize;
}
```

one group of root types a capture starts from

| Field | Type | |
| :--- | :--- | :--- |
| `data` | `*mach.type.kind.TypeId` | the first root |
| `len` | `usize` | how many roots the group holds |

### capture

```mach
pub fun capture[T](s: *mach.session.Session, a: *std.allocator.Allocator, roots: *Roots, count: usize, mode: CaptureMode, ctx: *T,
    prepare: fun(*T, mach.type.kind.TypeId) err[mach.base.fail.Fail]) res[Graph, mach.base.fail.Fail]
```

walks every type the roots reach, preparing each record, union and tag, and copies the
fields, discriminator and decorations of each into a new graph. in `MATERIALIZED` mode an
instance that has a field table is captured as well

| Parameter | |
| :--- | :--- |
| `s` | the session whose type table is read |
| `a` | the allocator the graph is held in |
| `roots` | the root groups |
| `count` | how many groups `roots` holds |
| `mode` | `RECIPES` or `MATERIALIZED` |
| `ctx` | the context passed to `prepare` |
| `prepare` | prepares one nominal's field table before it is read |
| **returns** | the graph, owned by the caller and released with `dnit`, or the failure of a scan, an allocation, a preparation or an invalid argument |

## Graph

```mach
pub rec Graph {
    nodes:                std.collections.vector.Vector[Node];
    installed:            bool;
    installed_in:         *mach.type.TypeInterner;
    installed_generation: u64;
}
```

a module's field recipes, and the root table they were last installed in, under its projection
generation, which every table descending from it answers them from

| Field | Type | |
| :--- | :--- | :--- |
| `nodes` | `std.collections.vector.Vector[Node]` | the captured field tables, sorted by type index and owned by the graph |
| `installed` | `bool` | whether `install` has published the graph in a root table |
| `installed_in` | `*mach.type.TypeInterner` | the root table it was published in, nil before the first root install |
| `installed_generation` | `u64` | the projection generation it was published under |

### init

```mach
pub fun init(a: *std.allocator.Allocator) Graph
```

an empty graph

| Parameter | |
| :--- | :--- |
| `a` | the allocator its nodes are held in |
| **returns** | the graph, released with `dnit` |

### dnit

```mach
pub fun dnit(graph: *Graph)
```

releases every node of `graph` and the graph's own storage

| Parameter | |
| :--- | :--- |
| `graph` | the graph to release |

### install

```mach
pub fun install(s: *mach.session.Session, graph: *Graph) err[mach.base.fail.Fail]
```

publishes each node of `graph` that the type table has no field table for, and holds every
one it already has to the node's fields, discriminator and decorations. a graph installed
under the current projection is not checked again

| Parameter | |
| :--- | :--- |
| `s` | the session whose type table receives the tables |
| `graph` | the graph to install |
| **returns** | ok, or a failure when a table disagrees with the graph or one cannot be published |

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `RECIPES` | `CaptureMode` | `0` |  |
| `MATERIALIZED` | `CaptureMode` | `1` |  |

