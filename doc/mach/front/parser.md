[mach](../README.md) / [front](README.md) / **parser**

# mach.front.parser

the parser's entry: one token stream parsed into a syntax tree, then its docstrings
linted, the tree handed back only when the parse completed

<sub>source: [`src/front/parser.mach`](../../../src/front/parser.mach) · imported by 3 modules</sub>

<sub>`front_lexer` is [`mach.front.lexer`](lexer.md) · `ast` is [`mach.front.ast`](ast.md) · `intern` is [`mach.base.intern`](../base/intern.md) · `diagnostic` is [`mach.base.diagnostic`](../base/diagnostic.md) · `report_phase` is [`mach.report.phase`](../report/phase.md) · `state` is [`mach.front.parser.state`](parser/state.md)</sub>

| Functions | |
| :--- | :--- |
| [`parse`](#parse) | the names the tree holds are interned in `names`, which must outlive it |

---

## Functions

### parse

```mach
pub fun parse(tokens: *front_lexer.TokenStream, out: *ast.Ast, names: *intern.Interner, diags: *diagnostic.DiagnosticStore) res[report_phase.PhaseStatus, state.ParseFail]
```

the names the tree holds are interned in `names`, which must outlive it

