# Editor query surface (`mach.lang.editor`)

The compiler-as-library entry point for editor tooling: language servers,
plugins, single-file linters. `mach-lsp` is its consumer. It analyzes one
in-memory buffer at a time and, when that buffer belongs to a project, does so
**project-aware**: the buffer is analyzed as the module it is inside its
project, through the same target-aware load, resolve, and sema services the
driver uses, with the project's real dependency set. A buffer outside any
project is analyzed on its own.

This module, the CLI, and the manifest schema are the supported surface of the
compiler. They are source-stable within a major version and not binary-stable;
everything else under `src/` is internal and carries no promise.

It is built on the same front-end phases the driver uses (`source`, `lexer`,
`parser`, `resolve`, `sema`, `diagnostic`) and reuses the session services
(`SourceMap`, the diagnostic store, interners). Every entry tolerates a
malformed buffer: it records diagnostics and returns a usable partial result
rather than bailing at the first error.

## Lifecycle

```
session.Session              // caller owns; provides SourceMap + diagnostic store
  └─ editor.EditorSession    // borrows the session, owns per-buffer analysis
       ├─ Buffer (per FileId)   // owned Ast / ResolveResult / SemaResult, or borrowed from the project
       └─ Project               // loaded on demand for a buffer inside a project root
```

```mach
var s: session.Session = unwrap_ok[...](session.init(?alloc));
var es: editor.EditorSession = editor.init(?s);
// ... drive es ...
editor.dnit(?es);   // frees every buffer's owned analysis and the loaded project
session.dnit(?s);   // tears the session down (editor never touches it)
```

The session must outlive the editor session. `editor.dnit` releases only what
the editor owns: per-buffer analysis it computed itself, and the project it
loaded. The borrowed `SourceMap` and diagnostic store belong to the session.

## API

| Function | Signature | Purpose |
|---|---|---|
| `init` | `fun(*session.Session) EditorSession` | construct the facade over a session |
| `dnit` | `fun(*EditorSession)` | release owned per-buffer analysis and the loaded project |
| `open` | `fun(*EditorSession, str, str) Result[source.FileId, str]` | register an unsaved buffer (path, text); return its `FileId` |
| `update` | `fun(*EditorSession, source.FileId, str) Result[bool, str]` | replace a buffer's text; `true` when the text changed, dropping cached analysis |
| `tokenize` | `fun(*EditorSession, source.FileId) Result[lexer.TokenStream, str]` | lex; lex errors land on the diagnostic store (caller frees the stream) |
| `parse` | `fun(*EditorSession, source.FileId) Result[*ast.Ast, str]` | best-effort parse; the project's AST for a project module |
| `resolve` | `fun(*EditorSession, source.FileId) Result[*res.ResolveResult, str]` | name-resolution side tables; project-aware |
| `analyze` | `fun(*EditorSession, source.FileId) Result[*context.SemaResult, str]` | type-check; project-aware |
| `diagnostics` | `fun(*EditorSession, source.FileId) Result[*diagnostic.DiagnosticStore, str]` | reset, then lex and parse; the session store holding this buffer's syntactic diagnostics |
| `ast_of` | `fun(*EditorSession, source.FileId) *ast.Ast` | cached `Ast`, or nil |
| `resolve_of` | `fun(*EditorSession, source.FileId) *res.ResolveResult` | cached `ResolveResult`, or nil |
| `sema_of` | `fun(*EditorSession, source.FileId) *context.SemaResult` | cached `SemaResult`, or nil |
| `expr_type_of` | `fun(*EditorSession, source.FileId, id.ExprId) type.TypeId` | the checked type of an expression (`TYPE_NIL` for `EXPR_NIL`, `TYPE_ERROR` when unavailable) |
| `decl_type_of` | `fun(*EditorSession, source.FileId, id.DeclId) type.TypeId` | the checked type of a declaration |
| `resolved_type_of` | `fun(*EditorSession, source.FileId, id.TypeId) type.TypeId` | the semantic type a syntactic type node resolved to |
| `build` | `fun(*EditorSession, *request.BuildRequest) Result[outcome.BuildOutcome, outcome.Fail]` | run a full build of the buffer's project through the build engine |
| `fail_dnit` | `fun(*EditorSession, *outcome.Fail)` | release a `Fail` returned by `build` |

`open` loads the buffer text into the session's `SourceMap` as an **overlay**:
when the project is loaded, the module at that path is read from the buffer,
not from disk, so unsaved edits are what get analyzed. `open` and `update`
both mark the project dirty, so the next project-aware query reloads it.
`update` returns `false` when the new text equals the old and keeps the cached
analysis.

Every analysis entry (`tokenize`, `parse`, `resolve`, `diagnostics`) first
resets the session's diagnostic store, so the store holds only the diagnostics
of the most recent query.

## Project-aware analysis

`parse`, `resolve`, and `analyze` decide how to analyze a buffer from its path:

1. Walk up from the buffer's directory to the nearest `mach.toml`. That
   directory is the project root.
2. If the buffer lies under that project's `src` directory, compose its module
   FQN and load the project (manifest, dependencies, every module the buffer's
   module reaches) exactly as `mach build` would, with the buffer's overlay
   text in place of the file. The query then returns the project's own `Ast`,
   `ResolveResult`, or `SemaResult` for that module: cross-module references
   through `use` bind to their real declarations, and `$if` gates evaluate
   against the project's selected target.
3. Otherwise (no manifest above the buffer, or a file outside `src`) the buffer
   is analyzed **in isolation** with an empty dependency set: local
   declarations bind, and anything reached through a `use` resolves to
   `SYMBOL_NIL`. The comptime context is seeded with host defaults.

A project load failure (an invalid manifest, a missing dependency) is returned
as the query's error string; it is the same message `mach build` would print.
The project is cached across queries and reloaded when a buffer changes, when
a buffer from a different project root is queried, or when a module not yet
among the loaded roots is queried.

Only the target the project selects by default is loaded; there is no
per-query target override on this surface.

## Diagnostics

`diagnostics` runs `tokenize → emit-lex-errors → parse` and returns the
session's store of **syntactic** diagnostics only. Each `diag.Diagnostic`
carries `(severity, loc, message, note, help, related[])`: `loc` is a
`Location` (`file_id`, `span`) for the primary report, `note` and `help` are
optional trailing lines, and `related` is a growable array (`related_len`
entries) of secondary `Location`s each with an optional `label`. Map each
`loc.span` to a position with `source.position`:

```mach
val store: *diagnostic.DiagnosticStore = unwrap_ok[...](editor.diagnostics(?es, fid));
var i: usize = 0;
for (i < store.len) {
    val d: *diagnostic.Diagnostic = ?store.items[i];
    val sf: *source.SourceFile = unwrap[...](source.get(?s.sources, d.loc.file_id));
    val pos: source.Position = source.position(sf, d.loc.span.offset); // 1-based line/col
    // d.severity, pos.line, pos.col, d.message -> LSP Diagnostic
    i = i + 1;
}
```

`help` and `related` are populated by the **resolve** and **sema** stages (a
suggestion rides `help`, rendered `= help:`; a prior binding rides `related`,
e.g. `previous definition here`), so an integrator wanting them drives
`resolve` or `analyze` and reads the session store afterwards. The parse-only
`diagnostics` slice never sets them. Map a `related` entry's `loc` to a
position the same way, and surface its `label` as an LSP `relatedInformation`
item.

## Position lookup: offset to id

Hover, go-to-definition, and references pivot on finding the AST node under a
byte offset, then reading the side tables. `mach.lang.fe.ast` provides the
lookups, each returning the *tightest* (smallest-span) enclosing node, or the
id-type sentinel when none covers the offset:

```mach
pub fun offset_to_expr(a: *ast.Ast, offset: usize) id.ExprId   // or id.EXPR_NIL
pub fun offset_to_stmt(a: *ast.Ast, offset: usize) id.StmtId   // or id.STMT_NIL
pub fun offset_to_decl(a: *ast.Ast, offset: usize) id.DeclId   // or id.DECL_NIL
pub fun offset_to_type(a: *ast.Ast, offset: usize) id.TypeId   // or id.TYPE_NIL
```

From an `ExprId`, the resolve side tables give the binding and the sema
tables give the type:

```mach
val eid: id.ExprId = ast.offset_to_expr(a, offset);
if (eid != id.EXPR_NIL) {
    val sid: res.SymbolId = rr.expr_resolved[eid];     // SYMBOL_NIL if unbound
    if (sid != res.SYMBOL_NIL) {
        val sym: *res.Symbol = ?rr.symbols[sid];
        // sym.decl (DeclId in sym.origin's module), sym.kind, sym.name
    }
    val ty: type.TypeId = editor.expr_type_of(?es, fid, eid);   // after analyze
}
```

`rr.expr_resolved`, `rr.type_resolved`, and `rr.decl_symbol` are parallel to
`a.exprs`, `a.types`, and `a.decls` respectively. For a project module the
`ResolveResult` is the project's, so `sym.origin` may name another module of
the project or of a dependency.

## Partial-result tolerance

Every phase is best-effort by construction:

- The **lexer** records malformed input as `LexError`s on the stream instead
  of aborting; `lexer.emit_diagnostics` (and the editor's `tokenize`/`parse`)
  drains them onto the diagnostic store.
- The **parser** is explicitly malformed-input tolerant: it emits diagnostics
  and synthesizes `*_KIND_ERROR` nodes while continuing, so a broken buffer
  still yields a partial `Ast` with a set `root_module`.
- **resolve** records name-resolution diagnostics and leaves the offending
  side-table slots as `SYMBOL_NIL`, producing usable tables over a broken tree.

A buffer with a missing `)` and a stray backtick still parses to a tree
containing its function declaration, surfaces both the lex and the parse
error, and resolves to populated side tables.

## Building from the editor

`build` runs a complete build of the project named by the request's
`project_root`: it loads the manifest, plans the artifact cells, and executes
them through the same engine `mach build` uses, so an editor can offer "build"
without spawning the CLI. The outcome and its `Fail` are the driver's own
types; a `Fail` whose kind is `FAIL_REPORTED` means the diagnostics were
already recorded on the session store, and `fail_dnit` releases whatever the
editor duplicated for the other kinds.

## See also

- [../cli.md](../cli.md) — the command-line surface the editor mirrors
- [../manifest.md](../manifest.md) — the project the editor discovers
