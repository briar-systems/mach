[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **growth**

# mach.front.sema.growth

which of a module's templates can instantiate themselves without end

the graph is over declarations: a node is one parameter of one template of the module, a
type parameter, a comptime value parameter or the pack, and an edge says that parameter
flows into an argument of a template a call in the body names. an edge grows when the
argument derives from the parameter without being it: `f[*T]` from `T`, `f(n + 1)` from
`n`, a pack forwarded with more elements than it had, or an element derived from one of
the pack's. a cycle with a growing edge mints a fresh instance at every trip around it, so
only a comptime gate on it can stop it. a call inside a comptime arm or an `$each` is gated;
one outside both is reached by every instance of its caller. a cycle all of whose edges are
ungated and one of which grows never stops, and is refused before any of it is expanded.
every other instantiation is finite: outside a growing cycle by construction, and inside
one only as far as its gates let it run, which is as far as the program asks. a call to
another module's template cannot come back, since modules do not import each other in a
cycle

Source: [`src/front/sema/growth.mach`](../../../../src/front/sema/growth.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.base.intern`](../../base/intern.md), [`mach.base.source`](../../base/source.md), [`mach.front.ast`](../ast.md), [`mach.front.ast.decl`](../ast/decl.md), [`mach.front.ast.id`](../ast/id.md), [`mach.front.resolve`](../resolve.md), [`mach.session`](../../session.md), [`mach.session.module`](../../session/module.md).

| Type | | |
| :--- | :--- | :--- |
| [`Cycle`](#cycle) | `rec` | a growing cycle: the templates on it, and whether it is refused, at the growing call of its ungated part and the templates that call joins |
| [`Analysis`](#analysis) | `rec` | nested: the walk met a body nested deeper than the stack lets it descend, at nested_at, and left the rest of that body unwalked |

| On `Analysis` | |
| :--- | :--- |
| [`init`](#init) | an empty analysis, with no cycles |
| [`dnit`](#dnit) | release an analysis; a nil one, or one never made, holds nothing |
| [`cycle_of`](#cycle_of) | the growing cycle declaration `decl` lies on, if any |
| [`cycle_at`](#cycle_at) | the growing cycle `id`, as `cycle_of` names it |
| [`analyze`](#analyze) | the growing cycles of module `own`, parsed as `a` and resolved as `rr` |

| Functions | |
| :--- | :--- |
| [`fun_is_template`](#fun_is_template) | a generic, a function with a comptime value parameter, or one with a pack: emitted only as its instances |

| Constants | |
| :--- | :--- |
| [`NONE`](#constants) | |

---

## Cycle

```mach
pub rec Cycle {
    members: mach.base.intern.StrId;
    refused: bool;
    site:    mach.base.source.Span;
    caller:  mach.base.intern.StrId;
    callee:  mach.base.intern.StrId;
}
```

a growing cycle: the templates on it, and whether it is refused, at the growing call of
its ungated part and the templates that call joins

## Analysis

```mach
pub rec Analysis {
    alloc:     *std.allocator.Allocator;
    module:    mach.session.module.ModuleId;
    cycle_of:  std.collections.map.Map[u32, u32];
    cycles:    std.collections.vector.Vector[Cycle];
    nested:    bool;
    nested_at: mach.base.source.Span;
}
```

nested: the walk met a body nested deeper than the stack lets it descend, at nested_at,
and left the rest of that body unwalked

### init

```mach
pub fun init(alloc: *std.allocator.Allocator) Analysis
```

an empty analysis, with no cycles

| Parameter | |
| :--- | :--- |
| `alloc` | the allocator its tables are held in |
| **returns** | the analysis, released with `dnit` |

### dnit

```mach
pub fun dnit(an: *Analysis)
```

release an analysis; a nil one, or one never made, holds nothing

### cycle_of

```mach
pub fun cycle_of(an: *Analysis, decl: mach.front.ast.id.DeclId) opt[u32]
```

the growing cycle declaration `decl` lies on, if any

### cycle_at

```mach
pub fun cycle_at(an: *Analysis, id: u32) *Cycle
```

the growing cycle `id`, as `cycle_of` names it

| Parameter | |
| :--- | :--- |
| `an` | the analysis holding the cycle |
| `id` | the cycle's index, which must be one `cycle_of` gave |
| **returns** | the cycle, owned by the analysis |

### analyze

```mach
pub fun analyze(s: *mach.session.Session, a: *mach.front.ast.Ast, rr: *mach.front.resolve.ResolveResult, source: str, own: mach.session.module.ModuleId,
    alloc: *std.allocator.Allocator) res[Analysis, mach.base.fail.Fail]
```

the growing cycles of module `own`, parsed as `a` and resolved as `rr`

## Functions

### fun_is_template

```mach
pub fun fun_is_template(a: *mach.front.ast.Ast, d: *mach.front.ast.decl.Decl) bool
```

a generic, a function with a comptime value parameter, or one with a pack: emitted only
as its instances

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `NONE` | `u32` | `0xFFFFFFFF` |  |

