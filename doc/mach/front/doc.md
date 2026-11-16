[mach](../README.md) / [front](README.md) / **doc**

# mach.front.doc

the model of one doc comment: its prose paragraphs, the summary first, and the
component lines after `# ---`

a doc comment is a span of source holding one or more `#` lines. this module is
the one reader of that span: the parser's linter walks its components, and the
documentation generator takes its paragraphs and component descriptions as text
with the comment marks removed, through `prose` and `flowed`

Source: [`src/front/doc.mach`](../../../src/front/doc.mach)

Uses types from [`mach.base.source`](../base/source.md).

| Type | | |
| :--- | :--- | :--- |
| [`DocComponent`](#doccomponent) | `rec` |  |
| [`DocComponentIter`](#doccomponentiter) | `rec` |  |
| [`ParagraphIter`](#paragraphiter) | `rec` | the paragraphs of a doc comment's prose, which ends at the separator or the end |

| On `DocComponentIter` | |
| :--- | :--- |
| [`component_iter`](#component_iter) | |
| [`component_next`](#component_next) | the next component written to `out`, false once none is left |

| On `ParagraphIter` | |
| :--- | :--- |
| [`paragraph_iter`](#paragraph_iter) | |
| [`paragraph_next`](#paragraph_next) | the next paragraph, the lines up to a blank `#` line, written to `out`; false once none is left |

| Functions | |
| :--- | :--- |
| [`has_component_block`](#has_component_block) | |
| [`summary_len`](#summary_len) | the length of the summary of `prose`, a doc comment's first paragraph without its comment marks: the text before its first sentence end, a `.` followed by white space or the end of the text, outside a code span, and not the last `.` of `e.g.`, `i.e.`, `etc.` or `vs.`; the whole text when no sentence ends in it |
| [`prose`](#prose) | the text of `span`, every line without its indentation, its `#` and the one blank after it, lines kept apart by a newline |
| [`flowed`](#flowed) | the text of `span`, every line without its `#` and every blank around its words, lines joined by one space: a component description that wraps over continuation lines |
| [`component_head_len`](#component_head_len) | the length of the component head that opens the comment line at `line`, from its `#` through its `:`: `#`, zero or one space, a name with no space in it, and a `:` that ends the line or is followed by a space |
| [`continues`](#continues) | whether the comment line at `line` continues a description: `#`, two or more spaces, then text |

---

## DocComponent

```mach
pub rec DocComponent {
    name_span: mach.base.source.Span;
    desc_span: mach.base.source.Span;
}
```

## DocComponentIter

```mach
pub rec DocComponentIter {
    source: str;
    off:    usize;
    end:    usize;
}
```

### component_iter

```mach
pub fun component_iter(source: str, doc: mach.base.source.Span) DocComponentIter
```

### component_next

```mach
pub fun component_next(it: *DocComponentIter, out: *DocComponent) bool
```

the next component written to `out`, false once none is left

## ParagraphIter

```mach
pub rec ParagraphIter {
    source: str;
    off:    usize;
    end:    usize;
}
```

the paragraphs of a doc comment's prose, which ends at the separator or the end

### paragraph_iter

```mach
pub fun paragraph_iter(source: str, doc: mach.base.source.Span) ParagraphIter
```

### paragraph_next

```mach
pub fun paragraph_next(it: *ParagraphIter, out: *mach.base.source.Span) bool
```

the next paragraph, the lines up to a blank `#` line, written to `out`; false once none is left

## Functions

### has_component_block

```mach
pub fun has_component_block(source: str, doc: mach.base.source.Span) bool
```

### summary_len

```mach
pub fun summary_len(prose: str) usize
```

the length of the summary of `prose`, a doc comment's first paragraph without its
comment marks: the text before its first sentence end, a `.` followed by white space or
the end of the text, outside a code span, and not the last `.` of `e.g.`, `i.e.`, `etc.`
or `vs.`; the whole text when no sentence ends in it

### prose

```mach
pub fun prose(a: *std.allocator.Allocator, source: str, span: mach.base.source.Span) res[str, std.allocator.Error]
```

the text of `span`, every line without its indentation, its `#` and the one blank after it,
lines kept apart by a newline

### flowed

```mach
pub fun flowed(a: *std.allocator.Allocator, source: str, span: mach.base.source.Span) res[str, std.allocator.Error]
```

the text of `span`, every line without its `#` and every blank around its words, lines
joined by one space: a component description that wraps over continuation lines

### component_head_len

```mach
pub fun component_head_len(source: str, line: mach.base.source.Span) usize
```

the length of the component head that opens the comment line at `line`, from its `#`
through its `:`: `#`, zero or one space, a name with no space in it, and a `:` that ends
the line or is followed by a space. 0 when the line opens no component. the formatter
aligns descriptions by it and the linter and the generator read components by it

### continues

```mach
pub fun continues(source: str, line: mach.base.source.Span) bool
```

whether the comment line at `line` continues a description: `#`, two or more spaces,
then text

