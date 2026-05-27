# mach.lang.fe.parser

Public entry to the parser. Constructs a [`state.Parser`](parser/state.md#parser)
over the lexer's token stream and delegates to the grammar files under
[`parser/`](parser/state.md). The parser is malformed-input tolerant —
errors are emitted into the supplied diagnostic sink while parsing
continues; the caller does not see a fallible return.

## Functions

### `parse`

```mach
pub fun parse(tokens: *lexer.TokenStream, out: *ast.Ast, diags: *diag.DiagList)
```

Parses `tokens` into `out`, sets [`out.root_module`](ast.md#ast) on
completion, and pushes diagnostics into `diags` as it goes.

| Param  | Type                                                  | Description                                              |
|--------|-------------------------------------------------------|----------------------------------------------------------|
| tokens | [`*lexer.TokenStream`](lexer.md#tokenstream)          | Lexer output to consume.                                 |
| out    | [`*ast.Ast`](ast.md#ast)                           | AST to append parsed nodes into.                         |
| diags  | [`*diag.DiagList`](../diagnostic.md#diaglist)         | Diagnostic sink; parse errors are pushed here.           |

Returns nothing. After return, [`out.root_module`](ast.md#ast)
identifies the parsed [`Module`](ast/module.md#module). If the input
was malformed, `diags` will contain one or more [`SEVERITY_ERROR`](../diagnostic.md#constants)
entries; the AST is best-effort with [`*_KIND_ERROR`](ast/expr.md#exprkind)
nodes at unrecoverable points.

## Dependencies

`std.types.bool`, `std.types.size`, `std.types.string`,
`std.types.result`, [`mach.lang.fe.lexer`](lexer.md),
[`mach.lang.diagnostic`](../diagnostic.md), [`mach.lang.fe.ast`](ast.md),
[`mach.lang.fe.parser.state`](parser/state.md),
[`mach.lang.fe.parser.decl`](parser/decl.md).
