# Generic instantiation limits

Monomorphization expands a generic family into one concrete body per distinct
instantiation. The expansion terminates on its own for every family whose
instance set is finite: both engines that drive it dedup, so a family reached a
thousand times is expanded once, and mutual recursion between two generics
closes as soon as it repeats an instantiation it already holds.

A family whose instance set is **infinite** never repeats, so nothing closes it.
`fun f[T]() { f[*T](); }` names `f[*u32]`, which names `f[**u32]`, which names
`f[***u32]`: every instantiation is new, so the dedup never fires and the
worklist grows until the process dies. Before the limits below, that program hung
the compiler (and, reached through the constant-time re-validation drain, blew
the stack instead) with no diagnostic — [#2163].

Two quantities are bounded, because one does not imply the other.

## Depth: how far a chain of derived instantiations may run

`MAX_INSTANCE_DEPTH` / `MAX_CT_INSTANCE_DEPTH` = **32**.

An instantiation named by an ordinary body starts a chain at depth 0; one named
by an *instance* body continues its parent's chain at one greater. That is the
quantity that diverges, and bounding it is what distinguishes "this generic
family grows without end" from "this module is large". A cap on a module's total
instance count cannot make that distinction: a generic-heavy module legitimately
holds hundreds of unrelated instances at depth 0, and telling its author to go
find a recursion that does not exist is worse than no diagnostic at all.

**Measured, not guessed.** Instrumenting `enqueue_instance` to record the deepest
chain each module derives:

| build | deepest legitimate chain |
|---|---|
| `mach build .` (the compiler, 215 modules) | **5** |
| `mach test .` (the compiler + its test bodies, 220 modules) | **5** |
| `mach test .` in mach-std at pin `eff1e8e` (611 tests) | **5** |

The limit is **6.4x** the deepest chain either codebase derives. It matches the
bound [#2168] measured independently for the sema re-validation drain, whose
deepest legitimate chain is also 5.

## Breadth: how many distinct instances one module may hold

`MAX_INSTANCES` / `MAX_CT_INSTANCES` = **8192**.

A depth bound alone is not enough. A body that names *two* strictly larger
instances of itself —

```mach
fun f[T]() {
    f[*T]();
    f[[2]T]();
}
```

— stays inside any depth budget while emitting 2^depth instances. Both worklists
dedup by a linear scan, so that is quadratic on top: a hang long before the depth
cap is reached. This bounds what depth cannot see.

**Measured, not guessed.** The largest number of distinct instances a single
module holds:

| build | most instances in one module |
|---|---|
| the compiler | **197** |
| mach-std at pin `eff1e8e` | **67** |

The limit is **41x** the largest module either codebase produces.

## Where they are enforced

Both engines that expand a generic family enforce the same policy against their
own instance set:

- `me.lower.context.enqueue_instance` — the monomorphizer's worklist, which
  EMITS one body per instantiation.
- `fe.sema.context.record_instance` — the per-instance constant-time
  re-validation worklist ([#2157]), which RE-TYPES an instance body under its
  concrete arguments. It tracks a pruned subset (an instantiation whose every
  argument is a public scalar says nothing new), so its counts are lower, but the
  divergent shape is identical.

The two sets differ, so each limit is measured against its own engine; the
numbers above are the same because the shape of real code is the same.

## The diagnostic names the chain

A limit alone is nearly useless — it says something diverged, not what. Both
engines report the derivation chain that reached the refusal:

```
error: generic instantiation depth limit exceeded: a generic instantiates itself
at a growing type argument; chain: ... (28 more) -> dead[*****u32] ->
dead[******u32] -> dead[*******u32] -> dead[********u32] -> dead[*********u32]
```

Only the last few links are spelled, with a count of those elided: the growth is
visible in the tail, and spelling a whole chain at the depth limit runs to
thousands of characters, since its final argument carries one qualifier per link.

## What these limits do NOT cover

A `rec` or `uni` that contains **itself by value** is a different defect with a
different mechanism — a cycle in the field graph, not an unbounded chain of
distinct instantiations — and it is not caught here. `rec R { next: R; }` needs
no generics at all. See [#2355].

[#2157]: https://github.com/briar-systems/mach/issues/2157
[#2163]: https://github.com/briar-systems/mach/issues/2163
[#2168]: https://github.com/briar-systems/mach/issues/2168
[#2355]: https://github.com/briar-systems/mach/issues/2355
