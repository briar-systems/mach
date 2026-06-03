# lang/fe/parser

Recursive-descent parser that consumes a `TokenStream` and fills an `Ast`, pushing diagnostics into a `DiagList`. The parent `parser.mach` exposes a single `parse()` entry point; the files here hold the implementation, split by syntactic category.

## Files

- `state.mach` — the `Parser` record plus the shared helpers every other file builds on: cursor (`current`, `peek`, `advance`), predicate (`at`, `at_kw`, `eat`, `eat_kw`), error reporting (`error_at`, `error_at_current`, `expect`, `sync_to`), span arithmetic (`span_of`), and the `push_*` wrappers that log allocation failures and return `*_NIL` rather than aborting. All sibling files depend on this one.
- `expr.mach` — expression and type parsing. Expression parsing is a hybrid recursive-descent primary / Pratt climber; type parsing is plain recursive descent. The two live in one file because they are mutually recursive: types contain expressions (array lengths), expressions contain types (casts, struct-literal and array-literal prefixes, generic arguments).
- `decl.mach` — declarations, statements, and the module-level parse entry point. Decls and stmts are kept together because they are mutually recursive (function bodies contain stmts; local `val`/`var` stmts wrap decls; `$if` conditionals appear at both scopes). Also owns the orchestrators `parse_module`, `parse_decl`, and `parse_stmt`.
- `iasm.mach` — inline assembly. Parses the mandatory ISA tag and collects the raw body text by tracking brace depth through the already-lexed token stream. There is no operand grammar: operand direction and clobbers are inferred later from the instruction stream, and local substitution happens through `{name}` references in the raw body.

## Dependency layout

```
parser.mach
   ├─ state.mach          (leaf — no parser/* imports)
   └─ decl.mach
          ├─ state.mach
          ├─ expr.mach    ←──── state.mach
          └─ iasm.mach    ←──── state.mach
```

`state.mach` is the leaf everything depends on; no cycles. The `expr.mach` file internally handles the type↔expression recursion by putting both grammars in one module.

## Error recovery

When a parse fails, the parser emits a diagnostic, sets its `panic` flag, and advances until it hits a synchronization token appropriate to the enclosing construct (`;`, `}`, matching delimiter, or start of the next top-level keyword). Panic is cleared at the sync point, the parser inserts an `*_KIND_ERROR` node so the tree stays well-formed, and parsing continues. Allocation failures surface through `state.push_*` helpers that log a diagnostic and return `*_NIL` rather than aborting.
