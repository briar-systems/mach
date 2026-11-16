[mach](../../README.md) / [front](../README.md) / [ast](README.md) / **module**

# mach.front.ast.module

the root node of one parsed file: its span, its docstring and its top-level declarations

<sub>source: [`src/front/ast/module.mach`](../../../../src/front/ast/module.mach) · imported by 14 modules</sub>

<sub>`base_source` is [`mach.base.source`](../../base/source.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Module`](#module) | `rec` | span:        the whole file doc:         the module docstring, empty when the file opens with none decls_start: the first top-level declaration decls_len:   how many top-level declarations |

---

## Module

```mach
pub rec Module {
    span:        base_source.Span;
    doc:         base_source.Span;
    decls_start: u32;
    decls_len:   u32;
}
```

span:        the whole file
doc:         the module docstring, empty when the file opens with none
decls_start: the first top-level declaration
decls_len:   how many top-level declarations

