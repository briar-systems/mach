[mach](../README.md) / [base](README.md) / **scc**

# mach.base.scc

strongly connected components of a directed graph over numbered nodes

<sub>source: [`src/base/scc.mach`](../../../src/base/scc.mach) · imported by 2 modules</sub>

<sub>`A` is `std.allocator` · `fail` is [`mach.base.fail`](fail.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Arc`](#arc) | `rec` |  |

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

### components

```mach
pub fun components(alloc: *A.Allocator, n: u32, arcs: *Arc, e: u32, comp: *u32) res[u32, fail.Fail]
```

Tarjan's strongly connected components over `n` nodes and `e` arcs, iteratively; `comp`
gets each node's component and the result is the component count

