[mach](../../README.md) / [tool](../README.md) / **doc**

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

The directory's root module is [`mach.tool.doc`](../doc.md).

| Module | |
| :--- | :--- |
| [`build`](build.md) | the document model of a project, built from the syntax trees of its source files |
| [`markdown`](markdown.md) | the markdown renderer of the document model: GitHub-flavoured pages, one per module, one `README.md` per directory, and the project's page at the root |
| [`model`](model.md) | the document model `mach doc` renders: one project, its directories, its modules and the entries each module exports |

