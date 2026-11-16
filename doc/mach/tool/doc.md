[mach](../README.md) / [tool](README.md) / **doc**

# mach.tool.doc

the documentation generator: a project's reference, written as markdown pages into
the one directory the generator owns, `<dir>/<project id>/`

the document model is built from the analyzed project (`mach.tool.doc.build`) and
rendered to pages (`mach.tool.doc.markdown`); this module writes the pages and
keeps the directory to them. a page whose text is unchanged is left as it is, and
whatever the directory holds that this run did not write is removed, a directory
left empty with it, so a module that is gone takes its page along. nothing outside
the directory is touched

<sub>source: [`src/tool/doc.mach`](../../../src/tool/doc.mach) · imported by 1 module</sub>

<sub>`A` is `std.allocator` · `project` is [`mach.compile.project`](../compile/project.md) · `manifest_model` is [`mach.manifest.model`](../manifest/model.md) · `fail` is [`mach.base.fail`](../base/fail.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Outcome`](#outcome) | `rec` | what a run did |

| On `Outcome` | |
| :--- | :--- |
| [`generate`](#generate) | write the reference of `p` under `<root>/<leaf>/<project id>` |

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
| `modules` | `u32` | the modules documented |
| `pages` | `u32` | the pages it wrote or found already current |
| `changed` | `u32` | the pages whose text it wrote |
| `removed` | `u32` | the files and directories it removed |

### generate

```mach
pub fun generate(alloc: *A.Allocator, p: *project.Project, m: *manifest_model.Manifest, root: str, leaf: str, out: *Outcome) err[fail.Fail]
```

write the reference of `p` under `<root>/<leaf>/<project id>`

| Parameter | |
| :--- | :--- |
| `alloc` | owns the outcome's path and every failure |
| `p` | the project, analyzed through name resolution over every artifact |
| `m` | its manifest |
| `root` | the project's directory |
| `leaf` | the directory below `root` the reference's directory is made in, such as `doc` |
| `out` | what the run did |
| **returns** | ok, or the first failure: allocation, or a directory or page that cannot be read, made, written or removed |

