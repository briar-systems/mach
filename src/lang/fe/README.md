# lang/fe — Frontend

Parses source text into an AST, resolves names against declared scopes, and produces a fully typed AST.

## Pipeline

```
source text → lexer → parser → resolve → sema → typed AST
```

Each step is resilient to malformed input: the lexer, parser, and semantic phases produce best-effort partial results with error markers rather than aborting. Consumers (CLI, LSP) receive diagnostics via the session's sink as they are produced.

## Files

- `token.mach` — token kinds and token record definition.
- `lexer.mach` — produces a token stream from source text.
- `ast.mach` — public surface for AST node types. Internal subdivision in `ast/`.
- `parser.mach` — public entry for parsing; delegates to the grammar files in `parser/`.
- `resolve.mach` — name resolution. Binds identifiers to symbols and builds scope information.
- `sema.mach` — public surface for semantic analysis. Internal subdivision in `sema/`.
- `comptime.mach` — compile-time evaluation of constant expressions encountered during semantic analysis.
