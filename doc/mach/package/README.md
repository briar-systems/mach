[mach](../README.md) / **package**

# mach.package

the dependency manager behind `mach dep` and `mach init`. it reads a project's
dependency closure from its manifests, resolves version ranges against the releases a
source publishes, realizes every dependency flat under dep/ and records its pin, and
checks a realized closure against all of it. a build never reaches it: the driver only
locates what dep/ holds (compile/closure.mach).

the closure engine (closure, resolve, edit, list, verify) works through three contracts,
so a new source is a new member and not a new engine:

- source.mach: where a dependency's releases and content come from
- pin.mach: where a project records the revision each dependency is held at
- report.mach: where progress, notes and failures go while it works

git, under package/git, is the member of the first two and the one outside tool mach
runs; path.mach realizes a dependency declared by a local path. this module holds what
every part shares: the slot layout under dep/ and the identity rule

The directory's root module is [`mach.package`](../package.md).

| Module | |
| :--- | :--- |
| [`agreement`](agreement.md) | agreement: one package pinned at one commit across a whole dependency closure |
| [`closure`](closure.md) | closure: the walk every dependency command shares |
| [`edit`](edit.md) | edit: the commands that change a project's dependencies: add and remove edit the manifest's [dep] tables, update moves selectors and pins |
| [`git`](git.md) | git: the one outside tool mach runs |
| [`list`](list.md) | list: what `mach dep list` shows: each root declaration with its winning selection, its pin and its state, and under it every requirement of the identity that selection overrides |
| [`path`](path.md) | path: the member that realizes a dependency declared by a local `path` |
| [`pin`](pin.md) | the pin contract: where a project records the exact revision each dependency is held at, and how it reads one back |
| [`pull`](pull.md) | pull: `mach dep pull`, which realizes a project's dependency closure and refuses one that pins a package at more than one commit |
| [`report`](report.md) | where dependency management reports while it works |
| [`resolve`](resolve.md) | resolve: version resolution over a project's closure |
| [`resolver`](resolver.md) | dependency version resolution: PubGrub over the releases each identity lists |
| [`source`](source.md) | the source contract: where a dependency named by a url comes from |
| [`verify`](verify.md) | verify: `mach dep verify`, which checks a realized closure without changing it |

| Directory | |
| :--- | :--- |
| [`git/`](git/README.md) | git: the one outside tool mach runs |

