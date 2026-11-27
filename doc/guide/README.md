# Mach guide

The guide is the short way into Mach: one page per topic, a sample first, then
the rules as a list or a table. The [language reference](../language/README.md)
is the authority, and every page ends by linking the reference pages that hold
the full rules.

[machlang.org/docs](https://machlang.org/docs/) renders these files. Each
release's tag carries the guide for that release, and the site shows the
newest by default. Start at [index.md](index.md).

## Files

- `<page>.md` is one page. Its `#` title and the paragraph under it are the
  page's title and lead.
- [nav.json](nav.json) lists every page once. `groups` gives the sidebar's
  groups and the order of the pages in each. `moved` maps a retired page's name
  to where its content went, as `"variadics": "functions.html#variadic-packs"`,
  so its address keeps answering.

## Writing a page

- A page describes the release it ships in and no other. What changed between
  releases belongs to the changelog.
- A section opens with a sample or a table. Prose follows it and stays short.
- A page that grows past a screen or two of prose is moving into the
  reference's territory: put the detail there and link it.

## Markdown

The site's renderer takes the forms below and refuses a page that holds
anything else, so a page reads the same here and there.

| Form | Written |
|---|---|
| Headings | `#` once for the title, then `##` to `####`. Two headings on a page never read the same |
| Paragraphs, `**bold**`, `*emphasis*`, `` `code` `` | As usual |
| Lists | `-` or `1.`, with one level of nesting |
| Tables | Pipe rows with a header row and a `\|---\|` rule. A `\|` inside a cell is escaped |
| Callout | A quote that opens with a bold label: `> **Note:** text` |
| Code | A fenced block that names its language: `mach`, `toml`, `sh` or `text` |

Raw HTML, images, footnotes and reference-style links are not among them.

## Links

| To | Written |
|---|---|
| A page of the guide | `[Types](types.md)`, `[casts](values.md#casts)` |
| A heading on the same page | `[Operators](#operators)` |
| A file of this repository | A relative path: `[types](../language/types.md)` |
| A page of the site outside the guide | Its full address: `https://machlang.org/playground/` |
| Anything else | Its full address |

A heading's anchor is its text in lowercase with each run of other characters
written as one `-`.

## Code blocks

The fence of a `mach` block says what the block is, in the words the
[reference](../language/README.md) uses.

| Fence | The block is |
|---|---|
| `` ```mach `` | A whole program or module. It builds as written, with std as its only dependency |
| `` ```mach fragment `` | Part of one. It needs something the block does not show: an earlier block, another file, a manifest or a library |
| `` ```mach error <text> `` | Refused by the compiler, with a diagnostic that holds `<text>` |

A block that shows several files starts each with `# file: src/<path>.mach`.
It is built as a project whose id is `example`, so its files import each other
as `example.<path>`.

A block is laid out as `mach fmt` writes it. Output a page shows is copied from
a real run, in a `text` block.

The site builds every block that is not a fragment against each release, runs
the programs among them and holds the refused ones to their text.
