# mach.lang.scc

strongly connected components of a directed graph over numbered nodes

## rec Arc

```mach
pub rec Arc;
```

## fun components

```mach
pub fun components(alloc: *A.Allocator, n: u32, arcs: *Arc, e: u32, comp: *u32) res[u32, fail.Fail];
```

Tarjan's strongly connected components over `n` nodes and `e` arcs, iteratively; `comp`
gets each node's component and the result is the component count

