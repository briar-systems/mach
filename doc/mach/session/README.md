[mach](../README.md) / **session**

# mach.session

the compiler session: the state one process keeps across its builds

a session owns the stores the phases read and write, each worked by its own module: the
diagnostics, the interned names, the types and their facts, the query database, the module
registry, the open buffers, the embedded files and what is retained between builds. it loads a
file's text as an input of the query database, and expires every view borrowed from it when an
input changes.

The directory's root module is [`mach.session`](../session.md).

| Module | |
| :--- | :--- |
| [`embed`](embed.md) | the files a build embeds, read and digested once and watched as query inputs |
| [`module`](module.md) | module ids: a module's index in the session and the stable identity of its name |
| [`query`](query.md) | the query database: every product is of one kind, a `Kind` row its owner declares beside the computation |
| [`retention`](retention.md) | which modules a session keeps products for between builds |

