# mach.lang.fe.parser

## fun parse

```mach
pub fun parse(tokens: *fe_lexer.TokenStream, out: *ast.Ast, names: *intern.Interner, diags: *diagnostic.DiagnosticStore) res[lang_phase.PhaseStatus, state.ParseFail];
```

the names the tree holds are interned in `names`, which must outlive it

