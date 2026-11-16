[mach](../README.md) / [tool](README.md) / **format**

# mach.tool.format

the formatter: one canonical layout of a source file, from its tokens and its parsed tree

layout is decided by the tokens and the tree, with four author choices consulted: a blank line
where one is emitted anyway, a line break inside an expression, a line break after a literal's
`{`, and a single-statement body written on one line. comments and inline assembly bodies are
carried verbatim as atoms of their own, and a doc comment of the `# name: description` form has
its description column aligned. the rendered lines are then aligned in runs, `use` blocks are
sorted, and a `use` alias that repeats the last path segment is dropped. the output is checked
to mean what the input means before it is returned.

Source: [`src/tool/format.mach`](../../../src/tool/format.mach)

Uses types from [`mach.base.diagnostic`](../base/diagnostic.md), [`mach.base.fail`](../base/fail.md), [`mach.base.source`](../base/source.md).

| Functions | |
| :--- | :--- |
| [`format_source`](#format_source) | format one source file |

---

## Functions

### format_source

```mach
pub fun format_source(a: *std.allocator.Allocator, text: str, file_id: mach.base.source.FileId, diags: *mach.base.diagnostic.DiagnosticStore) res[str, mach.base.fail.Fail]
```

format one source file. success transfers one nul-terminated string to the
caller's allocator. a file that does not parse is rejected through `diags`
with a reported failure and produces no output; the output is verified to
mean what the input means before it is returned, so a formatter defect
surfaces as an internal failure and never as a rewritten file

