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

<sub>source: [`src/front/sema/growth.mach`](../../../../src/front/sema/growth.mach) · imported by 5 modules</sub>

<sub>`intern` is [`mach.base.intern`](../../base/intern.md) · `base_source` is [`mach.base.source`](../../base/source.md) · `A` is `std.allocator` · `module` is [`mach.session.module`](../../session/module.md) · `map` is `std.collections.map` · `collections_vector` is `std.collections.vector` · `ast_id` is [`mach.front.ast.id`](../ast/id.md) · `session` is [`mach.session`](../../session.md) · `ast` is [`mach.front.ast`](../ast.md) · `resolve` is [`mach.front.resolve`](../resolve.md) · `fail` is [`mach.base.fail`](../../base/fail.md) · `ast_decl` is [`mach.front.ast.decl`](../ast/decl.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Cycle`](#cycle) | `rec` | a growing cycle: the templates on it, and whether it is refused, at the growing call of its ungated part and the templates that call joins |
| [`Analysis`](#analysis) | `rec` | nested: the walk met a body nested deeper than the stack lets it descend, at nested_at, and left the rest of that body unwalked |

| On `Analysis` | |
| :--- | :--- |
| [`init`](#init) | |
| [`dnit`](#dnit) | release an analysis; a nil one, or one never made, holds nothing |
| [`cycle_of`](#cycle_of) | the growing cycle declaration `decl` lies on, if any |
| [`cycle_at`](#cycle_at) | |
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
    members: intern.StrId;
    refused: bool;
    site:    base_source.Span;
    caller:  intern.StrId;
    callee:  intern.StrId;
}
```

a growing cycle: the templates on it, and whether it is refused, at the growing call of
its ungated part and the templates that call joins

## Analysis

```mach
pub rec Analysis {
    alloc:     *A.Allocator;
    module:    module.ModuleId;
    cycle_of:  map.Map[u32, u32];
    cycles:    collections_vector.Vector[Cycle];
    nested:    bool;
    nested_at: base_source.Span;
}
```

nested: the walk met a body nested deeper than the stack lets it descend, at nested_at,
and left the rest of that body unwalked

### init

```mach
pub fun init(alloc: *A.Allocator) Analysis
```

### dnit

```mach
pub fun dnit(an: *Analysis)
```

release an analysis; a nil one, or one never made, holds nothing

### cycle_of

```mach
pub fun cycle_of(an: *Analysis, decl: ast_id.DeclId) opt[u32]
```

the growing cycle declaration `decl` lies on, if any

### cycle_at

```mach
pub fun cycle_at(an: *Analysis, id: u32) *Cycle
```

### analyze

```mach
pub fun analyze(s: *session.Session, a: *ast.Ast, rr: *resolve.ResolveResult, source: str, own: module.ModuleId,
    alloc: *A.Allocator) res[Analysis, fail.Fail]
```

the growing cycles of module `own`, parsed as `a` and resolved as `rr`

## Functions

### fun_is_template

```mach
pub fun fun_is_template(a: *ast.Ast, d: *ast_decl.Decl) bool
```

a generic, a function with a comptime value parameter, or one with a pack: emitted only
as its instances

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `NONE` | `u32` | `0xFFFFFFFF` |  |

