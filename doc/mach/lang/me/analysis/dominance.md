# mach.lang.me.analysis.dominance

the dominator tree of a function, read from the edges its terminators name:
each block's predecessors, the reverse postorder from the entry, immediate
dominators, the tree's children and a numbering of the tree that makes a
dominance query an interval test, and on request the dominance frontiers

## val NONE

```mach
pub val NONE: u32 = 0xFFFFFFFF
```

the reverse postorder index of a block no path from the entry reaches

## rec Dominance

```mach
pub rec Dominance;
```

## fun init

```mach
pub fun init(fn: *me_ir.Function, alloc: *A.Allocator) res[Dominance, fail.Fail];
```

the dominator tree of `fn` as it stands, its storage taken from `alloc`

## fun dnit

```mach
pub fun dnit(d: *Dominance);
```

## fun block_is_reachable

```mach
pub fun block_is_reachable(d: *Dominance, b: ir_id.BlockId) bool;
```

whether a path from the entry reaches `b`

## fun dominates

```mach
pub fun dominates(d: *Dominance, a: ir_id.BlockId, b: ir_id.BlockId) bool;
```

whether every path from the entry to `b` passes through `a`; a block
dominates itself, and no block dominates or is dominated by an unreachable one

## fun frontiers_build

```mach
pub fun frontiers_build(d: *Dominance) err[fail.Fail];
```

the dominance frontiers, built once: a join's frontier walk climbs from each
reachable predecessor to the join's immediate dominator

