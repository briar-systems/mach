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

<sub>source: [`src/tool/format.mach`](../../../src/tool/format.mach) · imported by 1 module</sub>

<sub>`A` is `std.allocator` · `base_source` is [`mach.base.source`](../base/source.md) · `diagnostic` is [`mach.base.diagnostic`](../base/diagnostic.md) · `fail` is [`mach.base.fail`](../base/fail.md)</sub>

| Functions | |
| :--- | :--- |
| [`format_source`](#format_source) | format one source file |

---

## Functions

### format_source

```mach
pub fun format_source(a: *A.Allocator, text: str, file_id: base_source.FileId, diags: *diagnostic.DiagnosticStore) res[str, fail.Fail]
```

format one source file. success transfers one nul-terminated string to the
caller's allocator. a file that does not parse is rejected through `diags`
with a reported failure and produces no output; the output is verified to
mean what the input means before it is returned, so a formatter defect
surfaces as an internal failure and never as a rewritten file

