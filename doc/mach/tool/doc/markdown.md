[mach](../../README.md) / [tool](../README.md) / [doc](README.md) / **markdown**

# mach.tool.doc.markdown

the markdown renderer of the document model: GitHub-flavoured pages, one per module,
one `README.md` per directory, and the project's page at the root

a page's path is relative to the directory the reference is written in, and every
link on it is relative to the page. anchors follow GitHub's rule for headings, a
repeated heading on one page taking `-1`, `-2` and so on, and every page's anchors
are planned before any page is written, so a link to another page's entry names
the heading that page will carry. prose is escaped so `<` and `&` outside code
spans read as written

<sub>source: [`src/tool/doc/markdown.mach`](../../../../src/tool/doc/markdown.mach) · imported by 1 module</sub>

<sub>`A` is `std.allocator` · `model` is [`mach.tool.doc.model`](model.md) · `fail` is [`mach.base.fail`](../../base/fail.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Page`](#page) | `rec` | one page: its path below the reference's directory and its text |

| On `Page` | |
| :--- | :--- |
| [`render`](#render) | every page of `m` |

---

## Page

```mach
pub rec Page {
    path: str;
    text: str;
}
```

one page: its path below the reference's directory and its text

### render

```mach
pub fun render(a: *A.Allocator, m: *model.Project, root: str, out: *Vector[Page]) err[fail.Fail]
```

every page of `m`

| Parameter | |
| :--- | :--- |
| `a` | the arena the pages are built in |
| `m` | the model |
| `root` | the project root as seen from the reference's directory, such as `../../` |
| `out` | receives the pages, the project's page first |
| **returns** | ok, or the allocator's refusal |

