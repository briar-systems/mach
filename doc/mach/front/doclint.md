[mach](../README.md) / [front](README.md) / **doclint**

# mach.front.doclint

the docstring linter: the component lines of every documented `pub fun`, `pub rec`,
`pub uni` and `pub tag` checked against the declaration they document

a component must name an element of the declaration, carry a description, and come
in declaration order; each violation is a `doc.lint` warning at the line. the
parser runs it on every module it parses (documentation.md, "What the lint checks")

<sub>source: [`src/front/doclint.mach`](../../../src/front/doclint.mach) · imported by 1 module</sub>

<sub>`ast` is [`mach.front.ast`](ast.md) · `diagnostic` is [`mach.base.diagnostic`](../base/diagnostic.md) · `fail` is [`mach.base.fail`](../base/fail.md)</sub>

| Functions | |
| :--- | :--- |
| [`lint_module`](#lint_module) | |

---

## Functions

### lint_module

```mach
pub fun lint_module(a: *ast.Ast, source: str, diags: *diagnostic.DiagnosticStore) err[fail.Fail]
```

