# Editor query surface (`mach.lang.editor`)

The compiler-as-library entry point for editor tooling — language servers,
plugins, single-file linters. It exposes best-effort analysis of one
in-memory buffer at a time, without a `mach.toml`, a module graph, or a link
step. This is the foundation an LSP binds to.

It is built on the same front-end phases the driver uses (`source`, `lexer`,
`parser`, `resolve`, `diagnostic`) and reuses the session services
(`SourceMap`, `DiagList`, interners). Every entry tolerates a malformed
buffer: it records diagnostics and returns a usable partial result rather
than bailing at the first error.

## Lifecycle

```
sess.Session            // caller owns; provides SourceMap + DiagList
  └─ editor.EditorSession   // borrows the session, owns per-buffer analysis
       └─ Buffer (per FileId)   // owned Ast + ResolveResult, cached
```

```mach
var s: sess.Session = unwrap_ok[...](sess.init(?alloc));
var es: editor.EditorSession = editor.init(?s);
// ... drive es ...
editor.dnit(?es);   // frees every buffer's Ast / ResolveResult
sess.dnit(?s);      // tears the session down (editor never touches it)
```

The session must outlive the editor session. `editor.dnit` releases only what
the editor owns (the per-buffer `Ast`, `ResolveResult`, and comptime context);
the borrowed `SourceMap` and `DiagList` belong to the session.

## API

| Function | Signature | Purpose |
|---|---|---|
| `init` | `fun(*sess.Session) EditorSession` | construct the facade over a session |
| `dnit` | `fun(*EditorSession)` | release owned per-buffer analysis |
| `open` | `fun(*EditorSession, str, str) Result[src.FileId, str]` | register an unsaved buffer's TEXT, return its FileId |
| `update` | `fun(*EditorSession, src.FileId, str) Result[bool, str]` | replace a dirty buffer's text; drops cached analysis |
| `tokenize` | `fun(*EditorSession, src.FileId) Result[lexer.TokenStream, str]` | lex; lex errors land on the DiagList (caller frees the stream) |
| `parse` | `fun(*EditorSession, src.FileId) Result[*ast.Ast, str]` | best-effort parse; returns the cached `*Ast` |
| `resolve` | `fun(*EditorSession, src.FileId) Result[*res.ResolveResult, str]` | best-effort name resolution side tables (deps optional) |
| `diagnostics` | `fun(*EditorSession, src.FileId) Result[*diag.DiagList, str]` | reset + analyze; the session DiagList of this buffer |
| `ast_of` | `fun(*EditorSession, src.FileId) *ast.Ast` | cached parsed Ast, or nil |
| `resolve_of` | `fun(*EditorSession, src.FileId) *res.ResolveResult` | cached ResolveResult, or nil |

`open` loads the buffer text into the session's `SourceMap` and bumps the
query DB revision, so the incremental machinery already invalidates derived
entries over the file. `update` rebuilds the text and drops the buffer's
cached `Ast` / `ResolveResult`, so the next `parse` / `resolve` recomputes.

## Diagnostics — the first consumer slice

`diagnostics` runs `tokenize → emit-lex-errors → parse` and returns the
session `DiagList` of **syntactic** diagnostics only. Each `diag.Diagnostic`
carries `(severity, loc, message, note, help, related[])`: `loc` is a
`Location` (`file_id`, `span`) for the primary report, `note` and `help` are
optional trailing lines, and `related` is a growable array (`related_len`
entries) of secondary `Location`s each with an optional `label`. Map each
`loc.span` to a position with `source.position`:

```mach
val list: *diag.DiagList = unwrap_ok[...](editor.diagnostics(?es, fid));
var i: usize = 0;
for (i < list.len) {
    val d: *diag.Diagnostic = ?list.items[i];
    val sf: *src.SourceFile = unwrap[...](src.get(?s.sources, d.loc.file_id));
    val pos: src.Position = src.position(sf, d.loc.span.offset); // 1-based line/col
    // d.severity, pos.line, pos.col, d.message -> LSP Diagnostic
    i = i + 1;
}
```

`help` and `related` are populated by the **resolve** stage (a suggestion rides
`help`, rendered `= help:`; a prior binding rides `related`, e.g. `previous
definition here`), so an integrator wanting them drives `resolve` (or full
`analyze`) and reads `es.s.diags` — the parse-only `diagnostics` slice never
sets them. Map a `related` entry's `loc` to a position the same way, and surface
its `label` as an LSP `relatedInformation` item.

The `mach check <file>` CLI command is the in-tree consumer of the parse-only
slice; `mach build` renders the resolve-stage diagnostics that carry `help` and
`related`.

## Position lookup — offset → id

Hover, go-to-definition, and references pivot on finding the AST node under a
byte offset, then reading the resolve side tables. `mach.lang.fe.ast` provides
the lookups, each returning the *tightest* (smallest-span) enclosing node, or
the id-type sentinel when none covers the offset:

```mach
pub fun offset_to_expr(a: *ast.Ast, offset: usize) id.ExprId   // or id.EXPR_NIL
pub fun offset_to_stmt(a: *ast.Ast, offset: usize) id.StmtId   // or id.STMT_NIL
pub fun offset_to_decl(a: *ast.Ast, offset: usize) id.DeclId   // or id.DECL_NIL
pub fun offset_to_type(a: *ast.Ast, offset: usize) id.TypeId   // or id.TYPE_NIL
```

From an `ExprId`, the resolve side tables give the binding:

```mach
val eid: id.ExprId = ast.offset_to_expr(a, offset);
if (eid != id.EXPR_NIL) {
    val sid: res.SymbolId = rr.expr_resolved[eid];     // SYMBOL_NIL if unbound
    if (sid != res.SYMBOL_NIL) {
        val sym: *res.Symbol = ?rr.symbols[sid];
        // sym.decl (DeclId in sym.origin's module), sym.kind, sym.name
    }
}
```

`rr.expr_resolved`, `rr.type_resolved`, and `rr.decl_symbol` are parallel to
`a.exprs`, `a.types`, and `a.decls` respectively.

## Partial-result tolerance

Every phase is best-effort by construction:

- The **lexer** records malformed input as `LexError`s on the stream instead
  of aborting; `lexer.emit_diagnostics` (and the editor's `tokenize`/`parse`)
  drains them onto the `DiagList`.
- The **parser** is explicitly malformed-input tolerant — it emits diagnostics
  and synthesizes `*_KIND_ERROR` nodes while continuing, so a broken buffer
  still yields a partial `Ast` with a set `root_module`.
- **resolve** records name-resolution diagnostics and leaves the offending
  side-table slots as `SYMBOL_NIL`, producing usable tables over a broken tree.

A buffer with a missing `)` and a stray backtick still parses to a tree
containing its function declaration, surfaces both the lex and the parse
error, and resolves to populated side tables.

## Single-buffer scope and deps

`resolve` analyzes the buffer in isolation with an empty dependency set:
local declarations bind, but cross-module references (anything reached through
a `use`) resolve to `SYMBOL_NIL`. The comptime context is seeded with neutral
host defaults, so `$if` predicates evaluate against the host os/arch only.
Full cross-module resolution remains the driver's job (`driver.build_project`).

## Relationship to the query DB

This surface drives the front-end phases directly rather than routing through
`query.QueryDb.get(Q_PARSE / Q_RESOLVE)`. The derived query shards are not
registered until the query DB is wired into the build (issue #1164). The
buffer is still registered as a real source input whose revision the
`Q_FILE_TEXT` metadata source reads, so the incremental story is preserved;
the `get`-routed form of these entries can land with #1164 without changing
this module's signatures.
