[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **fields**

# mach.front.sema.fields

field graphs: the field tables a set of types reaches, captured for another reader

`capture` walks every type the roots reach, preparing each record, union and tag it meets, and
copies its fields, its tag discriminator and its layout decorations into a graph. `install`
publishes a graph into the type table's current projection and refuses one that disagrees with
a table already there, so a module's typing or published surface brings the field tables it
was read with.

Source: [`src/front/sema/fields.mach`](../../../../src/front/sema/fields.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.session`](../../session.md), [`mach.type.field`](../../type/field.md), [`mach.type.kind`](../../type/kind.md).

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
| [`init`](#init), [`dnit`](#dnit), [`install`](#install) | |

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

## Roots

```mach
pub rec Roots {
    data: *mach.type.kind.TypeId;
    len:  usize;
}
```

### capture

```mach
pub fun capture[T](s: *mach.session.Session, a: *std.allocator.Allocator, roots: *Roots, count: usize, mode: CaptureMode, ctx: *T,
    prepare: fun(*T, mach.type.kind.TypeId) err[mach.base.fail.Fail]) res[Graph, mach.base.fail.Fail]
```

## Graph

```mach
pub rec Graph {
    nodes:                std.collections.vector.Vector[Node];
    installed:            bool;
    installed_generation: u64;
}
```

### init

```mach
pub fun init(a: *std.allocator.Allocator) Graph
```

### dnit

```mach
pub fun dnit(graph: *Graph)
```

### install

```mach
pub fun install(s: *mach.session.Session, graph: *Graph) err[mach.base.fail.Fail]
```

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `RECIPES` | `CaptureMode` | `0` |  |
| `MATERIALIZED` | `CaptureMode` | `1` |  |

