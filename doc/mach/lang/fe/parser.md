# mach.lang.fe.parser

## fun parse

```mach
pub fun parse(tokens: *fe_lexer.TokenStream, out: *ast.Ast, diags: *diagnostic.DiagnosticStore) res[lang_phase.PhaseStatus, state.ParseFail];
```

