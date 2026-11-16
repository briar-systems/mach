[mach](../../README.md) / [tool](../README.md) / **doc**

# mach.tool.doc

the documentation generator: a project's reference, written as markdown pages into
the one directory the generator owns, `<dir>/<project id>/`

the document model is built from the analyzed project (`mach.tool.doc.build`) and
rendered to pages (`mach.tool.doc.markdown`); this module writes the pages and
keeps the directory to them. a page whose text is unchanged is left as it is, and
whatever the directory holds that this run did not write is removed, a directory
left empty with it, so a module that is gone takes its page along. nothing outside
the directory is touched

<sub>[`mach.tool.doc`](../doc.md) · 3 modules · imports [`base`](../../base/README.md), [`compile`](../../compile/README.md), [`front`](../../front/README.md), [`host`](../../host/README.md), [`manifest`](../../manifest/README.md), [`session`](../../session/README.md) · imported by [`cli`](../../cli/README.md)</sub>

| Module | |
| :--- | :--- |
| [`build`](build.md) | the document model of a loaded project, built from its syntax trees, its name resolution and its module graph |
| [`markdown`](markdown.md) | the markdown renderer of the document model: GitHub-flavoured pages, one per module, one `README.md` per directory, and the project's page at the root |
| [`model`](model.md) | the document model `mach doc` renders: one project, its directories, its modules and the entries each module exports |

