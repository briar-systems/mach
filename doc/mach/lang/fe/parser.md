# mach.lang.fe.parser

## fun parse

```mach
pub fun parse(tokens: *lexer.TokenStream, out: *ast.Ast, diags: *diagnostic.DiagnosticStore) res[fail.PhaseStatus, state.ParseFail];
```

