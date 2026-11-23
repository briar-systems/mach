[mach](../README.md) / [front](README.md) / **parser**

# mach.front.parser

the parser's entry: one token stream parsed into a syntax tree, handed back only when
the parse completed. its docstrings are linted by name resolution, for the project's
own modules only

Source: [`src/front/parser.mach`](../../../src/front/parser.mach)

Uses types from [`mach.base.diagnostic`](../base/diagnostic.md), [`mach.base.intern`](../base/intern.md), [`mach.front.ast`](ast.md), [`mach.front.lexer`](lexer.md), [`mach.front.parser.state`](parser/state.md), [`mach.report.phase`](../report/phase.md).

| Functions | |
| :--- | :--- |
| [`parse`](#parse) | the names the tree holds are interned in `names`, which must outlive it |

---

## Functions

### parse

```mach
pub fun parse(tokens: *mach.front.lexer.TokenStream, out: *mach.front.ast.Ast, names: *mach.base.intern.Interner, diags: *mach.base.diagnostic.DiagnosticStore) res[mach.report.phase.PhaseStatus, mach.front.parser.state.ParseFail]
```

the names the tree holds are interned in `names`, which must outlive it

