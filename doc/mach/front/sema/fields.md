[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **fields**

# mach.front.sema.fields

field graphs: the field tables a set of types reaches, captured for another reader

`capture` walks every type the roots reach, preparing each record, union and tag it meets, and
copies its fields, its tag discriminator and its layout decorations into a graph. `install`
publishes a graph into the type table's current projection and refuses one that disagrees with
a table already there, so a module's typing or published surface brings the field tables it
was read with.

<sub>source: [`src/front/sema/fields.mach`](../../../../src/front/sema/fields.mach) · imported by 4 modules</sub>

<sub>`type_kind` is [`mach.type.kind`](../../type/kind.md) · `collections_vector` is `std.collections.vector` · `type_field` is [`mach.type.field`](../../type/field.md) · `session` is [`mach.session`](../../session.md) · `A` is `std.allocator` · `CaptureMode` is private · `fail` is [`mach.base.fail`](../../base/fail.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Node`](#node) | `rec` |  |
| [`Roots`](#roots) | `rec` |  |
| [`Graph`](#graph) | `rec` |  |

| On `Roots` | |
| :--- | :--- |
| [`capture`](#capture) | |

| On `Graph` | |
| :--- | :--- |
| [`init`](#init) · [`dnit`](#dnit) · [`install`](#install) | |

| Constants | |
| :--- | :--- |
| [`RECIPES`](#constants) · [`MATERIALIZED`](#constants) | |

---

## Node

```mach
pub rec Node {
    ty:         type_kind.TypeId;
    entries:    collections_vector.Vector[type_field.Entry];
    decoration: type_field.Decoration;
    # the declared discriminator of a tag nominal; TYPE_NIL on every other node
    disc: type_kind.TypeId;
}
```

## Roots

```mach
pub rec Roots {
    data: *type_kind.TypeId;
    len:  usize;
}
```

### capture

```mach
pub fun capture[T](s: *session.Session, a: *A.Allocator, roots: *Roots, count: usize, mode: CaptureMode, ctx: *T,
    prepare: fun(*T, type_kind.TypeId) err[fail.Fail]) res[Graph, fail.Fail]
```

## Graph

```mach
pub rec Graph {
    nodes:                collections_vector.Vector[Node];
    installed:            bool;
    installed_generation: u64;
}
```

### init

```mach
pub fun init(a: *A.Allocator) Graph
```

### dnit

```mach
pub fun dnit(graph: *Graph)
```

### install

```mach
pub fun install(s: *session.Session, graph: *Graph) err[fail.Fail]
```

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `RECIPES` | `CaptureMode` | `0` |  |
| `MATERIALIZED` | `CaptureMode` | `1` |  |

