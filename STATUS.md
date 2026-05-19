# Status — docs-first design phase of the `new/` compiler rewrite

Snapshot for moving work between machines. Pick this up by checking
out the `rebuild` branch and the `feat/str` branch in
`dep/mach-std`.

## What we're doing right now

A **docs-first design phase**. Per-file spec sheets are being written
under `doc/mach/`, mirroring the `src/` tree (referenced via that path
throughout the specs even though the rewrite still lives at `new/` on
disk). The specs are the canonical design; the source in `new/` is the
existing partial implementation. After the spec tree is complete the
plan is: audit → iterate the specs → **implement the entire rewrite
in one shot from the canonical specs**.

Touching source code during the docs phase is intentionally limited to:

- Module-level doc header comments aligned with the spec opening paragraph.
- Per-symbol doc comment cleanups (case, wording) that match spec language.
- Already-approved source restructures driven by design decisions
  (e.g. AST pool split, sentinel-purge cascade, `TYPE_INSTANCE`
  addition). These are noted in this STATUS as "source aligned with
  spec".

## Where things are committed

- `rebuild` (parent repo, this commit) — every spec doc written so
  far + the matching source updates in `new/` + the bumped submodule
  pointer.
- `feat/str` (`dep/mach-std`) — mach-std changes that the rewrite
  consumes (most notably the new `std.crypto.hash.fnv1a` module).
- Old `tmp` branches are gone.

## Design decisions locked in

These are the load-bearing decisions the spec tree commits to. Each
has matching language in the relevant doc.

1. **Fat-pointer `str` is a compiler builtin.** `rec { len: u32;
   data: *u8 }`. Backtick literals (`` `...` ``) produce `zstr` =
   `*u8`. `zstr` is a userland `pub def` in `std.types.zstr`; the type
   universe never sees it as a distinct kind.
2. **Compiler can't see `usize`/`isize`.** Those are userland defs
   gated by `$mach.build.target.pointer_width`. The compiler-builtin
   `str` uses `u32` for `len`.
3. **No silent fallbacks; no readonly pointers.** No `&T`. Casts (`::`)
   are primitive-to-primitive numeric conversion or non-primitive bit
   reinterpretation requiring the same size.
4. **Typed AST pools.** `Ast` carries `decl_ids`, `stmt_ids`,
   `expr_ids`, `type_ids` (plus the existing typed pools for
   `typed_names`, `field_inits`, `generic_names`, `comptime_branches`,
   `asm_operands`). The old untyped `extras: *u32` pool is gone. AST
   node fields use `(start: u32, len: u32)` pairs (offsets); typed
   `Slice[T]` from `std.collections.slice` is only constructed at the
   accessor boundary (no stored Slices — pool reallocation would
   invalidate `data` pointers).
5. **Sentinels in node fields are OK; sentinels in function returns
   are not.** AST keeps `EXPR_NIL`, `STMT_NIL`, `DECL_NIL`, `TYPE_NIL`,
   `MODULE_NIL`, `STR_NIL`, `SYMBOL_NIL` (compact, intentional). All
   lookup functions return `Option[T]`: `intern.lookup`, `ast.get_*`,
   `source.get`, `source.line_start`, `type.get`, `type.prim`,
   `type.get_param`, `comptime.lookup`.
6. **`StmtFin` carries `body: StmtId`**, not an `ExprId`. Both
   `fin call();` and `fin { ... }` parse via `parse_stmt`.
7. **`pub fwd <module-prefixed-symbol>;` is a real form.** Parser
   sets `DECL_FLAG_FWD` (+ `DECL_FLAG_PUB`); resolve re-exports the
   target into this module's pub namespace.
8. **Shared `TypeInterner` on the session.** Type identity is
   `TypeId` equality. `intern_pointer` / `intern_array` /
   `intern_nominal` / `intern_generic_param` dedup via a
   `Map[Type, TypeId]`; `intern_function` and `intern_instance`
   bypass the map and dedup via linear scan (acceptable counts; can
   be re-optimised later).
9. **Sema-time per-use-site monomorphisation.** When sema encounters
   `Map[str, u32]` it calls `ty.intern_instance(target, args)` which
   returns the same `TypeId` everywhere. `TYPE_INSTANCE` lives in the
   shared interner.
10. **FNV-1a primitives extracted to `std.crypto.hash.fnv1a`.** Both
    `std.collections.map.hash_str` and the rewrite's
    `mach.lang.type.hash_type` consume them; no duplicated magic
    numbers.

## Doc tree progress

Tree mirrors `src/` (paths in specs reference `src/...`). Done = full
spec written and source-side header aligned. Empty cells = not yet
started.

| Path                                   | State                                                     |
|----------------------------------------|-----------------------------------------------------------|
| `doc/mach/README.md`                   | (removed by user — top-level doc/ README is not used)     |
| `doc/mach/main.md`                     | done                                                      |
| `doc/mach/lang/session.md`             | done                                                      |
| `doc/mach/lang/intern.md`              | done                                                      |
| `doc/mach/lang/source.md`              | done                                                      |
| `doc/mach/lang/diagnostic.md`          | done                                                      |
| `doc/mach/lang/type.md`                | done (incl. `TYPE_INSTANCE` for generics)                 |
| `doc/mach/lang/driver.md`              | **next**                                                  |
| `doc/mach/lang/query.md`               | not started (design — empty placeholder)                  |
| `doc/mach/lang/target.md`              | not started                                               |
| `doc/mach/lang/target/{abi,isa,of,os}.md` + each platform impl | not started      |
| `doc/mach/lang/fe/token.md`            | done                                                      |
| `doc/mach/lang/fe/lexer.md`            | done                                                      |
| `doc/mach/lang/fe/ast.md`              | done                                                      |
| `doc/mach/lang/fe/ast/{id,module,expr,stmt,decl,type}.md` | done                   |
| `doc/mach/lang/fe/parser.md`           | done                                                      |
| `doc/mach/lang/fe/parser/{state,expr,decl,asm}.md` | done                          |
| `doc/mach/lang/fe/comptime.md`         | done                                                      |
| `doc/mach/lang/fe/resolve.md`          | done                                                      |
| `doc/mach/lang/fe/sema.md`             | done (design)                                             |
| `doc/mach/lang/fe/sema/{infer,check,coerce,generics}.md` | done (design)           |
| `doc/mach/lang/me/*.md`                | not started (design — empty placeholders)                 |
| `doc/mach/lang/be/*.md`                | not started (design — empty placeholders)                 |
| `doc/mach/cli/{cmd,args,config}.md`    | not started                                               |
| `doc/mach/cli/cmd/{build,run,test,dep,init,help}.md` | not started                |

## Source aligned with spec (in `new/`)

Each source file's module-level doc header now matches the spec
opening paragraph; per-symbol docs match spec wording. Beyond that,
the following structural changes were applied to keep source coherent
with the spec design:

- `lang/fe/ast.mach`: replaced `extras: *u32` pool + `add_extra`
  function with the four typed pools (`decl_ids`, `stmt_ids`,
  `expr_ids`, `type_ids`) and their `add_*_id` functions. Updated
  `init` / `dnit`.
- `lang/fe/parser/state.mach`: replaced `push_extra` with four typed
  `push_*_id` helpers.
- `lang/fe/parser/decl.mach`, `lang/fe/parser/expr.mach`: every
  list-building site swapped to the appropriate typed pool and
  `push_*_id` helper.
- `lang/fe/parser/decl.mach::parse_fin_stmt`: now reads a statement
  for the body and writes `fin.body: StmtId`.
- `lang/fe/resolve.mach`, `lang/driver.mach`, `lang/fe/comptime.mach`:
  list-iterating sites read from the typed pools (`rc.a.decl_ids` etc.).
- `lang/intern.mach`, `lang/source.mach`, `lang/fe/ast.mach`,
  `lang/type.mach`, `lang/fe/comptime.mach`: lookup functions
  (`lookup`, `get_*`, `get`, `line_start`, `prim`, `get_param`)
  changed signature to `Option[T]`. All call sites in `driver.mach`,
  `resolve.mach`, `comptime.mach`, `parser/*` updated to either
  propagate via `is_none` → `ret err[...]` (Result-returning callers)
  or use inline `unwrap[T](...)` (parser sites where the id is
  construction-guaranteed valid; unwrap panics on bug rather than
  silently absorbing).
- `lang/fe/ast/stmt.mach`: `StmtFin.expr: ExprId` → `body: StmtId`.
- `lang/type.mach`: added `TYPE_INSTANCE` constant, `TypeInstance`
  rec, `intern_instance` function with linear-scan dedup.

## Where to pick up

1. **`doc/mach/lang/driver.md`** is the next file to write. The
   source is `new/lang/driver.mach` (implemented; 1062 lines). The
   spec describes the project driver — loads `mach.toml` + dep
   manifests, DFS-walks the module graph from the selected target's
   entrypoint, evaluates `$if` predicates during load, runs resolve
   (and eventually sema) in topological dep order.
2. After driver: `query.md`, then the `target/` subtree, then `cli/*`
   (small/fast), then the big slabs `me/*` and `be/*`.
3. Style is locked in. Each spec opens with one paragraph; sections
   are `## Types`, `## Constants`, `## Functions`, plus extension
   sections for complex modules (grammar, dispatch tables,
   algorithms). Code blocks for declarations + parameter tables with
   linked types for every documented identifier (cross-spec links
   like `[`Name`](other.md#name)`; primitives stay unlinked since
   they're stdlib). Dependencies listed individually (no
   brace-grouping).

## Open audit items (not blockers, just notes)

- `driver.mach::add_pub_const`, `register_val_for_load`,
  `intern_or_default` cleanups from a prior audit pass are landed in
  source but may need fresh eyes once the spec for driver is written
  — the spec is the canonical design, so any drift between source
  and the new spec needs reconciling in favour of the spec.
- The `cli/cmd/*.mach` subcommand files are empty placeholders today.
  Once `cli/cmd.md` exists, the implementation phase will fill them.

## Useful greps

- `grep -r "extras"` — should be empty in both `new/` and `doc/mach/`.
- `grep -r "TYPE_DEF"` — should only appear in language docs / spec
  prose explaining the absence.
- `grep -r "fwd"` — `pub fwd` is real; flag is `DECL_FLAG_FWD`.
