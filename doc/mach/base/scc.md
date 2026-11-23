[mach](../README.md) / [base](README.md) / **scc**

# mach.base.scc

strongly connected components of a directed graph over numbered nodes

Source: [`src/base/scc.mach`](../../../src/base/scc.mach)

Uses types from [`mach.base.fail`](fail.md).

| Type | | |
| :--- | :--- | :--- |
| [`Arc`](#arc) | `rec` | a directed arc between two numbered nodes |

| On `Arc` | |
| :--- | :--- |
| [`components`](#components) | Tarjan's strongly connected components over `n` nodes and `e` arcs, iteratively; `comp` gets each node's component and the result is the component count |

---

## Arc

```mach
pub rec Arc {
    from: u32;
    to:   u32;
}
```

a directed arc between two numbered nodes

| Field | Type | |
| :--- | :--- | :--- |
| `from` | `u32` | the node it leaves |
| `to` | `u32` | the node it enters |

### components

```mach
pub fun components(alloc: *std.allocator.Allocator, n: u32, arcs: *Arc, e: u32, comp: *u32) res[u32, mach.base.fail.Fail]
```

Tarjan's strongly connected components over `n` nodes and `e` arcs, iteratively; `comp`
gets each node's component and the result is the component count

