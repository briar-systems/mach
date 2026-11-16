[mach](../README.md) / [front](README.md) / **doclint**

# mach.front.doclint

the docstring linter: the component lines of every documented `pub fun`, `pub rec`,
`pub uni` and `pub tag` checked against the declaration they document

a component must name an element of the declaration, carry a description, and come
in declaration order; each violation is a `doc.lint` warning at the line. name
resolution runs it on every module of the project being built, never a dependency's,
as it does the unused-import lint, and `mach doc` on every file it documents
(documentation.md, "What the lint checks")

Source: [`src/front/doclint.mach`](../../../src/front/doclint.mach)

Uses types from [`mach.base.diagnostic`](../base/diagnostic.md), [`mach.base.fail`](../base/fail.md), [`mach.front.ast`](ast.md).

| Functions | |
| :--- | :--- |
| [`lint_module`](#lint_module) | |

---

## Functions

### lint_module

```mach
pub fun lint_module(a: *mach.front.ast.Ast, source: str, diags: *mach.base.diagnostic.DiagnosticStore) err[mach.base.fail.Fail]
```

