# mach.lang.me.analysis

the analyses a pass reads about a function, built on request and kept for
the passes after it. an analysis is served for the function as it stands; a
pass that changes a function and then asks about it again drops what it
changed with `del` first. every pass declares the kinds it preserves, and
the schedule drops the rest with `keep` after the pass runs, so the next
pass is served an analysis only where nothing since has made it stale

## def KindSet

```mach
pub def KindSet: u8
```

a set of analysis kinds

## val NONE

```mach
pub val NONE: KindSet = 0
```

## val DOMINANCE

```mach
pub val DOMINANCE: KindSet = 1
```

the dominator tree, its order and its frontiers: read from the edges alone,
so a pass that adds, removes or redirects no edge and no block preserves it

## val LOOPS

```mach
pub val LOOPS: KindSet = 2
```

the loops over the dominator tree: read from instructions too, so only a
pass that changes nothing preserves it, and it never outlives its tree

## val ALL

```mach
pub val ALL:   KindSet = 3
```

## rec Cache

```mach
pub rec Cache;
```

each kind lives in a region of its own, so dropping a kind releases its
storage at once. a dropped analysis stays readable until the next `keep`,
so a pass may finish reading one it asked `del` to drop

## fun init

```mach
pub fun init(c: *Cache, backing: *A.Allocator) err[fail.Fail];
```

an empty cache whose regions grow from `backing`; it must not move once
initialized

## fun dnit

```mach
pub fun dnit(c: *Cache);
```

releases in the reverse of the order a first request takes storage, so a
backing that reclaims only its last allocation reclaims all of it

## fun dominance_of

```mach
pub fun dominance_of(c: *Cache, m: *me_ir.Module, fn: *me_ir.Function) res[*dominance.Dominance, fail.Fail];
```

the dominator tree of `fn`, a function of `m`

## fun frontiers_of

```mach
pub fun frontiers_of(c: *Cache, m: *me_ir.Module, fn: *me_ir.Function) res[*dominance.Dominance, fail.Fail];
```

the dominator tree of `fn` with its dominance frontiers

## fun loops_of

```mach
pub fun loops_of(c: *Cache, m: *me_ir.Module, fn: *me_ir.Function) res[*loops.LoopAnalysis, fail.Fail];
```

the loops of `fn`, a function of `m`. a pass may update the analysis as it
moves instructions, which the loops kind's declaration already accounts for

## fun del

```mach
pub fun del(c: *Cache, m: *me_ir.Module, fn: *me_ir.Function);
```

drops every analysis of `fn`, which the calling pass changed

## fun keep

```mach
pub fun keep(c: *Cache, preserved: KindSet);
```

drops every analysis of a kind outside `preserved`, for every function

## fun each_innermost_first

```mach
pub fun each_innermost_first[T](c: *Cache, m: *me_ir.Module, fn: *me_ir.Function, alloc: *A.Allocator, ctx: *T,
per_loop: fun(*T, *loops.LoopAnalysis, u32, *A.Allocator) res[bool, fail.Fail]) res[bool, fail.Fail];
```

`per_loop` run on each loop of `fn` deepest first, against the loops the
cache serves for it; whether any run changed the function

