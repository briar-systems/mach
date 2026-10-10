# mach.lang.fe.sema.growth

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

## val NONE

```mach
pub val NONE: u32 = 0xFFFFFFFF
```

## rec Cycle

```mach
pub rec Cycle;
```

a growing cycle: the templates on it, and whether it is refused, at the growing call of
its ungated part and the templates that call joins

## rec Analysis

```mach
pub rec Analysis;
```

nested: the walk met a body nested deeper than the stack lets it descend, at nested_at,
and left the rest of that body unwalked

## fun init

```mach
pub fun init(alloc: *A.Allocator) Analysis;
```

## fun dnit

```mach
pub fun dnit(an: *Analysis);
```

## fun cycle_of

```mach
pub fun cycle_of(an: *Analysis, decl: ast_id.DeclId) opt[u32];
```

the growing cycle declaration `decl` lies on, if any

## fun cycle_at

```mach
pub fun cycle_at(an: *Analysis, id: u32) *Cycle;
```

## fun fun_is_template

```mach
pub fun fun_is_template(a: *ast.Ast, d: *ast_decl.Decl) bool;
```

a generic, a function with a comptime value parameter, or one with a pack: emitted only
as its instances

## fun analyze

```mach
pub fun analyze(s: *session.Session, a: *ast.Ast, rr: *resolve.ResolveResult, source: str, own: session.ModuleId,
alloc: *A.Allocator) res[Analysis, fail.Fail];
```

the growing cycles of module `own`, parsed as `a` and resolved as `rr`

