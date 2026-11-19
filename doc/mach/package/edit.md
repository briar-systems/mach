[mach](../README.md) / [package](README.md) / **edit**

# mach.package.edit

edit: the commands that change a project's dependencies: add and remove edit the
manifest's [dep] tables, update moves selectors and pins. each is a transaction over the
project root held open: the manifest is published only after the closure it declares is
realized, and a failure names, through the report's effects, every change it leaves behind

Source: [`src/package/edit.mach`](../../../src/package/edit.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.manifest.edit`](../manifest/edit.md), [`mach.package.closure`](closure.md), [`mach.package.report`](report.md), [`mach.session`](../session.md).

| Type | | |
| :--- | :--- | :--- |
| [`Edit`](#edit) | `rec` | the project root held open for an edit: the manifest is replaced through a sibling temporary and the dependency root is reached through it |

| On `Edit` | |
| :--- | :--- |
| [`begin`](#begin) | |
| [`finish`](#finish) | release the project root; err when closing it fails |
| [`apply`](#apply) | add or remove a declaration within an open edit |

| Functions | |
| :--- | :--- |
| [`start`](#start) | start recording pins for a new project at `root`, as `mach init` does before it adds the project's dependencies; a root that records them already is left as it is |
| [`update`](#update) | `mach dep update`: move one root-declared or transitive identity, or every one under `all` |
| [`update_apply`](#update_apply) | update: the fixed git selectors move first (a ref is fetched and checked out), so their manifests are the closure's when the version ranges resolve; then every chosen release is realized and the closure is pulled, which syncs every path copy once and reports it |
| [`notes`](#notes) | a resolution's notes, as a command shows them unless --quiet |
| [`release_range`](#release_range) | the caret range of the release resolution picks for the running compiler for the git dependency `spec` declares, added to the manifest `text` of the operation's project, which need not exist yet |
| [`change`](#change) | add or remove the [dep] table `spec` describes and realize the closure it leaves |

---

## Edit

```mach
pub rec Edit {
    a:       *std.allocator.Allocator;
    root:    str;
    root_fd: usize;
}
```

the project root held open for an edit: the manifest is replaced through a sibling
temporary and the dependency root is reached through it

| Field | Type | |
| :--- | :--- | :--- |
| `a` | `*std.allocator.Allocator` | holds the text of a refusal |
| `root` | `str` | the project root, as a refusal names it |
| `root_fd` | `usize` | the project root held open |

### begin

```mach
pub fun begin(edit: *Edit, a: *std.allocator.Allocator, root: str) err[mach.base.fail.Fail]
```

### finish

```mach
pub fun finish(edit: *Edit) err[mach.base.fail.Fail]
```

release the project root; err when closing it fails

### apply

```mach
pub fun apply(edit: *Edit, op: *mach.package.closure.Operation, mode: u8, spec: *mach.manifest.edit.DepTableSpec, name: str, adding: bool,
    purge: bool, quiet: bool, realize: bool, offline: bool) err[mach.base.fail.Fail]
```

add or remove a declaration within an open edit. realize false only declares: the manifest
gains the table (a git one at its resolved range) and nothing is checked out

## Functions

### start

```mach
pub fun start(s: *mach.session.Session, root: str) err[mach.base.fail.Fail]
```

start recording pins for a new project at `root`, as `mach init` does before it adds the
project's dependencies; a root that records them already is left as it is

### update

```mach
pub fun update(op: *mach.package.closure.Operation, all: bool, name: str, quiet: bool, lowest: bool, offline: bool) err[mach.base.fail.Fail]
```

`mach dep update`: move one root-declared or transitive identity, or every one under `all`

| Parameter | |
| :--- | :--- |
| `op` | the operation, on the project root; a failure may borrow from its session, so the caller shows it before ending either |
| `all` | move every identity |
| `name` | the identity to move when not `all` |
| `quiet` | report no progress lines |
| `lowest` | resolve each range to its lowest admitted release |
| `offline` | fetch nothing |
| **returns** | err naming the failure, whose effects the operation's report holds |

### update_apply

```mach
pub fun update_apply(op: *mach.package.closure.Operation, mode: u8, all: bool, name: str, quiet: bool, lowest: bool,
    offline: bool, closure: *Vector[mach.package.closure.Request]) err[mach.base.fail.Fail]
```

update: the fixed git selectors move first (a ref is fetched and checked out), so their
manifests are the closure's when the version ranges resolve; then every chosen release is
realized and the closure is pulled, which syncs every path copy once and reports it

`closure` receives the closure the final pull realized, which the caller frees

### notes

```mach
pub fun notes(rep: *mach.package.report.Report, lines: *Vector[str])
```

a resolution's notes, as a command shows them unless --quiet

### release_range

```mach
pub fun release_range(op: *mach.package.closure.Operation, text: str, spec: *mach.manifest.edit.DepTableSpec, quiet: bool) res[str, mach.base.fail.Fail]
```

the caret range of the release resolution picks for the running compiler for the git
dependency `spec` declares, added to the manifest `text` of the operation's project, which
need not exist yet. nothing is written or checked out, as `mach init` resolves its
dependencies before it writes the project, and the releases read stay in the operation for
the update that checks the dependency out

| Parameter | |
| :--- | :--- |
| `op` | the operation, on the project root; a failure may borrow from its session, so the caller shows it before ending either |
| `text` | the project's manifest, as it will be written |
| `spec` | the dependency, a git source with no ref, and the range to choose in or none for any |
| `quiet` | report no progress line |
| **returns** | the range, `^X.Y.Z`; err naming the failure |

### change

```mach
pub fun change(op: *mach.package.closure.Operation, spec: *mach.manifest.edit.DepTableSpec, name: str, adding: bool,
    purge: bool, quiet: bool, realize: bool, offline: bool) err[mach.base.fail.Fail]
```

add or remove the [dep] table `spec` describes and realize the closure it leaves

| Parameter | |
| :--- | :--- |
| `op` | the operation, on the project root; a failure may borrow from its session, so the caller shows it before ending either |
| `spec` | the table to add, or the name to remove |
| `name` | the dependency identity |
| `adding` | add the table; false removes it |
| `purge` | on remove, delete dep/\<name> as well |
| `quiet` | report no progress lines |
| `realize` | realize the closure; false only writes the table |
| `offline` | fetch nothing |
| **returns** | err naming the failure, whose effects the operation's report holds |

