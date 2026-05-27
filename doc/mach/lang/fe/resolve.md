# mach.lang.fe.resolve

Per-module name resolution. Walks one parsed [`Ast`](ast.md#ast) in
two passes — *collect* (registers every binding decl reachable from
module scope) and *bind* (walks every decl, stmt, expr, and type
looking up references against the scope chain). Inputs come from the
[driver](../driver.md), which has already resolved every dependency in
topological order; outputs are a [`ResolveResult`](#resolveresult) that
[sema](sema.md) consumes.

Errors do not halt the pass: an unresolved name becomes
[`SYMBOL_NIL`](#constants) and an error-severity diagnostic is pushed
into [`session.diags`](../session.md#session). Downstream phases treat
`SYMBOL_NIL` as poison that absorbs further operations without
cascading diagnostics.

## Types

### `SymbolId`

```mach
pub def SymbolId: u32;
```

Stable handle into a [`ResolveResult.symbols`](#resolveresult) array.
Identifiers in [`0, pub_count)`](#resolveresult) are the pub-flagged
symbols, in original declaration order; identifiers in
[`pub_count, symbol_count)`](#resolveresult) are private.

### `SymKind`

```mach
pub def SymKind: u8;
```

Discriminator for [`Symbol.kind`](#symbol). See [Constants](#constants)
for the enumerated values.

### `Symbol`

```mach
pub rec Symbol {
    kind:   SymKind;
    flags:  u8;
    name:   intern.StrId;
    decl:   id.DeclId;
    slot:   u32;
    origin: sess.ModuleId;
}
```

A resolved symbol.

| Field  | Type                                          | Description                                                |
|--------|-----------------------------------------------|------------------------------------------------------------|
| kind   | [`SymKind`](#symkind)                         | One of [`SYM_*`](#constants).                              |
| flags  | `u8`                                          | Bitfield of [`SYM_FLAG_*`](#constants) (currently only `SYM_FLAG_PUB`). |
| name   | [`intern.StrId`](../intern.md#strid)          | Interned identifier ([`STR_NIL`](../intern.md#constants) for anonymous symbols). |
| decl   | [`id.DeclId`](ast/id.md#declid)            | Originating DeclId in `origin`'s [`Ast`](ast.md#ast); [`DECL_NIL`](ast/id.md#constants) when synthetic. |
| slot   | `u32`                                         | Kind-dependent extra payload (see below).                  |
| origin | [`sess.ModuleId`](../session.md#moduleid)     | ModuleId of the module that owns `decl` (this module for locally-declared symbols; the dependency for `SYM_USE` / `SYM_IMPORTED`). |

`slot` interpretation by kind:

| Kind             | `slot` holds                                    |
|------------------|-------------------------------------------------|
| `SYM_PARAM`      | Parameter index within the enclosing function.  |
| `SYM_GENERIC`    | Generic-parameter index within the owning decl. |
| `SYM_USE`        | Target [`ModuleId`](../session.md#moduleid) of the imported module. |
| (other)          | 0                                               |

### `PublicSymbol`

```mach
pub rec PublicSymbol {
    kind:   SymKind;
    name:   intern.StrId;
    decl:   id.DeclId;
    origin: sess.ModuleId;
}
```

A snapshot of one pub symbol, copied by the [driver](../driver.md)
into a dependent module's [`ResolveDeps`](#resolvedeps) so resolve can
bind cross-module references without holding a reference back to the
producer module's full `ResolveResult`.

| Field  | Type                                          | Description                                       |
|--------|-----------------------------------------------|---------------------------------------------------|
| kind   | [`SymKind`](#symkind)                         | One of [`SYM_*`](#constants).                     |
| name   | [`intern.StrId`](../intern.md#strid)          | Interned identifier.                              |
| decl   | [`id.DeclId`](ast/id.md#declid)            | DeclId in the owning module's Ast.                |
| origin | [`sess.ModuleId`](../session.md#moduleid)     | ModuleId of the owning module.                    |

### `ModuleExports`

```mach
pub rec ModuleExports {
    path:    intern.StrId;
    module:  sess.ModuleId;
    symbols: *PublicSymbol;
    count:   u32;
}
```

The public surface of one module, indexed by interned FQN path text.
Produced by the [driver](../driver.md) from a previously-resolved
module's [`ResolveResult`](#resolveresult).

| Field   | Type                                          | Description                                          |
|---------|-----------------------------------------------|------------------------------------------------------|
| path    | [`intern.StrId`](../intern.md#strid)          | Interned FQN of the module (e.g. `"std.types.bool"`).|
| module  | [`sess.ModuleId`](../session.md#moduleid)     | ModuleId of the module.                              |
| symbols | [`*PublicSymbol`](#publicsymbol)              | Array of pub symbols.                                |
| count   | `u32`                                         | Number of entries in `symbols`.                      |

### `ResolveDeps`

```mach
pub rec ResolveDeps {
    entries: *ModuleExports;
    count:   u32;
}
```

Every dependency's exports, supplied by the [driver](../driver.md).
Resolve looks these up by interned path when handling `use` decls.

| Field   | Type                                          | Description                          |
|---------|-----------------------------------------------|--------------------------------------|
| entries | [`*ModuleExports`](#moduleexports)            | Array of [`ModuleExports`](#moduleexports), one per loaded dep. |
| count   | `u32`                                         | Number of entries.                   |

### `ResolveResult`

```mach
pub rec ResolveResult {
    alloc:         *Allocator;
    symbols:       *Symbol;
    symbol_count:  u32;
    pub_count:     u32;
    expr_resolved: *SymbolId;
    type_resolved: *SymbolId;
    decl_symbol:   *SymbolId;
}
```

The contract resolve produces for [sema](sema.md) and the driver's
export step.

| Field         | Type                                          | Description                                                                |
|---------------|-----------------------------------------------|----------------------------------------------------------------------------|
| alloc         | `*Allocator`                                  | Allocator backing every owned array on the result.                         |
| symbols       | [`*Symbol`](#symbol)                          | Universe of symbols; pub symbols sorted to indices `[0, pub_count)`.       |
| symbol_count  | `u32`                                         | Total number of symbols.                                                   |
| pub_count     | `u32`                                         | Number of pub symbols at the front of `symbols`.                           |
| expr_resolved | [`*SymbolId`](#symbolid)                      | Parallel to [`ast.exprs`](ast.md#ast); [`SYMBOL_NIL`](#constants) outside binding sites. |
| type_resolved | [`*SymbolId`](#symbolid)                      | Parallel to [`ast.types`](ast.md#ast); [`SYMBOL_NIL`](#constants) outside binding sites. |
| decl_symbol   | [`*SymbolId`](#symbolid)                      | Parallel to [`ast.decls`](ast.md#ast); [`SYMBOL_NIL`](#constants) for non-binding decls. |

## Constants

```mach
pub val SYMBOL_NIL: SymbolId = 0xFFFFFFFF;
```

Absent-symbol sentinel. Stored in
[`expr_resolved`](#resolveresult) / [`type_resolved`](#resolveresult)
/ [`decl_symbol`](#resolveresult) at positions where no binding
applies; also returned by lookup helpers when no matching symbol
exists.

```mach
pub val SYM_FUN:      SymKind = 0;
pub val SYM_REC:      SymKind = 1;
pub val SYM_DEF:      SymKind = 2;
pub val SYM_VAL:      SymKind = 3;
pub val SYM_VAR:      SymKind = 4;
pub val SYM_PARAM:    SymKind = 5;
pub val SYM_GENERIC:  SymKind = 6;
pub val SYM_USE:      SymKind = 7;
pub val SYM_IMPORTED: SymKind = 8;
pub val SYM_TEST:     SymKind = 9;
```

[`SymKind`](#symkind) values.

| Constant         | Value | Meaning                                                                  |
|------------------|-------|--------------------------------------------------------------------------|
| `SYM_FUN`        | 0     | Function declaration.                                                    |
| `SYM_REC`        | 1     | Nominal record declaration.                                              |
| `SYM_DEF`        | 2     | Type alias.                                                              |
| `SYM_VAL`        | 3     | Immutable binding (`val`).                                               |
| `SYM_VAR`        | 4     | Mutable binding (`var`).                                                 |
| `SYM_PARAM`      | 5     | Function parameter (per-fun scope).                                      |
| `SYM_GENERIC`    | 6     | Generic parameter (per-decl scope).                                      |
| `SYM_USE`        | 7     | `use foo: bar.baz;` alias for an imported module.                        |
| `SYM_IMPORTED`   | 8     | Symbol imported from another module via a bare `use bar.baz;`.           |
| `SYM_TEST`       | 9     | `test "..." { ... }` declaration.                                        |

```mach
pub val SYM_FLAG_PUB: u8 = 1;
```

Symbol-flag bit set when the originating decl carries
[`DECL_FLAG_PUB`](ast/decl.md#constants) at module scope.

```mach
val INITIAL_SCOPE_ENTRY_CAP: u32 = 8;
val INITIAL_SCOPES_CAP:      u32 = 8;
val INITIAL_SYMBOLS_CAP:     u32 = 32;
```

Starting capacities for the scope entries, scopes array, and symbol
table respectively. Each doubles on overflow.

## Functions

### `resolve`

```mach
pub fun resolve(
    s:          *sess.Session,
    a:          *ast.Ast,
    deps:       *ResolveDeps,
    ctx:        *comptime.ComptimeCtx,
    own_module: sess.ModuleId,
) Result[ResolveResult, str]
```

Resolves one module against its already-resolved dependencies. Runs
[init_context](#internal-helpers) → [collect](#pass-1-collect) →
[bind](#pass-2-bind) → [build_result](#internal-helpers); on any
allocation failure, tears down the partial context and propagates the
error.

| Param      | Type                                                  | Description                                                |
|------------|-------------------------------------------------------|------------------------------------------------------------|
| s          | [`*sess.Session`](../session.md#session)              | Shared session (allocator, sources, diagnostics, interner).|
| a          | [`*ast.Ast`](ast.md#ast)                           | Parsed AST for the module being resolved.                  |
| deps       | [`*ResolveDeps`](#resolvedeps)                        | Exports of every dependency, populated by the driver.      |
| ctx        | [`*comptime.ComptimeCtx`](comptime.md#comptimectx)    | Comptime context, pre-seeded with target params and imports' pub consts; resolve mutates it as it discovers comptime-evaluable val decls. |
| own_module | [`sess.ModuleId`](../session.md#moduleid)             | Project-wide ModuleId of the module being resolved.        |

Returns the populated [`ResolveResult`](#resolveresult), or an
allocation error.

### `dnit_result`

```mach
pub fun dnit_result(r: *ResolveResult)
```

Releases every array owned by the result and zeroes its fields. `nil`
is a no-op. Called by the [driver](../driver.md) when a module's
resolve result is no longer needed.

| Param | Type                                  | Description                          |
|-------|---------------------------------------|--------------------------------------|
| r     | [`*ResolveResult`](#resolveresult)    | Result to tear down. `nil` is a no-op. |

## Pass 1: collect

Walks every reachable decl in source order and registers a Symbol for
each binding. Comptime `$if` predicates are evaluated against
[`ctx`](comptime.md#comptimectx) during this pass so only the active
branch contributes symbols. `use` decls inject either a single
[`SYM_USE`](#constants) alias (for `use foo: bar.baz;`) or a flat
slice of imported [`SYM_IMPORTED`](#constants) symbols (for the
bare-import form) into the module scope.

[`collect_one_decl`](#internal-helpers) dispatches per
[`DeclKind`](ast/decl.md#declkind):

| `DECL_KIND_*`        | Action                                                                                |
|----------------------|---------------------------------------------------------------------------------------|
| `USE`                | [`collect_use`](#internal-helpers): alias or bulk-import.                             |
| `FWD`                | [`collect_fwd`](#internal-helpers): deferred to bind — re-export needs the full module scope in place first. |
| `FUN`                | Declare a `SYM_FUN` in the enclosing scope.                                           |
| `REC`                | Declare a `SYM_REC` in the enclosing scope.                                           |
| `VAL`                | Declare a `SYM_VAL`. Also calls [`try_register_comptime_const`](#internal-helpers) to make the initializer available to subsequent `$if`. |
| `VAR`                | Declare a `SYM_VAR`.                                                                  |
| `DEF`                | Declare a `SYM_DEF`.                                                                  |
| `TEST`               | Declare a `SYM_TEST` (anonymous; tracked for the test runner).                        |
| `COMPTIME_IF`        | [`collect_comptime_if`](#internal-helpers): evaluate each arm's cond via [`comptime_branch_active`](#internal-helpers); recurse into the first matching arm's body. |
| `COMPTIME_ATTR`      | Skipped during collect (no name binding).                                             |
| `ERROR`              | Skipped.                                                                              |

## Pass 2: bind

Walks every decl, stmt, expr, and type. For each name reference,
resolves it against the [scope chain](#scope-chain) and writes the
resolved [`SymbolId`](#symbolid) into the parallel
[`expr_resolved`](#resolveresult) / [`type_resolved`](#resolveresult)
arrays. Unresolved names produce a [`SEVERITY_ERROR`](../diagnostic.md#constants)
diagnostic and stay at [`SYMBOL_NIL`](#constants).

Function and record decls open generic and parameter scopes for the
duration of their bodies:

| Bind handler           | Scope opened                                                                |
|------------------------|-----------------------------------------------------------------------------|
| [`bind_fun`](#internal-helpers) | Generic scope → param scope (child of generic) → body sees both. |
| [`bind_rec`](#internal-helpers) | Generic scope only. Field types see the generic params.          |

[`bind_local_decl`](#internal-helpers) binds a local
[`val`](ast/decl.md#declkind)/[`var`](ast/decl.md#declkind)'s
type and initializer **before** declaring the new name. This implements
let-not-letrec semantics — `val x: i32 = x + 1;` resolves `x` against
the outer scope, not against the (not-yet-declared) shadow.

[`bind_expr`](#internal-helpers) is exhaustive over
[`ExprKind`](ast/expr.md#exprkind) except
[`EXPR_KIND_MEMBER`](ast/expr.md#exprkind), which is left
intentionally at [`SYMBOL_NIL`](#constants) because member resolution
requires the object's type — sema's job.

## Scope chain

```
Module scope (root, no parent)
  └─ Generic scope (per fun/rec)
       └─ Param scope (per fun)
            └─ Block scope (per `{...}` body)
                 └─ Block scope (nested)
                      ...
```

[`open_scope`](#internal-helpers) appends a [`Scope`](#scope-records)
to `rc.scopes` with the supplied parent and returns its `ScopeId`. The
current head lives at [`rc.current_scope`](#scope-records); each
construct that opens a scope saves the old head, swaps in the new
one, and restores the old head on exit.

**Name lookup** uses two predicates:

- [`scope_lookup_local`](#internal-helpers) — checks only the named
  scope; used by [`declare`](#internal-helpers) to detect duplicates.
- [`scope_lookup_chain`](#internal-helpers) — walks parents until it
  hits [`SCOPE_NIL`](#scope-records); the canonical "resolve a name in
  this code position" call.

**Shadowing**: a duplicate name *in the same scope* is rejected by
[`declare`](#internal-helpers) with `"duplicate definition in this
scope"`. Cross-scope shadowing (a block declares a name that an
outer scope also has) is allowed implicitly because the duplicate
check is local-only.

## Use decls

`use foo: bar.baz;` (with alias):

1. Intern the dotted path span.
2. [`find_module_exports`](#internal-helpers) over [`deps`](#resolvedeps)
   by path.
3. If found, declare a single [`SYM_USE`](#constants) named `foo`,
   with `slot = target ModuleId`.
4. Write the new [`SymbolId`](#symbolid) into [`decl_symbol`](#resolveresult)
   for the use decl.

`use bar.baz;` (bare):

1. Intern the dotted path span.
2. [`find_module_exports`](#internal-helpers) over [`deps`](#resolvedeps).
3. If found, iterate [`mx.symbols`](#moduleexports) (which contains
   only pub entries) and declare one [`SYM_IMPORTED`](#constants) per
   entry in the module scope.
4. The use decl's [`decl_symbol`](#resolveresult) entry stays at
   [`SYMBOL_NIL`](#constants) (no single symbol; the import expanded
   into many).

Imports for which the dep is not present in [`deps`](#resolvedeps)
emit `"imported module is not in the dep set"` and skip registration.

## Re-export (`fwd`)

`fwd <identifier>;` ([`DECL_KIND_FWD`](ast/decl.md#declkind)) is
its own declaration form — re-exporting always, with no `pub` flag.
The identifier follows standard scope rules: a bare name resolves
against the scope chain (hitting any [`SYM_IMPORTED`](#constants)
introduced by a bare `use`, or a local decl), a dotted form hops
module aliases declared by `use foo: bar.baz;`. Whatever
[`Symbol`](#symbol) the identifier resolves to is copied into the
pub-flagged half of this module's
[`ResolveResult.symbols`](#resolveresult) with [`SYM_FLAG_PUB`](#constants)
set; `origin` and `decl` continue to point at the originating module
so cross-module clients reach the same canonical decl.

[`collect_fwd`](#internal-helpers) is a no-op during collect — the
re-export is handled in [bind](#pass-2-bind), once the module scope
(including every sibling `use`) is fully populated. An identifier
that resolves to nothing emits `"forwarded symbol not in scope"` and
the `fwd` decl contributes no pub symbol.

The driver does not special-case `fwd` during DFS load — the
forwarded symbol is only reachable because some sibling `use` in the
same module already brought its source module into the dep set. This
is by design: `fwd` is the concrete handle for re-exporting symbols
whose backing module was selected at comptime (typical shape: a
`$if`-guarded `use impl: this.os.linux;` followed by `fwd impl.FOO;`).

## Cross-module dedup

`lookup_in_module(m, name)` returns a [`SymbolId`](#symbolid) for
`name` exported by module `m`. To avoid bloating the symbol table
with one [`SYM_IMPORTED`](#constants) per use-site, a
[`Map[u64, SymbolId]`](../intern.md) on [`ResolveContext.imported_cache`](#scope-records)
caches by `(origin << 32) | decl`. The same `(origin, decl)` pair
always resolves to the same [`SymbolId`](#symbolid).

## Internal helpers

File-private; listed for reference.

| Function                          | Role                                                                       |
|-----------------------------------|----------------------------------------------------------------------------|
| `init_context`                    | Allocates parallel arrays sized to `ast.expr_len` / `type_len` / `decl_len` and the dedup map. |
| `dnit_context`                    | Reverse of `init_context`. Used by `resolve` on error paths and folded into `build_result` on success. |
| `fill_nil`                        | Fills a `*SymbolId` array of `len` slots with [`SYMBOL_NIL`](#constants).  |
| `open_scope`                      | Append a [`Scope`](#scope-records); return its `ScopeId`.                  |
| `scope_add`                       | Append a `(name, sym)` [`ScopeEntry`](#scope-records) to the named scope.  |
| `scope_lookup_local`              | Linear scan over one scope's entries.                                      |
| `scope_lookup_chain`              | Walk parents; first hit wins.                                              |
| `add_symbol`                      | Append a [`Symbol`](#symbol) to `rc.symbols`; return its [`SymbolId`](#symbolid). |
| `declare`                         | `add_symbol` + `scope_add` with duplicate detection.                       |
| `collect_decl_list`               | Loop calling `collect_one_decl` for every [`DeclId`](ast/id.md#declid) in `[start, start+len)`. |
| `collect_one_decl`                | Dispatch per [`DeclKind`](ast/decl.md#declkind) during collect.         |
| `declare_decl`                    | Helper used by per-kind collect branches.                                  |
| `symbol_flags_from`               | Maps [`DECL_FLAG_*`](ast/decl.md#constants) → [`SYM_FLAG_*`](#constants); pub only set at module scope. |
| `try_register_comptime_const`     | Attempts to evaluate a val initializer and register it as a [`NamedConst`](comptime.md#namedconst). Silent on failure (non-comptime vals are normal). |
| `collect_comptime_if`             | Walks branches; calls `comptime_branch_active`; recurses into the active arm. |
| `comptime_branch_active`          | Evaluates `br.cond` via [`comptime.eval`](comptime.md#eval); on eval error or non-bool result emits a diagnostic and returns `ok(false)`. |
| `collect_use`                     | `use` decl handling (alias / bare forms).                                  |
| `collect_fwd`                     | No-op during collect; the `fwd` re-export is resolved in bind once the module scope is complete. |
| `find_module_exports`             | Linear scan of [`deps.entries`](#resolvedeps) by interned path.            |
| `bind_decl_list`                  | Loop calling `bind_one_decl` over a decl id slice.                         |
| `bind_one_decl`                   | Dispatch per [`DeclKind`](ast/decl.md#declkind) during bind.            |
| `bind_fwd`                        | Resolves a [`DeclFwd.target`](ast/decl.md#declfwd) identifier against the scope chain and copies the resolved [`Symbol`](#symbol) into the pub half with [`SYM_FLAG_PUB`](#constants). |
| `bind_comptime_if`                | Stmt-and-decl-scope `$if` handler; takes `(branches_start, branches_len, at_decl_scope)`. |
| `bind_fun`                        | Opens generic + param scopes; binds return type and body.                  |
| `bind_rec`                        | Opens generic scope; binds field types.                                    |
| `bind_stmt_list`                  | Loop calling `bind_stmt` over a stmt id slice.                             |
| `bind_stmt`                       | Dispatch per [`StmtKind`](ast/stmt.md#stmtkind).                        |
| `bind_block`                      | Opens a new block scope; binds the contained stmts; closes the scope.      |
| `bind_local_decl`                 | Binds local `val`/`var` type + init; **then** declares the symbol.         |
| `if_val_kind`                     | Returns [`SYM_VAL`](#constants) or [`SYM_VAR`](#constants) per the bool.   |
| `bind_expr`                       | Dispatch per [`ExprKind`](ast/expr.md#exprkind).                        |
| `bind_expr_list`                  | Loop calling `bind_expr` over an expr id slice.                            |
| `bind_ident`                      | Interns the ident span and `scope_lookup_chain`s; writes the result to [`expr_resolved`](#resolveresult); emits `"unresolved identifier"` on miss. |
| `bind_type`                       | Dispatch per [`TypeKind`](ast/type.md#typekind).                        |
| `resolve_type_path`               | Walks a dotted [`TypeNamed.name`](ast/type.md#typenamed) span left-to-right, hopping module aliases via [`SYM_USE`](#constants) → [`ModuleExports`](#moduleexports) until the leaf segment is resolved. |
| `lookup_in_module`                | Looks up `name` among a module's pub exports; caches the resulting [`SYM_IMPORTED`](#constants) via [`imported_cache`](#scope-records). |
| `build_result`                    | Allocates `out_syms`, partitions pub-first, transfers ownership of parallel arrays, dnit's the context. |

## Scope records

The following are file-private but documented because their shape
informs the algorithm above.

```mach
def ScopeId: u32;
val SCOPE_NIL: ScopeId = 0xFFFFFFFF;

rec ScopeEntry {
    name: intern.StrId;
    sym:  SymbolId;
}

rec Scope {
    parent:  ScopeId;
    entries: *ScopeEntry;
    len:     u32;
    cap:     u32;
}

rec ResolveContext {
    s:          *sess.Session;
    a:          *ast.Ast;
    deps:       *ResolveDeps;
    ctx:        *comptime.ComptimeCtx;
    own_module: sess.ModuleId;
    source:     str;

    symbols:    *Symbol;
    sym_len:    u32;
    sym_cap:    u32;

    expr_resolved: *SymbolId;
    type_resolved: *SymbolId;
    decl_symbol:   *SymbolId;

    scopes:        *Scope;
    scope_len:     u32;
    scope_cap:     u32;

    module_scope:  ScopeId;
    current_scope: ScopeId;

    imported_cache: map.Map[u64, SymbolId];
}
```

`ResolveContext` is owned by [`resolve`](#resolve); never escapes.
[`build_result`](#internal-helpers) transfers the parallel arrays into
the returned [`ResolveResult`](#resolveresult) and `dnit`s everything
else.

## Dependencies

`std.types.bool`, `std.types.option`, `std.types.size`,
`std.types.string`, `std.types.zstr`, `std.types.result`,
`std.allocator`, `std.collections.map`,
[`mach.lang.source`](../source.md), [`mach.lang.diagnostic`](../diagnostic.md),
[`mach.lang.intern`](../intern.md), [`mach.lang.session`](../session.md),
[`mach.lang.fe.ast`](ast.md), [`mach.lang.fe.ast.id`](ast/id.md),
[`mach.lang.fe.ast.module`](ast/module.md),
[`mach.lang.fe.ast.decl`](ast/decl.md),
[`mach.lang.fe.ast.stmt`](ast/stmt.md),
[`mach.lang.fe.ast.expr`](ast/expr.md),
[`mach.lang.fe.ast.type`](ast/type.md),
[`mach.lang.fe.token`](token.md), [`mach.lang.fe.comptime`](comptime.md).
