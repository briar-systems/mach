[mach](../README.md) / [tool](README.md) / **doc**

# mach.tool.doc

the documentation generator: a project's reference, written as markdown pages into
the one directory the generator owns, `<dir>/<project id>/`

every `.mach` file under the project's source root is parsed and nothing more is
run: no target, no name resolution, no dependency. the document model is built from
the syntax trees (`mach.tool.doc.build`) and rendered to pages
(`mach.tool.doc.markdown`); this module reads the files, writes the pages and keeps
the directory to them. a page whose text is unchanged is left as it is, and whatever
the directory holds that this run did not write is removed, a directory left empty
with it, so a file that is gone takes its page along. nothing outside the directory
is touched

Source: [`src/tool/doc.mach`](../../../src/tool/doc.mach)

Uses types from [`mach.base.diagnostic`](../base/diagnostic.md), [`mach.base.fail`](../base/fail.md), [`mach.base.intern`](../base/intern.md), [`mach.base.source`](../base/source.md), [`mach.manifest.model`](../manifest/model.md).

| Type | | |
| :--- | :--- | :--- |
| [`Outcome`](#outcome) | `rec` | what a run did |

| On `Outcome` | |
| :--- | :--- |
| [`generate`](#generate) | parse every source file of the project at `root` and write its reference under `<root>/<leaf>/<project id>` |

---

## Outcome

```mach
pub rec Outcome {
    dir:     str;
    modules: u32;
    pages:   u32;
    changed: u32;
    removed: u32;
}
```

what a run did

| Field | Type | |
| :--- | :--- | :--- |
| `dir` | `str` | the directory it owns, `<dir>/<project id>` |
| `modules` | `u32` | the source files it read |
| `pages` | `u32` | the pages it wrote or found already current |
| `changed` | `u32` | the pages whose text it wrote |
| `removed` | `u32` | the files and directories it removed |

### generate

```mach
pub fun generate(alloc: *std.allocator.Allocator, sources: *mach.base.source.SourceMap, diags: *mach.base.diagnostic.DiagnosticStore, itn: *mach.base.intern.Interner,
    m: *mach.manifest.model.Manifest, root: str, leaf: str, out: *Outcome) err[mach.base.fail.Fail]
```

parse every source file of the project at `root` and write its reference under
`<root>/<leaf>/<project id>`

| Parameter | |
| :--- | :--- |
| `alloc` | owns the outcome's path and every failure |
| `sources` | receives every file read, so a diagnostic can name it |
| `diags` | receives the lexer's and parser's diagnostics, the doc linter's warnings among them |
| `itn` | the interner the manifest's names are in |
| `m` | the project's manifest |
| `root` | the project's directory |
| `leaf` | the directory below `root` the reference's directory is made in, such as `doc` |
| `out` | what the run did |
| **returns** | ok; a reported failure when a file does not parse, its diagnostics on `diags`; or the first failure of allocation, or of a file or directory that cannot be read, made, written or removed |

